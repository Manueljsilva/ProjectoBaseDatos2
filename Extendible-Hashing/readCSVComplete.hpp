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