#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>

using namespace std;

struct Series {
  string title, releaseYear, runtime, genre, rating, cast, synopsis;

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

  int getSizeOfRecord() {
      return sizeof(short) + title.size() +
              sizeof(short) + releaseYear.size() +
              sizeof(short) + runtime.size() +
              sizeof(short) + genre.size() +
              sizeof(short) + rating.size() +
              sizeof(short) + cast.size() +
              sizeof(short) + synopsis.size();
  }

  // empaqueto los datos de la serie en un buffer
  void empaquetar(char* buffer, int sizeTotal) {
    stringstream stream;
    // empaqueto cada campo con su tamaño
    empaquetarCampo(stream, title);
    empaquetarCampo(stream, releaseYear);
    empaquetarCampo(stream, runtime);
    empaquetarCampo(stream, genre);
    empaquetarCampo(stream, rating);
    empaquetarCampo(stream, cast);
    empaquetarCampo(stream, synopsis);
    // copio el contenido del stream al buffer
    memcpy(buffer, stream.str().c_str(), sizeTotal);
  }

  // empaquetamiento unitario (cada campo)
  void empaquetarCampo(stringstream& stream, const string& campo) {
    short size_aux = campo.size();
    stream.write(reinterpret_cast<const char*>(&size_aux), sizeof(short));
    stream.write(campo.c_str(), size_aux);
  }

  // desempaqueta los datos desde un buffer
  void desempaquetar(char* buffer, int sizeTotal) {
    stringstream stream(string(buffer, sizeTotal));
    // desempaqueto cada campo
    desempaquetarCampo(stream, title);
    desempaquetarCampo(stream, releaseYear);
    desempaquetarCampo(stream, runtime);
    desempaquetarCampo(stream, genre);
    desempaquetarCampo(stream, rating);
    desempaquetarCampo(stream, cast);
    desempaquetarCampo(stream, synopsis);
  }

  // desempaquetamiento unitario (cada campo)
  void desempaquetarCampo(stringstream& stream, string& campo) {
      short size;
      stream.read(reinterpret_cast<char*>(&size), sizeof(short));
      char* campo_buffer = new char[size];
      stream.read(campo_buffer, size);
      campo_buffer[size] = '\0';  // Asegurar que termine en null
      campo = string(campo_buffer);
      delete[] campo_buffer;
  }
};

string cleanYear(string &yearField) {
  string cleaned = yearField;
  cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), [](unsigned char c) { 
                return !isalnum(c) && c != '(' && c != ')' && c != '-'; }),
                cleaned.end());

  // uso una expresión regular para capturar un rango de años o un año entre los paréntesis
  regex yearPattern(R"(\((\d{4})-?(\d{4})?\))");
  smatch match;
  
  if (regex_search(cleaned, match, yearPattern)) {
    return (match.size() > 2 && match[2].matched)
    ? match[1].str() + "-" + match[2].str() // si hay un rango de años
    : match[1].str(); // si es un solo año, devuelve solo el año sin paréntesis
  } 
  return cleaned;
}

// Limpia caracteres especiales y espacios en los campos
string cleanField(string& field) {
  string cleaned = field;
  cleaned.erase(cleaned.begin(), find_if(cleaned.begin(), cleaned.end(), [](unsigned char ch) {
    return !isspace(ch);
  }));
  cleaned.erase(find_if(cleaned.rbegin(), cleaned.rend(), [](unsigned char ch) {
    return !isspace(ch);
  }).base(), cleaned.end());

  if (!cleaned.empty() && cleaned.front() == '"' && cleaned.back() == '"') {
    cleaned = cleaned.substr(1, cleaned.size() - 2);
  }
  return cleaned;
}

// Función para leer y limpiar un campo desde un stream
void leerCampo(stringstream& ss, string& campo, char delimiter = ',') {
  string field;
  if (delimiter == '"') {
    getline(ss, field, '"'); // Leer hasta la primera comilla
    getline(ss, field, '"'); // Leer el contenido entre comillas
    ss.ignore(1);            // Ignorar la coma que sigue al campo
  } else {
    getline(ss, field, delimiter);
  }
  campo = cleanField(field);
}

// Función para leer y procesar un archivo CSV
vector<Series> readCSV(string& filename, int maxRecords = 20) {
  ifstream file(filename);
  string line;
  vector<Series> seriesList;
  int count = 0; // Contador para limitar el número de registros leídos

  if (!file.is_open()) {
      cerr << "Error: No se pudo abrir el archivo." << endl;
      return seriesList;
  }

  // Leer la primera línea (header)
  getline(file, line);

  while (getline(file, line) && count < maxRecords) {
    stringstream ss(line);
    Series series;
    string field;

    // Leer y limpiar cada campo usando comillas como delimitadores
    leerCampo(ss, series.title);
    leerCampo(ss, field);
    series.releaseYear = cleanYear(field);
    leerCampo(ss, series.runtime);
    leerCampo(ss, series.genre, '"'); // Género con comillas dobles
    leerCampo(ss, series.rating);
    leerCampo(ss, series.cast, '"'); // Cast con comillas dobles
    leerCampo(ss, series.synopsis, '"'); // Sinopsis con comillas dobles

    seriesList.push_back(series);
    count++;
  }

  file.close();
  return seriesList;
}