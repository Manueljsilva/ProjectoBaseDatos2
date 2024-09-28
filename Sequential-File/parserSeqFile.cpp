#include <iostream>
#include <sstream>
#include <string>
#include "sequential.cpp"  // Incluir la definición de SequentialFile

using namespace std;

class Parser {
private:
    SequentialFile<string>* sf; // Apuntador a la instancia de SequentialFile
public:
    Parser() : sf(nullptr) {}

    // Función para interpretar y ejecutar comandos
    void parseCommand(const string& command) {
        istringstream iss(command);
        string operation;
        iss >> operation;

        if (operation == "create") {
            parseCreateTable(iss);
        } else if (operation == "insert") {
            parseInsert(iss);
        } else if (operation == "select") {
            parseSelect(iss);
        } else if (operation == "delete") {
            parseDelete(iss);
        } else {
            cout << "Comando no reconocido" << endl;
        }
    }

    void parseCreateTable(istringstream& iss) {
        string command, tableName, from, filePath, usingWord, index, hash;

        // Leer el comando "create table"
        iss >> command >> tableName >> from;

        // Validar que el siguiente token sea "from"
        if (from == "from") {
            // Leer el nombre del archivo entre comillas
            getline(iss, filePath, '\"'); // Leer hasta la primera comilla
            getline(iss, filePath, '\"'); // Leer el contenido entre comillas
            iss >> usingWord >> index;  // Leer las siguientes palabras: "using" y "index"

            // Leer el hash (que contendrá la declaración del índice)
            getline(iss, hash); // Leer el resto de la línea para obtener el hash
        }

        // Extraer el nombre de la columna del hash
        string columnName = hash.substr(hash.find("(") + 1, hash.find(")") - hash.find("(") - 1);
        columnName.erase(remove(columnName.begin(), columnName.end(), '"'), columnName.end()); // Eliminar comillas si hay

        cout << "Columna: " << columnName << endl; 
        cout << from << " " << filePath << " " << usingWord << " " << index << " " << hash << endl;

        // Validar la estructura del comando
        if (from == "from" && usingWord == "using" && index == "index") {
            cout << "Creando tabla: " << tableName << " desde el archivo: " << filePath << " usando " << columnName << " como clave." << endl;

            // Crear la instancia de SequentialFile y leer los registros
            sf = new SequentialFile<string> (tableName, columnName);
            vector<Registro> registros = readCSV(filePath, 1000); // Llama a la función para leer el CSV

            // Insertar los registros leídos en el archivo secuencial
            for (auto& reg : registros) {
                sf->add(reg);
            }
            cout << "Tabla creada e índices generados." << endl;
        } else {
            cout << "Error al parsear el comando 'create table'" << endl;
        }
    }


    void parseSelect(istringstream& iss) {
        string all, from, tableName, where, keyField, operatorType;

        // Leer los primeros tokens
        iss >> all >> from >> tableName >> where >> keyField >> operatorType;

        // Verificar la estructura del comando SELECT
        if (all == "*" && from == "from" && where == "where") {
            if (operatorType == "=") {
                string keyValue;
                // Leer el valor clave
                getline(iss >> ws, keyValue); // Elimina espacios en blanco y lee el resto de la línea
                keyValue = cleanField(keyValue); // Limpiar el valor

                // Verificar si la tabla está creada
                if (sf) {
                    Registro registro = sf->search(keyValue); // Buscar el registro
                    if (!string(registro.title).empty()) {
                        mostrarRegistro(registro);  // Mostrar el registro encontrado
                    } else {
                        cout << "Registro no encontrado" << endl; // Mensaje si no se encuentra el registro
                    }
                } else {
                    cout << "No hay ninguna tabla creada para buscar registros" << endl; // Mensaje si no hay tabla
                }

            } else if (operatorType == "between") {
                // Si el operador es "between", llamar a parseRangeSelect
                parseRangeSelect(iss);
            } else {
                cout << "Operador no soportado: " << operatorType << endl;
            }
        } else {
            cout << "Error al parsear el comando 'select *'" << endl; // Mensaje de error para estructura incorrecta
        }
    }

    void parseRangeSelect(istringstream& iss) {
        string keyStart, andWord, keyEnd;

        // Leer el valor inicial entre comillas
        getline(iss >> ws, keyStart, '"'); // Leer hasta la primera comilla de apertura
        getline(iss, keyStart, '"');       // Leer hasta la segunda comilla (el valor)

        // Leer la palabra 'and'
        iss >> andWord;

        // Verificar si la palabra es 'and'
        if (andWord != "and") {
            cout << "Error en la comparación. Se esperaba 'and'." << endl;
            return;
        }

        // Leer el valor final entre comillas
        getline(iss >> ws, keyEnd, '"');   // Leer hasta la primera comilla de apertura
        getline(iss, keyEnd, '"');         // Leer hasta la segunda comilla (el valor)

        // Limpiar los valores si es necesario
        keyStart = cleanField(keyStart);
        keyEnd = cleanField(keyEnd);

        // Verificar si la tabla está creada
        if (sf) {
            vector<Registro> registros = sf->rangeSearch(keyStart, keyEnd);  // Búsqueda por rango
            for (const auto& reg : registros) {
                mostrarRegistro(reg);  // Mostrar cada registro encontrado
            }
        } else {
            cout << "No hay ninguna tabla creada para buscar registros" << endl;
        }
    }

    void parseInsert(istringstream& iss) {
        string into, tableName, values;
        iss >> into >> tableName >> values;

        // Validar si la estructura del comando es correcta ("insert into" y "values")
        if (into == "into" && values == "values") {
            string nombre, releaseYear, runtime, genre, rating, cast, synopsis;

            // Leer y procesar cada campo usando la función leerCampo
            leerCampo(iss, nombre);        // Leer el campo "title"
            leerCampo(iss, releaseYear);   // Leer el campo "releaseYear"
            leerCampo(iss, runtime);       // Leer el campo "runtime"
            leerCampo(iss, genre, '"');    // Leer el campo "genre" (puede contener comas)
            leerCampo(iss, rating);        // Leer el campo "rating"
            leerCampo(iss, cast, '"');     // Leer el campo "cast" (puede contener comas)
            leerCampo(iss, synopsis, '"'); // Leer el campo "synopsis" (puede contener comas)

            // Crear un nuevo registro y copiar los valores a la estructura
            Registro registro;
            copiarCampo(registro.title, nombre, sizeof(registro.title));
            copiarCampo(registro.releaseYear, releaseYear, sizeof(registro.releaseYear));
            copiarCampo(registro.runtime, runtime, sizeof(registro.runtime));
            copiarCampo(registro.genre, genre, sizeof(registro.genre));
            copiarCampo(registro.rating, rating, sizeof(registro.rating));
            copiarCampo(registro.cast, cast, sizeof(registro.cast));
            copiarCampo(registro.synopsis, synopsis, sizeof(registro.synopsis));

            // Verificar si hay una tabla activa donde insertar el registro
            if (sf) {
                sf->add(registro);  // Insertar el registro en la estructura secuencial
                cout << "Registro insertado con éxito." << endl;
            } else {
                cout << "No hay ninguna tabla creada para insertar registros." << endl;
            }
        } else {
            // Error de sintaxis en el comando
            cout << "Error al parsear el comando 'insert into'. Se esperaba 'into' y 'values'." << endl;
        }
    }


    void parseDelete(istringstream& iss) {
        string from, tableName, where, keyField, equals, keyValue;

        // Leer las partes del comando
        iss >> from >> tableName >> where >> keyField >> equals;

        // Verificar si la siguiente parte tiene comillas para limpiar correctamente el campo
        getline(iss >> ws, keyValue);  // Leer el valor de la clave (puede tener espacios o comillas)
        keyValue = cleanField(keyValue);  // Limpiar el campo del valor clave

        // Validar la sintaxis del comando
        if (from == "from" && where == "where" && equals == "=") {
            // Verificar si la estructura secuencial (sf) está inicializada
            if (sf) {
                // Eliminar el registro usando el valor de la clave
                bool result = sf->removeKey(keyValue);
                if (result) {
                    cout << "Registro con clave '" << keyValue << "' eliminado con éxito." << endl;
                } else {
                    cout << "No se encontró ningún registro con la clave proporcionada: '" << keyValue << "'." << endl;
                }
            } else {
                cout << "No hay ninguna tabla creada para eliminar registros." << endl;
            }
        } else {
            // Error de sintaxis en el comando "delete"
            cout << "Error al parsear el comando 'delete from'. Sintaxis incorrecta." << endl;
        }
    }


};

int main() {
    Parser parser;

    // Crear tabla desde archivo CSV
    parser.parseCommand("create table parserTest from file \"TV Series_modificado.csv\" using index hash(\"title\")");

    // Insertar registro
    parser.parseCommand("insert into parserTest values \"Title Example\", 2023, 120, \"Action, Drama\", 8.5, \"Main Cast\", \"This is a synopsis\"");

    // Seleccionar un registro
    parser.parseCommand("select * from parserTest where title = \"Stranger Things\"");

    // Búsqueda por rango
    parser.parseCommand("select * from parserTest where title between \"A Court of Thorns and Roses\" and \"A Series of Unfortunate Events\"");

    // Eliminar un registro
    parser.parseCommand("delete from parserTest where title = \"Title Example\"");

    return 0;
}
