
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>
#include <cstring>
#include <cctype>

using namespace std;

struct TVSeriesRecord
{
  char title[100];
  char releaseYear[10];
  char runtime[10];
  char genre[100];
  char rating[5];
  char cast[200];
  char synopsis[400];

  void show() const
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

  // Load data from a file stream
  void load(std::fstream &file)
  {
    file.read(reinterpret_cast<char *>(title), sizeof(title));
    file.read(reinterpret_cast<char *>(releaseYear), sizeof(releaseYear));
    file.read(reinterpret_cast<char *>(runtime), sizeof(runtime));
    file.read(reinterpret_cast<char *>(genre), sizeof(genre));
    file.read(reinterpret_cast<char *>(rating), sizeof(rating));
    file.read(reinterpret_cast<char *>(cast), sizeof(cast));
    file.read(reinterpret_cast<char *>(synopsis), sizeof(synopsis));
  }

  // Save data to a file stream
  void save(std::fstream &file) const
  {
    file.write(reinterpret_cast<const char *>(title), sizeof(title));
    file.write(reinterpret_cast<const char *>(releaseYear), sizeof(releaseYear));
    file.write(reinterpret_cast<const char *>(runtime), sizeof(runtime));
    file.write(reinterpret_cast<const char *>(genre), sizeof(genre));
    file.write(reinterpret_cast<const char *>(rating), sizeof(rating));
    file.write(reinterpret_cast<const char *>(cast), sizeof(cast));
    file.write(reinterpret_cast<const char *>(synopsis), sizeof(synopsis));
  }

  // Save data to an output file stream
  void save(std::ofstream &file) const
  {
    file.write(reinterpret_cast<const char *>(title), sizeof(title));
    file.write(reinterpret_cast<const char *>(releaseYear), sizeof(releaseYear));
    file.write(reinterpret_cast<const char *>(runtime), sizeof(runtime));
    file.write(reinterpret_cast<const char *>(genre), sizeof(genre));
    file.write(reinterpret_cast<const char *>(rating), sizeof(rating));
    file.write(reinterpret_cast<const char *>(cast), sizeof(cast));
    file.write(reinterpret_cast<const char *>(synopsis), sizeof(synopsis));
  }


  // Get the key (identifier) for this record, cleaning spaces and null characters
    string get_key() const {
        string key = string(title);
        key.erase(remove_if(key.begin(), key.end(), ::isspace), key.end()); // Remove spaces
        key.erase(remove(key.begin(), key.end(), NULL), key.end());         // Remove null characters
        return key;
    }
};

string cleanYear(string &yearField)
{
  string cleaned = yearField;
  cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), [](unsigned char c)
                          { return !isalnum(c) && c != '(' && c != ')' && c != '-'; }),
                cleaned.end());

  // Uso de una expresión regular para capturar un rango de años o un año entre los paréntesis
  regex yearPattern(R"(\((\d{4})-?(\d{4})?\))");
  smatch match;

  if (regex_search(cleaned, match, yearPattern))
  {
    return (match.size() > 2 && match[2].matched)
               ? match[1].str() + "-" + match[2].str() // Si hay un rango de años
               : match[1].str();                       // Si es un solo año, devuelve solo el año sin paréntesis
  }
  return cleaned;
}

// Limpia caracteres especiales y espacios en los campos
string cleanField(string &field)
{
  string cleaned = field;
  // Eliminar espacios al inicio
  cleaned.erase(cleaned.begin(), find_if(cleaned.begin(), cleaned.end(), [](unsigned char ch)
                                         { return !isspace(ch); }));
  // Eliminar espacios al final
  cleaned.erase(find_if(cleaned.rbegin(), cleaned.rend(), [](unsigned char ch)
                        { return !isspace(ch); })
                    .base(),
                cleaned.end());

  // Eliminar comillas dobles si están al inicio y al final
  if (!cleaned.empty() && cleaned.front() == '"' && cleaned.back() == '"')
  {
    cleaned = cleaned.substr(1, cleaned.size() - 2);
  }
  return cleaned;
}

// Función para leer y limpiar un campo desde un stream
void leerCampo(stringstream &ss, string &campo, char delimiter = ',')
{
  string field;
  if (delimiter == '"')
  {
    getline(ss, field, '"'); // Leer hasta la primera comilla
    getline(ss, field, '"'); // Leer el contenido entre comillas
    ss.ignore(1);            // Ignorar la coma que sigue al campo
  }
  else
  {
    getline(ss, field, delimiter);
  }
  campo = cleanField(field);
}

// Función para truncar una cadena y rellenar con ceros si es necesario
void truncateAndCopy(char *destination, const string &source, size_t size)
{
  strncpy(destination, source.c_str(), size - 1);
  destination[size - 1] = '\0'; // Asegurar terminación nula
}

// Función para leer y procesar un archivo CSV
vector<TVSeriesRecord> readCSV(const string &filename, int maxRecords = 20)
{
  ifstream file(filename);
  string line;
  vector<TVSeriesRecord> registrosList;
  int count = 0; // Contador para limitar el número de registros leídos

  if (!file.is_open())
  {
    cerr << "Error: No se pudo abrir el archivo." << endl;
    return registrosList;
  }

  // Leer la primera línea (header)
  getline(file, line);

  while (getline(file, line) && count < maxRecords)
  {
    stringstream ss(line);
    TVSeriesRecord registro;
    string field;

    // Inicializar todos los campos a cadenas vacías
    memset(&registro, 0, sizeof(TVSeriesRecord));

    // Leer y limpiar cada campo usando comillas como delimitadores
    string temp;

    // Título
    leerCampo(ss, temp);
    truncateAndCopy(registro.title, temp, sizeof(registro.title));

    // Año de lanzamiento
    leerCampo(ss, temp);
    temp = cleanYear(temp);
    truncateAndCopy(registro.releaseYear, temp, sizeof(registro.releaseYear));

    // Duración
    leerCampo(ss, temp);
    truncateAndCopy(registro.runtime, temp, sizeof(registro.runtime));

    // Género
    leerCampo(ss, temp, '"');
    truncateAndCopy(registro.genre, temp, sizeof(registro.genre));

    // Calificación
    leerCampo(ss, temp);
    truncateAndCopy(registro.rating, temp, sizeof(registro.rating));

    // Reparto
    leerCampo(ss, temp, '"');
    truncateAndCopy(registro.cast, temp, sizeof(registro.cast));

    // Sinopsis
    leerCampo(ss, temp, '"');
    truncateAndCopy(registro.synopsis, temp, sizeof(registro.synopsis));

    registrosList.push_back(registro);
    count++;
  }

  file.close();
  return registrosList;
}

// int main() {
//   string csvFilename = "prueba.csv";

//   vector<TVSeriesRecord> registros = readCSV(csvFilename, 20);

//   cout << "Registros leídos del CSV:" << endl;
//   for (const auto &reg : registros)
//   {
//     reg.show();
//   }

//   return 0;
// }
