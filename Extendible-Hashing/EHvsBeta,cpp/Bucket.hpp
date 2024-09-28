#ifndef BUCKET_H
#define BUCKET_H

#include <iostream>
#include <vector>
#include <fstream>
#include "CommonTypes.hpp"
#include "TVSeriesRecord.hpp"
using namespace std;

template <typename RecordType>
struct Bucket
{
  bucketSize_t bsize;
  position_t next_bucket;
  std::vector<RecordType> records;

  void load(std::fstream &file, position_t bucket_position, bucketSize_t bucket_size);
  void save(std::fstream &file, position_t bucket_pos, bucketSize_t bucket_size);
  void save(std::ofstream &file, bucketSize_t bucket_size);
};

template <typename RecordType>
void Bucket<RecordType>::load(std::fstream &file, position_t bucket_position, bucketSize_t bucket_size)
{
  file.seekg(bucket_position, std::ios::beg);
  file.read(reinterpret_cast<char *>(&bsize), sizeof(bucketSize_t));
  file.read(reinterpret_cast<char *>(&next_bucket), sizeof(position_t));
  records.clear();
  for (int i = 0; i < bsize; i++)
  {
    RecordType record;
    record.load(file);
    records.push_back(record);
  }
}

template <typename RecordType>
void Bucket<RecordType>::save(std::fstream &file, position_t bucket_pos, bucketSize_t bucket_size)
{
  file.seekp(bucket_pos, std::ios::beg);
  file.write(reinterpret_cast<const char *>(&bsize), sizeof(bucketSize_t));
  file.write(reinterpret_cast<const char *>(&next_bucket), sizeof(position_t));
  RecordType record;
  for (int i = 0; i < bucket_size; i++)
  {
    if (i < records.size())
    {
      record = records[i];
    }
    record.save(file);
  }
}

template <typename RecordType>
void Bucket<RecordType>::save(std::ofstream &file, bucketSize_t bucket_size)
{
  file.write(reinterpret_cast<const char *>(&bsize), sizeof(bucketSize_t));
  file.write(reinterpret_cast<const char *>(&next_bucket), sizeof(position_t));
  RecordType record;
  for (int i = 0; i < bucket_size; i++)
  {
    if (i < records.size())
    {
      record = records[i];
    }
    record.save(file);
  }
}

#endif // BUCKET_H
