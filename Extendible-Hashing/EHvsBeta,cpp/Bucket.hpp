#ifndef BUCKET_HPP
#define BUCKET_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include "CommonTypes.hpp"
#include "TVSeriesRecord.hpp"

template <typename RecordType>
struct Bucket
{
  position_t next_bucket; // puntero lógico del sgte bucket (-1 si no hay)
  depth_t local_depth;    // profundidad local de cada bucket
  std::vector<RecordType> records;

  Bucket() : next_bucket(-1), local_depth(1) {}

  void load(std::fstream &file, position_t bucket_position, size_t bucket_size);
  void save(std::fstream &file, position_t bucket_position, size_t bucket_size);
};


template <typename RecordType>
void Bucket<RecordType>::load(std::fstream &file, position_t bucket_position, size_t bucket_size)
{
  file.seekg(bucket_position, std::ios::beg);
  file.read(reinterpret_cast<char *>(&next_bucket), sizeof(position_t));
  file.read(reinterpret_cast<char *>(&local_depth), sizeof(depth_t));
  size_t record_count;
  file.read(reinterpret_cast<char *>(&record_count), sizeof(size_t));
  records.resize(record_count);
  for (size_t i = 0; i < record_count; ++i)
  {
    records[i].load(file);
  }
}

template <typename RecordType>
void Bucket<RecordType>::save(std::fstream &file, position_t bucket_position, size_t bucket_size)
{
  file.seekp(bucket_position, std::ios::beg);
  file.write(reinterpret_cast<const char *>(&next_bucket), sizeof(position_t));
  file.write(reinterpret_cast<const char *>(&local_depth), sizeof(depth_t));
  size_t record_count = records.size();
  file.write(reinterpret_cast<const char *>(&record_count), sizeof(size_t));
  for (const auto &record : records)
  {
    record.save(file);
  }
  // se llena el espacio vacío con registros si es necesario
  RecordType empty_record;
  for (size_t i = records.size(); i < bucket_size; ++i)
  {
    empty_record.save(file);
  }
}

#endif // BUCKET_HPP