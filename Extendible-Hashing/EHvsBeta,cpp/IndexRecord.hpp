#ifndef INDEX_RECORD_HPP
#define INDEX_RECORD_HPP

#include <iostream>
#include <fstream>
#include "CommonTypes.hpp"

struct IndexRecord
{
  depth_t depth;              // profundidad local
  suffix_t suffix;            // sufijo del índice (bits significativos del hash)
  position_t bucket_position; // posición del bucket correspondiente

  void load(std::ifstream &file);
  void save(std::ofstream &file) const;
  void print() const;
};

void IndexRecord::load(std::ifstream &file)
{
  file.read(reinterpret_cast<char *>(&depth), sizeof(depth_t));
  file.read(reinterpret_cast<char *>(&suffix), sizeof(suffix_t));
  file.read(reinterpret_cast<char *>(&bucket_position), sizeof(position_t));
}

void IndexRecord::save(std::ofstream &file) const
{
  file.write(reinterpret_cast<const char *>(&depth), sizeof(depth_t));
  file.write(reinterpret_cast<const char *>(&suffix), sizeof(suffix_t));
  file.write(reinterpret_cast<const char *>(&bucket_position), sizeof(position_t));
}

void IndexRecord::print() const
{
  std::cout << "Depth: " << depth << " | Suffix: " << suffix << " | Bucket Position: " << bucket_position << std::endl;
}

#endif // INDEX_RECORD_HPP
