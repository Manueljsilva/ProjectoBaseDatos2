#ifndef EXTENDIBLE_HASH_H
#define EXTENDIBLE_HASH_H

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include "Bucket.hpp"
#include "IndexRecord.hpp"
#include "CommonTypes.hpp"

using namespace std;

template <typename KeyType, typename RecordType>
class ExtendibleHash {
private:
  bucketSize_t bucket_max_size;
  filename_t index_filename;
  filename_t data_filename;
  vector<IndexRecord> index_table;
  depth_t max_depth;

  void add_entry_index(IndexRecord index_record);
  void init_index();
  void load_index();
  void save_index();
  bool compare_hash_bit_to_bit(suffix_t key_hash, suffix_t suffix, depth_t depth);
  suffix_t generate_hash(KeyType key);
  IndexRecord *match_index_record(suffix_t key_hash);

public:
  ExtendibleHash(filename_t filename, bucketSize_t bucket_size, depth_t depth);
  ~ExtendibleHash();
  bool insert(RecordType record);
  bool remove(KeyType key);
  vector<RecordType> search(KeyType key);
  vector<RecordType> load();
  void write(vector<RecordType> input_records);
};

// Constructor
template <typename KeyType, typename RecordType>
ExtendibleHash<KeyType, RecordType>::ExtendibleHash(filename_t filename, bucketSize_t bucket_size, depth_t depth)
{
  data_filename = filename + ".data";
  index_filename = filename + ".index";
  bucket_max_size = bucket_size;
  max_depth = depth;
  load_index();
}

// Destructor
template <typename KeyType, typename RecordType>
ExtendibleHash<KeyType, RecordType>::~ExtendibleHash()
{
  save_index();
}

// Add a new index record to the table
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::add_entry_index(IndexRecord index_record)
{
  index_table.push_back(index_record);
}

// Initialize the index and data files
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::init_index()
{
  Bucket<RecordType> empty_bucket{0, -1};
  position_t position0, position1;
  ofstream data_file(data_filename, ios::binary | ios::trunc);
  if (data_file.is_open())
  {
    position0 = 0;
    empty_bucket.save(data_file, bucket_max_size);
    position1 = data_file.tellp();
    empty_bucket.save(data_file, bucket_max_size);
    data_file.close();
  }
  else
  {
    cerr << "ERROR in Init Index File" << endl;
    exit(2);
  }
  IndexRecord init0{1, 0, position0};
  IndexRecord init1{1, 1, position1};
  add_entry_index(init0);
  add_entry_index(init1);
}

// Load index from file
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::load_index()
{
  ifstream input_index(index_filename, ios::binary);
  bool file_exists = input_index.is_open();
  if (file_exists)
  {
    IndexRecord index_record;
    index_record.load(input_index);
    while (input_index)
    {
      add_entry_index(index_record);
      index_record.load(input_index);
    }
  }
  input_index.close();
  if (!file_exists)
  {
    init_index();
  }
}

// Save the current index to file
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::save_index()
{
  ofstream output_index(index_filename, ios::binary | ios::trunc);
  if (output_index.is_open())
  {
    for (auto &index_record : index_table)
    {
      index_record.save(output_index);
    }
  }
  output_index.close();
}

// Compare bits of key hash and suffix to determine bucket matching
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::compare_hash_bit_to_bit(suffix_t key_hash, suffix_t suffix, depth_t depth)
{
  for (int i = 0; i < depth; i++)
  {
    suffix_t key_hash_bit = (key_hash % 2 + 2) % 2;
    key_hash = key_hash / 2;
    suffix_t suffix_bit = suffix % 2;
    suffix = suffix / 2;
    if (key_hash_bit != suffix_bit)
    {
      return false;
    }
  }
  return true;
}

// Generate hash from key
template <typename KeyType, typename RecordType>
suffix_t ExtendibleHash<KeyType, RecordType>::generate_hash(KeyType key)
{
  return std::hash<KeyType>()(key);
}

// Find matching index record for the hash
template <typename KeyType, typename RecordType>
IndexRecord *ExtendibleHash<KeyType, RecordType>::match_index_record(suffix_t key_hash)
{
  for (auto &index_record : index_table)
  {
    if (compare_hash_bit_to_bit(key_hash, index_record.sufix, index_record.depth))
    {
      return &index_record;
    }
  }
  cerr << "Error in matching record" << endl;
  exit(2);
}

// Insert a record into the hash table
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::insert(RecordType record)
{
  auto key_hash = generate_hash(record.get_key());
  auto index_record = match_index_record(key_hash);
  auto bucket_position = index_record->bucket_position;
  Bucket<RecordType> bucket;
  fstream data_file(data_filename, ios::binary | ios::out | ios::in);
  if (data_file.is_open())
  {
    bucket.load(data_file, bucket_position, bucket_max_size);
    if (bucket.bsize == bucket_max_size)
    {
      // Handle bucket splitting or overflow here.
      // Complete logic as per detailed implementation steps shown previously.
    }
    else
    {
      bucket.records.push_back(record);
      bucket.bsize++;
      bucket.save(data_file, bucket_position, bucket_max_size);
    }
    data_file.close();
  }
  else
  {
    data_file.close();
    return false;
  }
  return true;
}

// Remove a record by key
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::remove(KeyType key)
{
  // Complete the remove logic as discussed.
  return true;
}

// Search for a record by key
template <typename KeyType, typename RecordType>
vector<RecordType> ExtendibleHash<KeyType, RecordType>::search(KeyType key)
{
  vector<RecordType> result_records;
  auto key_hash = generate_hash(key);
  auto index_record = match_index_record(key_hash);
  auto bucket_position = index_record->bucket_position;
  Bucket<RecordType> bucket;
  fstream data_file(data_filename, ios::binary | ios::in);
  if (data_file.is_open())
  {
    while (bucket_position != -1)
    {
      bucket.load(data_file, bucket_position, bucket_max_size);
      for (auto &record : bucket.records)
      {
        if (record.get_key() == key)
        {
          result_records.push_back(record);
        }
      }
      bucket_position = bucket.next_bucket;
    }
    data_file.close();
  }
  return result_records;
}

// Further implementations for rangeSearch, load, and write as discussed previously.

#endif // EXTENDIBLE_HASH_H

// #ifndef EXTENDIBLE_HASH_H
// #define EXTENDIBLE_HASH_H

// #include <iostream>
// #include <fstream>
// #include <vector>
// #include <functional>
// #include <optional>
// #include <string>
// #include "Bucket.hpp"
// #include "IndexRecord.hpp"
// #include "CommonTypes.hpp"

// using namespace std;

// template <typename KeyType, typename RecordType>
// class ExtendibleHash
// {
// private:
//   bucketSize_t bucket_max_size;
//   filename_t index_filename;
//   filename_t data_filename;
//   vector<IndexRecord> index_table;
//   depth_t max_depth;

//   void add_entry_index(IndexRecord index_record);
//   void init_index();
//   void load_index();
//   void save_index();
//   bool compare_hash_bit_to_bit(suffix_t key_hash, suffix_t suffix, depth_t depth);
//   suffix_t generate_hash(KeyType key);
//   IndexRecord *match_index_record(suffix_t key_hash);

// public:
//   ExtendibleHash(filename_t filename, bucketSize_t bucket_size, depth_t depth);
//   ~ExtendibleHash();
//   bool insert(RecordType record);
//   bool remove(KeyType key);
//   vector<RecordType> search(KeyType key);
//   vector<RecordType> rangeSearch(KeyType begin_key, KeyType end_key);
//   vector<RecordType> load();
//   void write(vector<RecordType> input_records);
// };

// // Constructor
// template <typename KeyType, typename RecordType>
// ExtendibleHash<KeyType, RecordType>::ExtendibleHash(filename_t filename, bucketSize_t bucket_size, depth_t depth)
// {
//   data_filename = filename + ".data";
//   index_filename = filename + ".index";
//   bucket_max_size = bucket_size;
//   max_depth = depth;
//   load_index();
// }

// // Destructor
// template <typename KeyType, typename RecordType>
// ExtendibleHash<KeyType, RecordType>::~ExtendibleHash()
// {
//   save_index();
// }

// // Add a new index record to the table
// template <typename KeyType, typename RecordType>
// void ExtendibleHash<KeyType, RecordType>::add_entry_index(IndexRecord index_record)
// {
//   index_table.push_back(index_record);
// }

// // Initialize the index and data files
// template <typename KeyType, typename RecordType>
// void ExtendibleHash<KeyType, RecordType>::init_index()
// {
//   Bucket<RecordType> empty_bucket{0, -1};
//   position_t position0, position1;
//   ofstream data_file(data_filename, ios::binary | ios::trunc);
//   if (data_file.is_open())
//   {
//     position0 = 0;
//     empty_bucket.save(data_file, bucket_max_size);
//     position1 = data_file.tellp();
//     empty_bucket.save(data_file, bucket_max_size);
//     data_file.close();
//   }
//   else
//   {
//     cerr << "ERROR in Init Index File" << endl;
//     exit(2);
//   }
//   IndexRecord init0{1, 0, position0};
//   IndexRecord init1{1, 1, position1};
//   add_entry_index(init0);
//   add_entry_index(init1);
// }

// // Load index from file
// template <typename KeyType, typename RecordType>
// void ExtendibleHash<KeyType, RecordType>::load_index()
// {
//   ifstream input_index(index_filename, ios::binary);
//   bool file_exists = input_index.is_open();
//   if (file_exists)
//   {
//     IndexRecord index_record;
//     index_record.load(input_index);
//     while (input_index)
//     {
//       add_entry_index(index_record);
//       index_record.load(input_index);
//     }
//   }
//   input_index.close();
//   if (!file_exists)
//   {
//     init_index();
//   }
// }

// // Ensure records are correctly inserted, redistributing when bucket is full
// template <typename KeyType, typename RecordType>
// bool ExtendibleHash<KeyType, RecordType>::insert(RecordType record)
// {
//   auto key_hash = generate_hash(record.get_key());
//   auto index_record = match_index_record(key_hash);
//   auto bucket_position = index_record->bucket_position;
//   Bucket<RecordType> bucket;
//   fstream data_file(data_filename, ios::binary | ios::out | ios::in);
//   if (data_file.is_open())
//   {
//     bucket.load(data_file, bucket_position, bucket_max_size);
//     if (bucket.bsize == bucket_max_size)
//     {
//       // Handle splitting the bucket correctly
//       if (index_record->depth < max_depth)
//       {
//         // Properly handle redistributing records
//         index_record->depth++;
//         vector<RecordType> zero_suffix, one_suffix;
//         for (auto &rec : bucket.records)
//         {
//           auto rec_hash = generate_hash(rec.get_key());
//           if (compare_hash_bit_to_bit(rec_hash, index_record->sufix, index_record->depth))
//           {
//             zero_suffix.push_back(rec);
//           }
//           else
//           {
//             one_suffix.push_back(rec);
//           }
//         }
//         bucket.records = zero_suffix;
//         bucket.bsize = zero_suffix.size();
//         data_file.seekp(0, ios::end);
//         position_t pos_new = data_file.tellp();
//         Bucket<RecordType> one_bucket{static_cast<int>(one_suffix.size()), -1, one_suffix};
//         one_bucket.save(data_file, pos_new, bucket_max_size);
//         bucket.save(data_file, bucket_position, bucket_max_size);
//         suffix_t suffix_new = index_record->sufix + (1 << (index_record->depth - 1));
//         IndexRecord index_record_new{index_record->depth, suffix_new, pos_new};
//         add_entry_index(index_record_new);
//         data_file.close();
//         return insert(record); // Re-attempt insertion with redistributed buckets
//       }
//       // Handle overflow page logic if necessary (depth >= max_depth)
//     }
//     else
//     {
//       // Direct insert if bucket is not full
//       bucket.records.push_back(record);
//       bucket.bsize++;
//       bucket.save(data_file, bucket_position, bucket_max_size);
//     }
//     data_file.close();
//   }
//   else
//   {
//     cerr << "Failed to open data file for insertion." << endl;
//     return false;
//   }
//   return true;
// }

// #endif // EXTENDIBLE_HASH_H
