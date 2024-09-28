#ifndef INDEX_RECORD_H
#define INDEX_RECORD_H

#include <iostream>
#include <fstream>
#include "CommonTypes.hpp"
using namespace std;

struct IndexRecord
{
  depth_t depth;
  suffix_t sufix;
  position_t bucket_position;

  void load(std::ifstream &file);
  void save(std::ofstream &file);
  void print();
};

void IndexRecord::load(std::ifstream &file)
{
  file.read(reinterpret_cast<char *>(&depth), sizeof(depth_t));
  file.read(reinterpret_cast<char *>(&sufix), sizeof(suffix_t));
  file.read(reinterpret_cast<char *>(&bucket_position), sizeof(position_t));
}

void IndexRecord::save(std::ofstream &file)
{
  file.write(reinterpret_cast<const char *>(&depth), sizeof(depth_t));
  file.write(reinterpret_cast<const char *>(&sufix), sizeof(suffix_t));
  file.write(reinterpret_cast<const char *>(&bucket_position), sizeof(position_t));
}

void IndexRecord::print()
{
  std::cout << "Depth: " << depth << " | Suffix: " << sufix << " | Bucket Position: " << bucket_position << std::endl;
}

#endif // INDEX_RECORD_H
