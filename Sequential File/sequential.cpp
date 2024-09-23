#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>

// #include "structs.h";
using namespace std;


struct Registro
{
    // estructura estatica de un registro prueba
    char codigo[5];
    char nombre[20];
    char apellidos[20];
    int ciclo;

    int posNext; // pos siguiente 0>= : posicion en aux o en dat,
    char nextEspacioType; // puede ser a: auxiliar o d: datos, e:elimnado
};

template <typename PK>
class SequentialFile{
private:
    string fileData;
    string nameKey;
    PK primaryKey;
public:
    SequentialFile(string _fileData, string nameKey;);

    bool add(Registro registro);

    void readCSV(string csvFile, PK key);

    Registro search(PK key);

    vector<Registro> rangeSearch(PK begin_key, PK end_key);

    bool remove(PK key);
private:
    void reconstruir();

    bool MayorQue(Registro r1, Registro r2, PK nameKey);

    PK getPrimaryKeyFromRegistro(Registro& reg, string nameKey) {
        if (nameKey == "codigo") {
            return string(reg.codigo);
        } else if (nameKey == "nombre") {
            return string(reg.nombre);
        } else if (nameKey == "apellidos") {
            return string(reg.apellidos);
        } else if (nameKey == "ciclo") {
            if constexpr (std::is_same<PK, int>::value) {
                return reg.ciclo;
            } else if constexpr (std::is_same<PK, string>::value) {
                return to_string(reg.ciclo);
            }
        }
        throw runtime_error("Clave primaria no válida");
    }
   
    int posRegistro(PK key);
};

template <typename PK>
bool SequentialFile<PK>::MayorQue(Registro r1, Registro r2, PK nameKey){
    PK key1 = getPrimaryKeyFromRegistro(r1, this->nameKey);
    PK key2 = getPrimaryKeyFromRegistro(r2, this->nameKey);
    return key1 > key2;
}

template <typename PK>
int SequentialFile<PK>::posRegistro(PK key) {
    ifstream file(fileData, ios::binary);
    int N;
    char nameKey[20];
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    int low = 0, high = N-1;
    int mid;
    Registro reg;

    int posPrev = -1;

    while (low <= high) {
        mid = (low + high) / 2;

        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg); // me ubico al medio
        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));

        if (reg.nextEspacioType == 'e'){
            low = mid + 1;
            continue;
        }

        PK registroKey = getPrimaryKeyFromRegistro<PK>(reg, nameKey);

        if (registroKey == key) {
            throw runtime_error("El registro ya existe");
        } else if (registroKey < key) {
            posPrev = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    file.close();
    return posPrev;
}

template <typename PK>
SequentialFile<PK>::SequentialFile(string _fileData, string nameKey) {
    // guardamos el nombre como un formato
    this->fileData = "seqFile_" + _fileData;

    // si no existe el archivo, se crea, pues se toma como vacio
    ifstream file(fileData, ios::binary);
    if (!file.is_open()){
        
        // fuerzo la creacion del archivo
        fstream file(fileData, ios::binary | ios::in | ios::out | ios::trunc);
        int N = 0; // cantidad de reg. en la parte ordenada del archivo.
        file.write(reinterpret_cast<char*>(&N), sizeof(N));
        char nameKeyArr[20] = {0};
        strncpy(nameKeyArr, nameKey.c_str(), sizeof(nameKeyArr)-1);
        file.write(reinterpret_cast<char*>(nameKeyArr), sizeof(nameKeyArr));
        file.close();
    }

    file.close();
};

template <typename PK>
bool SequentialFile<PK>::add(Registro registro){
    // considero que para un N>= 2 considero recien el espacio auxiliar
    fstream file(fileData, ios::binary | ios::in | ios::out);
    int N; // cantidad de reg. ordenados
    char nameKey[20]; // que atributo es la llave
    // Leemos el N
    file.seekg(0, ios::beg); // puntero de lectura al inicio
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));
    if (N == 0){
        N = 1;
        // indica que su siguiente libre esta al final
        registro.posNext = -1;
        registro.nextEspacioType = 'd';

        // escribir al final
        file.seekp(0, ios::beg); // puntero de escritura al final
        file.write(reinterpret_cast<char*>(&N), sizeof(int)); // actualizo el N
        file.seekp(0, ios::end);
        file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
        file.close();
        return true;
    } 
    
    // hay por lo menos un registro, se enlazan los registros  
    Registro r1;
    if(N == 1){
        // comparo
        
        file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));
        file.seekp(0, ios::beg);
        N = 2;
        file.write(reinterpret_cast<char*>(&N), sizeof(int));
        if (MayorQue(r1, registro, nameKey)){ 
            // si r1>registro segun el nameKey indicado
            // me muevo a la posicion de r1 en escritura para guardar registro
            registro.posNext = 2;
            registro.nextEspacioType = 'd';
            r1.posNext = -1;
            r1.nextEspacioType = 'd';
            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
            file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
            // aproveche el haberme movido al final
            file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
        } else {
            // sino, el registro puede ser escrito al final
            r1.posNext = 2;
            file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
            registro.posNext = -1;
            registro.nextEspacioType = 'd';
            file.seekp(0, ios::end);
            file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
        }
        file.close();
        return true;
    }
    // N ya es >= 2
    // busco el registro mas cercano al nuevo valor
    int pos = posRegistro(registro); // siempre es el menor ultimo o a si esta libre, inseta ahi

    file.seekg(sizeof(int) + sizeof(char[20]) + pos*sizeof(Registro)), ios::beg);
    file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));

    if (r1.nextEspacioType == 'a') {
        int prevPos = pos;
        int nextPos = r1.posNext + N;

        while (r1.nextEspacioType == 'a') {
            file.seekg(sizeof(int) + sizeof(char[20]) + nextPos * sizeof(Registro), ios::beg);
            file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));

            if (MayorQue(r1, registro, nameKey)) break;

            prevPos = nextPos;
            nextPos = r1.posNext + N;
        }
        // conectamos al puntero del registro anterior
        file.seekp(sizeof(int) + sizeof(char[20]) + prevPos * sizeof(Registro), ios::beg);
        Registro rPrev;
        file.read(reinterpret_cast<char*>(&rPrev), sizeof(Registro));

        rPrev.posNext = nextPos;  // Apunto el anterior al nuevo registro
        rPrev.nextEspacioType = 'a'; 

        // Escribir el registro previo actualizado
        file.seekp(sizeof(int) + sizeof(char[20]) + prevPos * sizeof(Registro), ios::beg);
        file.write(reinterpret_cast<char*>(&rPrev), sizeof(Registro));

        // Escribir el nuevo registro
        file.seekp(0, ios::end);
        int newPos = ((file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro)) - N;
        registro.posNext = r1.posNext;
        registro.nextEspacioType = r1.nextEspacioType;
        file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));

        // Actualizar el siguiente registro (si es necesario)
        r1.nextEspacioType = 'a';
        r1.posNext = newPos;
        file.seekp(sizeof(int) + sizeof(char[20]) + nextPos * sizeof(Registro), ios::beg);
        file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
    }
    else if (r1.nextEspacioType == 'd') {
        // actualizo el puntero al next del registro actual
        file.seekp(0, ios::beg);
        r1.posNext = (file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro); // posicion final del archivo
        r1.nextEspacioType = 'a';
        // actualizo el puntero del registro a insertar al siguiente registro en el archivo de datos ordenado
        registro.nextEspacioType = 'd';
        registro.posNext = pos + 1;
        // aprovecho que estoy al final e inserto el nuevo registro
        file.write(reinterpret_cast<char*>(&registro), sizeof(registro));
        // actualizo el registro anterior
        file.seekp(sizeof(int) + sizeof(char[20]) + pos*sizeof(Registro)), ios::beg);
        file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
    }

    // no inserta en los e.

    // obtengo los k elementos sin ordenar
    // total de bytes - bytes de cabeceras / bytes por registro - N
    int k = (int(file.tellp()) - int(sizeof(int) + sizeof(char[20]))) / int(sizeof(Registro)) - N;
    file.close();
    if(k > log2(N)) reconstruir();
    
    return false;
}