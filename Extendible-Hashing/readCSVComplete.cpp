#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>

using namespace std;

// Estructura para encapsular los datos de cada serie
struct Series
{
  string title;
  string releaseYear;
  string runtime;
  string genre;
  string rating;
  string cast;
  string synopsis;

  // Método para mostrar los datos de la serie
  void show()
  {
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

// Función para limpiar caracteres especiales y extraer correctamente el año
string cleanYear(const string &yearField)
{
  string cleaned = yearField;
  cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), [](unsigned char c)
                          { return !isalnum(c) && c != '(' && c != ')' && c != '-'; }),
                cleaned.end());

  // Expresión regular para capturar los años entre paréntesis
  regex yearPattern(R"(\((\d{4})-?(\d{4})?\))");
  smatch match;
  if (regex_search(cleaned, match, yearPattern))
  {
    if (match.size() > 2 && match[2].matched)
    {
      return match[1].str() + "-" + match[2].str();
    }
    else
    {
      return match[1].str();
    }
  }
  return cleaned;
}

// Función para limpiar las comillas de un campo
string cleanField(const string &field)
{
  string cleaned = field;
  // Eliminar espacios en blanco al inicio y al final
  cleaned.erase(cleaned.begin(), find_if(cleaned.begin(), cleaned.end(), [](unsigned char ch){ return !isspace(ch); }));
  cleaned.erase(find_if(cleaned.rbegin(), cleaned.rend(), [](unsigned char ch)
                        { return !isspace(ch); })
                    .base(),
                cleaned.end());

  if (!cleaned.empty() && cleaned.front() == '"' && cleaned.back() == '"')
  {
    cleaned = cleaned.substr(1, cleaned.size() - 2); // Remover comillas al inicio y fin
  }
  return cleaned;
}

// Función para leer y procesar el CSV, limitando a los primeros 20 registros
vector<Series> readCSV(const string &filename, int maxRecords = 20)
{
  ifstream file(filename);
  string line;
  vector<Series> seriesList;
  int count = 0;

  if (!file.is_open())
  {
    cerr << "Error: No se pudo abrir el archivo." << endl;
    return seriesList;
  }

  // Leer la primera línea (header)
  getline(file, line);

  // Leer y procesar cada línea del archivo hasta el límite
  while (getline(file, line) && count < maxRecords)
  {
    stringstream ss(line);
    Series series;
    string field;

    // Leer y limpiar cada campo
    getline(ss, field, ',');
    series.title = cleanField(field);

    getline(ss, field, ',');
    series.releaseYear = cleanYear(field);

    getline(ss, field, ',');
    series.runtime = cleanField(field);

    // Leer campo de género (delimitado por comillas dobles)
    getline(ss, field, '"');
    getline(ss, field, '"');
    series.genre = field;

    ss.ignore(1); // Ignorar la coma que sigue al campo de género

    getline(ss, field, ',');
    series.rating = cleanField(field);

    // Leer el campo de cast (delimitado por comillas dobles)
    getline(ss, field, '"');
    getline(ss, field, '"');
    series.cast = field;

    ss.ignore(1); // Ignorar la coma que sigue al campo de cast

    // Leer el campo de sinopsis (delimitado por comillas dobles)
    getline(ss, field, '"');
    getline(ss, field, '"');
    series.synopsis = field;

    seriesList.push_back(series);
    count++;
  }

  file.close();
  return seriesList;
}

int main()
{
  string filename = "TVSeriesPrueba.csv";
  vector<Series> seriesList = readCSV(filename, 20);

  // Mostrar las series procesadas
  for (auto &series : seriesList) {
    series.show();
  }

  return 0;
}
