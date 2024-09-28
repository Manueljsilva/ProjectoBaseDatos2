
#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include "ExtendibleHash.hpp" // Including the corrected ExtendibleHash header with implementations.
using namespace std;

// Function to display the interactive menu
void displayMenu()
{
  cout << endl
       << endl
       << "        Menu " << std::endl;
  cout << "--------------------" << std::endl;
  cout << "1. Cargar .csv" << std::endl;
  cout << "3. Buscar registro" << std::endl;
  cout << "0. Salir" << std::endl;
  cout << "\nIngrese una opcion: ";
}

int main()
{
  cout << "Iniciando el programa..." << std::endl;

  // Create an instance of ExtendibleHash with a global depth of 3, bucket size of 2
  ExtendibleHash<string, TVSeriesRecord> hashTable("buckets", 2, 3); // Specify KeyType as string for TVSeriesRecord

  vector<TVSeriesRecord> seriesList;
  int opcion;
  string filename;
  bool repetir = true;

  do
  {
    displayMenu();
    cin >> opcion;

    switch (opcion)
    {
    case 1:
    {
      cout << "Ingrese el nombre del archivo CSV (sin extension): ";
      cin >> filename;
      filename += ".csv";                // Append the .csv extension
      seriesList = readCSV(filename, 7); // Load up to 7 records for testing

      if (seriesList.empty())
      {
        cout << "No se cargaron series desde el archivo CSV." << endl;
      }
      else
      {
        cout << "Series cargadas desde el archivo CSV." << endl;
        // Insert series into the hash table
        int cont = {};
        for (const auto &series : seriesList)
        {
          cout << "Insertando: " << series.title << endl;
          hashTable.insert(series);
          cont++;
        }
        cout << "Insercion completa." << endl;
        cout << "Hay "<<cont <<" registros." << endl;
      }
      break;
    }
    case 3:
    {
      string key;
      cout << "Ingrese la clave (titulo) para buscar: ";
      cin.ignore();
      getline(cin, key);

      auto start = chrono::high_resolution_clock::now();
      auto resultado = hashTable.search(key);
      auto end = chrono::high_resolution_clock::now();
      chrono::duration<double> duration = end - start;

      if (!resultado.empty())
      {
        cout << "Registros encontrados: " << std::endl;
        for (const auto &record : resultado)
        {
          record.show(); // Assuming show() prints the details of the record.
        }
      }
      else
      {
        cout << "Registro no encontrado." << std::endl;
      }
      cout << "BUSQUEDA: Tiempo transcurrido: " << duration.count() << " segundos" << std::endl;
      break;
    }
    case 0:
      repetir = false;
      break;
    default:
      cout << "Opcion invalida." << std::endl;
      break;
    }
  } while (repetir);

  cout << "Programa finalizado." << std::endl;
  return 0;
}
