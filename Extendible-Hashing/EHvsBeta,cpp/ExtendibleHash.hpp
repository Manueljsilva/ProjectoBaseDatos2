#ifndef EXTENDIBLE_HASH_HPP
#define EXTENDIBLE_HASH_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <string>
#include "Bucket.hpp"
#include "IndexRecord.hpp"
#include "CommonTypes.hpp"

template <typename KeyType, typename RecordType>
class ExtendibleHash
{
private:
  size_t bucket_max_size;
  std::string index_filename;
  std::string data_filename;
  std::vector<IndexRecord> index_table;
  depth_t global_depth;

  void add_entry_index(const IndexRecord &index_record);
  void init_index();
  void load_index();
  void save_index();
  bool compare_hash_bits(suffix_t key_hash, suffix_t suffix, depth_t depth);
  suffix_t generate_hash(KeyType key);
  IndexRecord *match_index_record(suffix_t key_hash);

public:
  ExtendibleHash(std::string filename, size_t bucket_size = 2, depth_t depth = 1);
  ~ExtendibleHash();
  bool insert(const RecordType &record);
  bool remove(KeyType key);
  std::vector<RecordType> search(KeyType key);
  void show_all_records();
};

// Constructor
template <typename KeyType, typename RecordType>
ExtendibleHash<KeyType, RecordType>::ExtendibleHash(std::string filename, size_t bucket_size, depth_t depth)
    : bucket_max_size(bucket_size), global_depth(depth)
{
  data_filename = filename + "_data.dat";
  index_filename = filename + "_index.dat";
  load_index();
}

// Destructor
template <typename KeyType, typename RecordType>
ExtendibleHash<KeyType, RecordType>::~ExtendibleHash()
{
  save_index();
}

// Añadir un nuevo registro de índice
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::add_entry_index(const IndexRecord &index_record)
{
  index_table.push_back(index_record);
}

// Initialize the index and data files
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::init_index()
{
  Bucket<RecordType> empty_bucket;
  empty_bucket.local_depth = global_depth;
  std::fstream data_file(data_filename, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
  if (!data_file.is_open())
  {
    std::cerr << "ERROR initializing data file" << std::endl;
    exit(1);
  }

  // Create 2^global_depth buckets
  size_t num_buckets = 1 << global_depth;
  std::vector<position_t> bucket_positions(num_buckets);
  for (size_t i = 0; i < num_buckets; ++i)
  {
    position_t position = data_file.tellp();
    bucket_positions[i] = position;
    empty_bucket.save(data_file, position, bucket_max_size);
  }
  data_file.close();

  // Create index records
  index_table.clear();
  for (size_t i = 0; i < num_buckets; ++i)
  {
    IndexRecord index_record;
    index_record.depth = global_depth;
    index_record.suffix = i;
    index_record.bucket_position = bucket_positions[i];
    add_entry_index(index_record);
  }
}

// Cargar el índice desde el archivo
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::load_index()
{
  std::ifstream input_index(index_filename, std::ios::binary);
  if (input_index.is_open())
  {
    IndexRecord index_record;
    while (input_index.read(reinterpret_cast<char *>(&index_record.depth), sizeof(depth_t)))
    {
      input_index.read(reinterpret_cast<char *>(&index_record.suffix), sizeof(suffix_t));
      input_index.read(reinterpret_cast<char *>(&index_record.bucket_position), sizeof(position_t));
      add_entry_index(index_record);
    }
    input_index.close();
    if (!index_table.empty())
    {
      global_depth = index_table[0].depth;
    }
  }
  else
  {
    init_index();
  }
}

// Guardar el índice en el archivo
template <typename KeyType, typename RecordType>
void ExtendibleHash<KeyType, RecordType>::save_index()
{
  std::ofstream output_index(index_filename, std::ios::binary | std::ios::trunc);
  if (output_index.is_open())
  {
    for (const auto &index_record : index_table)
    {
      index_record.save(output_index);
    }
    output_index.close();
  }
  else
  {
    std::cerr << "ERROR saving index file" << std::endl;
  }
}

// Comparar bits del hash y el sufijo
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::compare_hash_bits(suffix_t key_hash, suffix_t suffix, depth_t depth)
{
  suffix_t mask = (1 << depth) - 1;
  return (key_hash & mask) == suffix;
}

// Generar hash de la clave
template <typename KeyType, typename RecordType>
suffix_t ExtendibleHash<KeyType, RecordType>::generate_hash(KeyType key)
{
  return std::hash<KeyType>{}(key);
}

// Encontrar el registro de índice correspondiente al hash
template <typename KeyType, typename RecordType>
IndexRecord *ExtendibleHash<KeyType, RecordType>::match_index_record(suffix_t key_hash)
{
  for (auto &index_record : index_table)
  {
    if (compare_hash_bits(key_hash, index_record.suffix, index_record.depth))
    {
      return &index_record;
    }
  }
  return nullptr;
}

// Insertar un registro en el hash
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::insert(const RecordType &record)
{
  auto key_hash = generate_hash(record.get_key());
  auto index_record = match_index_record(key_hash);
  if (!index_record)
  {
    std::cerr << "Index record not found" << std::endl;
    return false;
  }

  std::fstream data_file(data_filename, std::ios::binary | std::ios::in | std::ios::out);
  if (!data_file.is_open())
  {
    std::cerr << "Error opening data file." << std::endl;
    return false;
  }

  // Cargar el bucket correspondiente
  Bucket<RecordType> bucket;
  bucket.load(data_file, index_record->bucket_position, bucket_max_size);

  // Buscar el último bucket en la cadena si es necesario
  position_t current_bucket_position = index_record->bucket_position;
  while (true)
  {
    // Cargar el bucket actual
    bucket.load(data_file, current_bucket_position, bucket_max_size);

    // Verificar si el bucket tiene espacio
    if (bucket.records.size() < bucket_max_size)
    {
      // Insertar el registro en el bucket actual
      bucket.records.push_back(record);
      bucket.save(data_file, current_bucket_position, bucket_max_size);
      data_file.close();
      return true;
    }
    else
    {
      // Si hay un siguiente bucket, moverse a él
      if (bucket.next_bucket != -1)
      {
        current_bucket_position = bucket.next_bucket;
      }
      else
      {
        // No hay siguiente bucket, crear uno nuevo y encadenarlo
        Bucket<RecordType> new_bucket;
        new_bucket.local_depth = bucket.local_depth; // La profundidad local se mantiene
        new_bucket.records.push_back(record);

        // Escribir el nuevo bucket al final del archivo
        data_file.seekp(0, std::ios::end);
        position_t new_bucket_position = data_file.tellp();
        new_bucket.save(data_file, new_bucket_position, bucket_max_size);

        // Actualizar el bucket actual para que apunte al nuevo bucket
        bucket.next_bucket = new_bucket_position;
        bucket.save(data_file, current_bucket_position, bucket_max_size);
        data_file.close();
        return true;
      }
    }
  }
}

// Eliminar un registro por clave
template <typename KeyType, typename RecordType>
bool ExtendibleHash<KeyType, RecordType>::remove(KeyType key)
{
  auto key_hash = generate_hash(key);
  auto index_record = match_index_record(key_hash);
  if (!index_record)
  {
    return false; // No se encontró el índice correspondiente
  }

  std::fstream data_file(data_filename, std::ios::binary | std::ios::in | std::ios::out);
  if (!data_file.is_open())
  {
    std::cerr << "Error opening data file." << std::endl;
    return false;
  }

  position_t current_bucket_position = index_record->bucket_position;
  position_t previous_bucket_position = -1;
  Bucket<RecordType> bucket;

  while (current_bucket_position != -1)
  {
    bucket.load(data_file, current_bucket_position, bucket_max_size);

    // Buscar el registro en el bucket actual
    bool record_found = false;
    for (auto it = bucket.records.begin(); it != bucket.records.end(); ++it)
    {
      if (it->get_key() == key)
      {
        // Eliminar el registro
        bucket.records.erase(it);
        record_found = true;
        break;
      }
    }

    if (record_found)
    {
      // guardo el bucket actualizado
      bucket.save(data_file, current_bucket_position, bucket_max_size);

      if (bucket.records.empty() && previous_bucket_position != -1)
      {
        // update el next_bucket del bucket anterior
        Bucket<RecordType> previous_bucket;
        previous_bucket.load(data_file, previous_bucket_position, bucket_max_size);
        previous_bucket.next_bucket = bucket.next_bucket;
        previous_bucket.save(data_file, previous_bucket_position, bucket_max_size);
      }

      data_file.close();
      return true; // Registro eliminado con éxito
    }
    else
    {
      // Mover al siguiente bucket en la cadena
      previous_bucket_position = current_bucket_position;
      current_bucket_position = bucket.next_bucket;
    }
  }

  data_file.close();
  return false; // Registro no encontrado
}

// Buscar registros por clave
template <typename KeyType, typename RecordType>
std::vector<RecordType> ExtendibleHash<KeyType, RecordType>::search(KeyType key)
{
  std::vector<RecordType> result_records;
  auto key_hash = generate_hash(key);
  auto index_record = match_index_record(key_hash);
  if (!index_record)
  {
    return result_records;
  }

  std::fstream data_file(data_filename, std::ios::binary | std::ios::in);
  if (data_file.is_open())
  {
    position_t current_bucket_position = index_record->bucket_position;
    Bucket<RecordType> bucket;

    while (current_bucket_position != -1)
    {
      bucket.load(data_file, current_bucket_position, bucket_max_size);

      for (const auto &record : bucket.records)
      {
        if (record.get_key() == key)
        {
          result_records.push_back(record);
          // solo el primer registro encontrado
        }
      }

      // Mover al siguiente bucket en la cadena
      current_bucket_position = bucket.next_bucket;
    }
    data_file.close();
  }
  return result_records;
}

#endif // EXTENDIBLE_HASH_HPP
