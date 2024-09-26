#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <utility>
#include <cmath>
#include <bitset>

using namespace std;

// estructura de las series de TV
struct Series {
  string title;
  string releaseYear;
  string runtime;
  string genre;
  string rating;
  string cast;
  string synopsis;

// imprimo los datos de cada serie de TV
  void show() {
    cout << "Title: " << title << endl;
    cout << "Release Year: " << releaseYear << endl;
    cout << "Runtime: " << runtime << endl;
    cout << "Genre: " << genre << endl;
    cout << "Rating: " << rating << endl;
    cout << "Cast: " << cast << endl;
    cout << "Synopsis: " << synopsis << endl;
    cout << "-----------------------------------" << endl;
  }
};

// limpio caracteres especiales para extraer un rango de años o un año
string cleanYear(string &yearField) {
  string cleaned = yearField;
  cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), [](unsigned char c) { 
                return !isalnum(c) && c != '(' && c != ')' && c != '-'; }),
                cleaned.end());

  // uso una expresión regular para capturar un rango de años o un año entre los paréntesis
  regex yearPattern(R"(\((\d{4})-?(\d{4})?\))");
  smatch match;
  if (regex_search(cleaned, match, yearPattern)) {
    if (match.size() > 2 && match[2].matched) {
      // si hay un rango de años 
      return match[1].str() + "-" + match[2].str();
    }
    else
    {
      // Si es un solo año, devuelve solo el año sin paréntesis
      return match[1].str();
    }
  }
  return cleaned;
}

// en algunos campos del registro diferentes al de Release Year, los valores están dentro de comillas
string cleanField(string &field) {
  string cleaned = field;
  // formateo de espacios en blanco al inicio y final del campo
  cleaned.erase(cleaned.begin(), find_if(cleaned.begin(), cleaned.end(), [](unsigned char ch)
                                         { return !isspace(ch); })); // función lambda
  cleaned.erase(find_if(cleaned.rbegin(), cleaned.rend(), [](unsigned char ch) {
                return !isspace(ch); }).base(),
                cleaned.end());

  if (!cleaned.empty() && cleaned.front() == '"' && cleaned.back() == '"') {
    cleaned = cleaned.substr(1, cleaned.size() - 2); // remuevo comillas al inicio y fin
  }
  return cleaned;
}

// leer y procesar el .csv
vector<Series> readCSV(string &filename, int maxRecords = 20) {
  ifstream file(filename);
  string line;
  vector<Series> seriesList;
  int count = 0; // contador para limitar el número de registros leídos

  if (!file.is_open()) {
    cerr << "Error: No se pudo abrir el archivo." << endl;
    return seriesList;
  }

  // leo la primera línea (header) -> ya que son los campos de cada registro
  getline(file, line);

  while (getline(file, line) && count < maxRecords) {
    stringstream ss(line);
    Series series;
    string field;

    // Se lee y limpia cada campo
    // Título
    getline(ss, field, ',');
    series.title = cleanField(field);

    // Año de lanzamiento
    getline(ss, field, ',');
    series.releaseYear = cleanYear(field);

    // Duración
    getline(ss, field, ',');
    series.runtime = cleanField(field);

    // Leer campo de género (delimitado por comillas dobles)
    getline(ss, field, '"'); // leo hasta la primera comilla
    getline(ss, field, '"'); // le el contenido
    series.genre = field;

    // **Ignorar la coma que sigue al campo de género**
    ss.ignore(1);

    // Leer el campo de rating
    getline(ss, field, ',');
    series.rating = cleanField(field);

    // Leer el campo de cast (delimitado por comillas dobles)
    getline(ss, field, '"'); // leo hasta la primera comilla
    getline(ss, field, '"'); // le el contenido
    series.cast = field;

    // **Ignorar la coma que sigue al campo de cast**
    ss.ignore(1);

    // Leer el campo de sinopsis (delimitado por comillas dobles)
    getline(ss, field, '"'); // leo hasta la primera comilla
    getline(ss, field, '"'); // le el contenido
    series.synopsis = field;

    seriesList.push_back(series);
    count++; // incremento la cantidad de registros leídos
  }

  file.close();
  return seriesList;
}

// Bucket en Extendible Hashing para series de TV
class Bucket {
public:
  int localDepth;                  // bucket local depth
  vector<pair<int, Series>> items; // almaceno pares clave-Serie
  // `items` es un vector de pares y maneja su propia memoria

  Bucket(int depth) : localDepth(depth) {}

  bool isFull(int bucketSize) const { return items.size() >= bucketSize; }

  void insert(int key, const Series &series) {
    items.emplace_back(key, series);
  }

  // si la serie se encuentra, retorna un puntero
  Series *search(int key) {
    for (auto &item : items) {
      if (item.first == key) {
        return &(item.second);
      }
    }
    return nullptr; // no se encontró el registro
  }

  bool remove(int key) {
    auto it = remove_if(items.begin(), items.end(), [key](const pair<int, Series> &item)
                        { return item.first == key; });

    if (it != items.end()) {
      items.erase(it, items.end());
      return true;
    }
    return false;
  }
};

// ExtendibleHashing indexado por bits para almacenar Series de TV
class ExtendibleHashing {
private:
  int globalDepth;            // directory global depth
  int bucketSize;             // factor de bloque (tamaño máximo de cada bucket)
  vector<Bucket *> directory; 

  // Función hash que devuelve los primeros D bits de la clave
  int hashFunction(int key) const {
    return key & ((1 << globalDepth) - 1); // bits para el hashing
  }

  // divido un bucket cuándo está lleno
  void splitBucket(int index) {
    Bucket *oldBucket = directory[index];
    int oldLocalDepth = oldBucket->localDepth;
    int newLocalDepth = oldLocalDepth + 1;
    oldBucket->localDepth = newLocalDepth;

    // creo un nuevo bucket con la profundidad local actualizada
    Bucket *newBucket = new Bucket(newLocalDepth);

    // hallo el bit de división (el bit que se está incrementando)
    int splitBit = 1 << oldLocalDepth;

    // redistribuyo los elementos en los buckets
    auto it = oldBucket->items.begin();
    while (it != oldBucket->items.end()) {
      if (it->first & splitBit) {
        newBucket->insert(it->first, it->second);
        it = oldBucket->items.erase(it);
      }
      else { ++it; }
    }

    // actualizo el directorio para apuntar a los nuevos buckets (por ahora de forma dinámica y no lógica)
    int directorySize = directory.size();
    for (int i = 0; i < directorySize; ++i) {
      if ((i & ((1 << newLocalDepth) - 1)) == index) {
        directory[i] = oldBucket;
      }
      else if ((i & ((1 << newLocalDepth) - 1)) == (index | splitBit)) {
        directory[i] = newBucket;
      }
    }
  }

  // duplico el directorio cuando alcanzo la profundidad máxima
  void doubleDirectory() {
    globalDepth++;
    int currentSize = directory.size();
    directory.resize(currentSize * 2);
    for (int i = 0; i < currentSize; ++i) {
      directory[i + currentSize] = directory[i];
    }
  }

public:
  ExtendibleHashing(int bucketSize) : globalDepth(1), bucketSize(bucketSize) {
    // seteo directorio con dos buckets iniciales, cada uno con localDepth = 1
    directory.push_back(new Bucket(globalDepth));
    directory.push_back(new Bucket(globalDepth));
  }

  // inserto registros en la el extendible
  void insert(int key, const Series &series) {
    int idx = hashFunction(key);
    Bucket *bucket = directory[idx];

    if (bucket->isFull(bucketSize)) {
      // si el bucket está lleno, evaluo si tengo que duplicar el directorio
      if (bucket->localDepth == globalDepth) {
        doubleDirectory();
      }
      // divido el bucket y vuelvo a insertar
      splitBucket(idx);
      insert(key, series); // después de dividir reinserto
    }
    else {
      bucket->insert(key, series);
    }
  }

  // busco un registro
  Series *search(int key) {
    int idx = hashFunction(key);
    return directory[idx]->search(key);
  }

  // elimina un registro
  void remove(int key) {
    int idx = hashFunction(key);
    Bucket *bucket = directory[idx];
    if (!bucket->remove(key)) {
      cout << "Registro no encontrado para eliminar: " << key << endl;
    }
  }

  // print añ directorio y los buckets
  void display() const {
    cout << "Estado del directorio (profundidad global: " << globalDepth << "):" << endl;
    int bitWidth = globalDepth;
    for (int i = 0; i < directory.size(); i++) {
      // representación binaria del índice
      string binary = bitset<32>(i).to_string(); // convierto a binario de 32 bits
      binary = binary.substr(32 - bitWidth);     // solo los bits relevantes

      cout << "Indice " << i << " (" << binary << ") (profundidad local: "
           << directory[i]->localDepth << "): ";

      if (directory[i]->items.empty()) {
        cout << "Vacio";
      }
      else {
        for (auto &item : directory[i]->items) {
          cout << "[" << item.first << ", " << item.second.title << "] ";
        }
      }
      cout << endl;
    }
    cout << "-----------------------------------" << endl;
  }

  // libero la memoria de los buckets
  ~ExtendibleHashing() {    
    vector<Bucket *> uniqueBuckets;
    for (auto bucket : directory)
    {
      if (find(uniqueBuckets.begin(), uniqueBuckets.end(), bucket) == uniqueBuckets.end())
      {
        uniqueBuckets.push_back(bucket);
        delete bucket;
      }
    }
  }
};

int main()
{
  string filename = "prueba.csv";
  vector<Series> seriesList = readCSV(filename, 7); // Leer solo los primeros 20 registros si no hay paso por valor

  ExtendibleHashing hashTable(2); // Tamaño máximo de la bucket es 2

  // asigno una clave única a cada serie
  int currentKey = 0;
  for (const auto &series : seriesList) {
    hashTable.insert(currentKey, series);
    currentKey++;
  }

  cout << "Despues de las inserciones:" << endl;
  hashTable.display();

  // buscar y mostrar una serie por su clave
  int searchKey = 1; 
  Series *result = hashTable.search(searchKey);
  if (result != nullptr) {
    cout << "Resultado de la busqueda para la clave " << searchKey << ":" << endl;
    result->show();
  }
  else {
    cout << "Serie no encontrada para la clave " << searchKey << "." << endl;
  }

  // elimino una serie por su clave
  int removeKey = 2;
  hashTable.remove(removeKey);
  cout << "Despues de eliminar la clave " << removeKey << ":" << endl;
  hashTable.display();

  // intento buscar la serie eliminada
  Series *postRemoveResult = hashTable.search(removeKey);
  if (postRemoveResult != nullptr) {
    cout << "Resultado de la busqueda para la clave " << removeKey << ":" << endl;
    postRemoveResult->show();
  }
  else {
    cout << "Serie no encontrada para la clave " << removeKey << " despues de la eliminacion." << endl;
  }

  return 0;
}
