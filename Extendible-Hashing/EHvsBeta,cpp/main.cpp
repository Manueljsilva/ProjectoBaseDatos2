#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <limits> // std::numeric_limits
#include "ExtendibleHash.hpp"
using namespace std;

void displayMenu()
{
  cout << endl
       << endl
       << "        Menu " << endl;
  cout << "--------------------" << endl;
  cout << "1. Cargar .csv" << endl;
  cout << "2. Buscar registro" << endl;
  cout << "3. Eliminar registro" << endl;
  cout << "0. Salir" << endl;
  cout << endl
       << "Ingrese una opcion: ";
}

int main()
{
  cout << "Iniciando el programa..." << endl;
  string file = "buckets";

  // ExtendibleHash con profundidad global de 1 y tamaño de bucket de 10
  ExtendibleHash<string, TVSeriesRecord> hashTable(file, 10, 1);

  // // 100 registros
  // ExtendibleHash<string, TVSeriesRecord> hashTable(file, 16, 3);

  // // 1k registros
  // ExtendibleHash<string, TVSeriesRecord> hashTable(file, 32, 5);

  // // 10k registros
  // ExtendibleHash<string, TVSeriesRecord> hashTable(file, 64, 8);

  // // 50k registros
  // ExtendibleHash<std::string, TVSeriesRecord> hashTable("buckets", 64, 10);

  vector<TVSeriesRecord> seriesList;
  int opcion;
  string filename;
  bool repetir = true;

  do
  {
    displayMenu();
    cin >> opcion;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // limpio el buffer de entrada

    switch (opcion)
    {
    case 1:
    {
      cout << "Ingrese el nombre del archivo CSV (sin extension): ";
      getline(cin, filename);
      filename += ".csv";

      // cargo los registros
      seriesList = readCSV(filename);

      if (seriesList.empty())
      {
        cout << "No se cargaron series desde el archivo CSV." << endl;
      }
      else
      {
        cout << "Series cargadas desde el archivo CSV." << endl;
        // Insertar las series en la tabla hash
        int cont = 0;

        auto start = chrono::high_resolution_clock::now();
        for (const auto &series : seriesList)
        {
          cout << "Insertando: " << series.title << endl;
          hashTable.insert(series);
          cont++;
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double> duration = end - start;
        cout << "Insercion completa." << endl
             << endl;
        cout << "Se insertaron " << cont << " registros." << endl;
        cout << "INSERCION: Tiempo transcurrido: " << duration.count() << " segundos" << endl;
      }
      break;
    }
    case 2:
    {
      string key;
      cout << "Ingrese la clave (titulo) para buscar: ";
      getline(cin, key);

      // elimino caracteres nulos y convierto a minúscula para generar el mismo key de registro que en TVSeriesRecord y poder ejecutar una búsqueda correcta
      key.erase(std::remove(key.begin(), key.end(), '\0'), key.end());
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);

      // clave de búsqueda
      cout << "Buscando clave: [" << key << "]" << endl;

      auto start = chrono::high_resolution_clock::now();
      auto resultado = hashTable.search(key);
      auto end = chrono::high_resolution_clock::now();
      chrono::duration<double> duration = end - start;

      if (!resultado.empty())
      {
        cout << "Registros encontrados: " << endl;
        for (const auto &record : resultado)
        {
          record.show(); // cada campo del registro
        }
      }
      else
      {
        cout << "Registro no encontrado." << endl;
      }
      cout << "BUSQUEDA: Tiempo transcurrido: " << duration.count() << " segundos" << endl;
      break;
    }
    case 3:
    {
      std::string key;
      cout << "Ingrese la clave (titulo) para eliminar: ";
      getline(cin, key);

      key.erase(std::remove(key.begin(), key.end(), '\0'), key.end());
      std::transform(key.begin(), key.end(), key.begin(), ::tolower);

      auto start = chrono::high_resolution_clock::now();
      bool success = hashTable.remove(key);
      auto end = chrono::high_resolution_clock::now();
      chrono::duration<double> duration = end - start;

      if (success)
      {
        cout << "REGISTRO ELIMINADO CORRECTAMENTE." << endl;
      }
      else
      {
        cout << "No se encontro el registro a eliminar." << endl;
      }
      cout << "ELIMINACION: Tiempo transcurrido: " << duration.count() << " segundos" << endl;
      break;
    }
    case 0:
    {
      repetir = false;
    }
    break;
    }
  } while (repetir);

  cout << "Programa finalizado." << endl;
  return 0;
}
