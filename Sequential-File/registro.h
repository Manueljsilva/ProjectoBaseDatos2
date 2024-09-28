#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <regex>
#include <algorithm>

using namespace std;


struct Registro {
    char title[100];         // Título de la serie
    char releaseYear[10];    // Año de lanzamiento
    char runtime[10];        // Duración
    char genre[100];          // Género
    char rating[5];          // Calificación
    char cast[200];          // Reparto
    char synopsis[400];      // Sinopsis

    int posNext;             // Posición del siguiente registro
    char nextEspacioType;    // Puede ser 'a' (auxiliar) o 'd' (datos), 'e' (eliminado)
};

void mostrarRegistro(Registro reg){
    cout << "Título: " << reg.title 
         << ", Año de lanzamiento: " << reg.releaseYear 
         << ", Duración: " << reg.runtime 
         << ", Género: " << reg.genre 
         << ", Calificación: " << reg.rating 
         << ", Reparto: " << reg.cast 
         << ", Sinopsis: " << reg.synopsis 
        << ", puntero: " << reg.posNext << reg.nextEspacioType << endl;
}

// struct Registro {
//     string title, releaseYear, runtime, genre, rating, cast, synopsis;
//     // para el puntero
//     int posNext;
//     char nextEspacioType;


//     void show() {
//         cout << "Title: " << title << endl;
//         cout << "Release Year: " << releaseYear << endl;
//         cout << "Runtime: " << runtime << endl;
//         cout << "Genre: " << genre << endl;
//         cout << "Rating: " << rating << endl;
//         cout << "Cast: " << cast << endl;
//         cout << "Synopsis: " << synopsis << endl;
//         cout << "Puntero: " << posNext << nextEspacioType << endl;
//         cout << "-----------------------------------" << endl;
//     }

//     int getSizeOfRecord() {
//         return sizeof(short) + title.size() +
//                 sizeof(short) + releaseYear.size() +
//                 sizeof(short) + runtime.size() +
//                 sizeof(short) + genre.size() +
//                 sizeof(short) + rating.size() +
//                 sizeof(short) + cast.size() +
//                 sizeof(short) + synopsis.size() +
//                 sizeof(posNext) + sizeof(nextEspacioType);
//     }

//     void empaquetar(char* buffer, int sizeTotal) {
//         stringstream stream;
//         // empaqueto cada campo con su tamaño
//         empaquetarCampo(stream, title);
//         empaquetarCampo(stream, releaseYear);
//         empaquetarCampo(stream, runtime);
//         empaquetarCampo(stream, genre);
//         empaquetarCampo(stream, rating);
//         empaquetarCampo(stream, cast);
//         empaquetarCampo(stream, synopsis);
        
//         stream.write(reinterpret_cast<const char*>(&posNext), sizeof(posNext));
//         stream.write(reinterpret_cast<const char*>(&nextEspacioType), sizeof(nextEspacioType));
        
//         // copio el contenido del stream al buffer
//         memcpy(buffer, stream.str().c_str(), sizeTotal);
//     }

//     void empaquetarCampo(stringstream& stream, const string& campo) {
//         short size_aux = campo.size();
//         stream.write(reinterpret_cast<const char*>(&size_aux), sizeof(short));
//         stream.write(campo.c_str(), size_aux);
//     }

//     // desempaqueta los datos desde un buffer
//     void desempaquetar(char* buffer, int sizeTotal) {
//         stringstream stream(string(buffer, sizeTotal));
//         // desempaqueto cada campo
//         desempaquetarCampo(stream, title);
//         desempaquetarCampo(stream, releaseYear);
//         desempaquetarCampo(stream, runtime);
//         desempaquetarCampo(stream, genre);
//         desempaquetarCampo(stream, rating);
//         desempaquetarCampo(stream, cast);
//         desempaquetarCampo(stream, synopsis);

//         stream.read(reinterpret_cast<char*>(&posNext), sizeof(posNext));
//         stream.read(reinterpret_cast<char*>(&nextEspacioType), sizeof(nextEspacioType));
//     }

//     // desempaquetamiento unitario (cada campo)
//     void desempaquetarCampo(stringstream& stream, string& campo) {
//         short size;
//         stream.read(reinterpret_cast<char*>(&size), sizeof(short));
//         char* campo_buffer = new char[size];
//         stream.read(campo_buffer, size);
//         campo_buffer[size] = '\0';  // Asegurar que termine en null
//         campo = string(campo_buffer);
//         delete[] campo_buffer;
//     }
// };

// Función para reemplazar espacios por '_'
void replaceSpaces(string& field) {
    replace(field.begin(), field.end(), ' ', '_');
}

// Función para limpiar caracteres especiales y reemplazar espacios
string cleanField(string& field) {
    string cleaned = field;
    
    // Eliminar espacios al principio y final
    cleaned.erase(cleaned.begin(), find_if(cleaned.begin(), cleaned.end(), [](unsigned char ch) {
        return !isspace(ch);
    }));
    cleaned.erase(find_if(cleaned.rbegin(), cleaned.rend(), [](unsigned char ch) {
        return !isspace(ch);
    }).base(), cleaned.end());

    // Si hay comillas al principio y al final, las quitamos
    if (!cleaned.empty() && cleaned.front() == '"' && cleaned.back() == '"') {
        cleaned = cleaned.substr(1, cleaned.size() - 2);
    }

    // Reemplazar los espacios por guiones bajos
    replaceSpaces(cleaned);

    return cleaned;
}

// Función para limpiar y formatear el campo de año
string cleanYear(string& yearField) {
    string cleaned = yearField;
    cleaned.erase(remove_if(cleaned.begin(), cleaned.end(), [](unsigned char c) { 
                return !isalnum(c) && c != '(' && c != ')' && c != '-'; }),
                cleaned.end());

    regex yearPattern(R"(\((\d{4})-?(\d{4})?\))");
    smatch match;

    if (regex_search(cleaned, match, yearPattern)) {
        return (match.size() > 2 && match[2].matched)
        ? match[1].str() + "-" + match[2].str()
        : match[1].str();
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



// Función para copiar strings a arreglos de caracteres de forma segura
void copiarCampo(char* destino, const string& origen, size_t size) {
    strncpy(destino, origen.c_str(), size - 1);
    destino[size - 1] = '\0'; // Asegurar que el string esté correctamente terminado en null
}

// Función para leer y procesar un archivo CSV
vector<Registro> readCSV(const string& filename, int maxRecords = 20) {
    ifstream file(filename);
    string line;
    vector<Registro> registros;
    int count = 0; // Contador para limitar el número de registros leídos

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo." << endl;
        return registros;
    }

    // Leer la primera línea (header)
    getline(file, line);

    while (getline(file, line) && count < maxRecords) {
        stringstream ss(line);
        Registro registro;
        string field;

        // Leer y limpiar cada campo
        leerCampo(ss, field); // Título
        copiarCampo(registro.title, field, sizeof(registro.title));

        leerCampo(ss, field); // Año de lanzamiento
        field = cleanYear(field);
        copiarCampo(registro.releaseYear, field, sizeof(registro.releaseYear));

        leerCampo(ss, field); // Duración
        copiarCampo(registro.runtime, field, sizeof(registro.runtime));

        leerCampo(ss, field, '"'); // Género (puede contener comas, por eso se usa comillas dobles)
        copiarCampo(registro.genre, field, sizeof(registro.genre));

        leerCampo(ss, field); // Calificación
        copiarCampo(registro.rating, field, sizeof(registro.rating));

        leerCampo(ss, field, '"'); // Reparto (puede contener comas, por eso se usa comillas dobles)
        copiarCampo(registro.cast, field, sizeof(registro.cast));

        leerCampo(ss, field, '"'); // Sinopsis (puede contener comas, por eso se usa comillas dobles)
        copiarCampo(registro.synopsis, field, sizeof(registro.synopsis));

        // Inicializar los campos adicionales
        registro.posNext = -1; // Inicialmente sin siguiente registro
        registro.nextEspacioType = 'd'; // Inicialmente tipo datos

        registros.push_back(registro);
        count++;
    }

    file.close();
    return registros;
}