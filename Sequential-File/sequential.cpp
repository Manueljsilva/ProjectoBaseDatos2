#include <iostream>
#include <vector>
#include <cstring>
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

void mostrarRegistro(Registro reg){
    cout << "Codigo: " << reg.codigo
        << ", Nombre: " << reg.nombre
        << ", Apellidos: " << reg.apellidos
        << ", Ciclo: " << reg.ciclo
        << ", puntero: " << reg.posNext << reg.nextEspacioType << endl;
}

template <typename PK>
class SequentialFile{
private:
    string fileData;
    string nameKey;
    PK primaryKey;
public:
    SequentialFile(string _fileData, string nameKey);

    bool add(Registro registro);

    void readCSV(string csvFile, PK key);

    Registro search(PK key);

    vector<Registro> rangeSearch(PK keyStart, PK KeyEnd);

    bool removeKey(PK key);
    
    void displayRecords();
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
        cout << "Error: clave no existe, se ha puesto codigo como clave" << endl;
        return string(reg.codigo);
    }
   
    int posRegistro(PK key);

    int minRegistro(PK key);

    int minRegistroRemove(PK key);
};

template <typename PK>
void SequentialFile<PK>::reconstruir() {
    ifstream fileIn(fileData, ios::binary);
    if (!fileIn.is_open()) {
        throw runtime_error("No se pudo abrir el archivo para reconstruir.");
    }

    int N;
    char nameKey[20];
    fileIn.read(reinterpret_cast<char*>(&N), sizeof(int));
    fileIn.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    vector<Registro> registros;

    // lectura en la parte ordenada y auxiliar
    Registro reg;
    // tengo que asegurar que el primer registro nunca este marcado como e, para poder empezar el enlazamiento
    fileIn.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
    cout << "Test Reconstuir " << endl;
    cout << "reg: ";
    mostrarRegistro(reg);
    while (true) {

        if (reg.nextEspacioType != 'e') { // aseguro que lea un e
            registros.push_back(reg);
        }

        if (reg.nextEspacioType == 'a') {
            // lectura a su enlazado, recorre hasta que el proximo sea un d
            int j = N + reg.posNext;
            while (reg.nextEspacioType != 'd') {
                fileIn.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * j, ios::beg);
                fileIn.read(reinterpret_cast<char*> (&reg), sizeof(Registro));
                cout << "reg: ";
                mostrarRegistro(reg);
                registros.push_back(reg);

                j = N + reg.posNext;
            }
            if (reg.posNext == -1) break; // quiere decir que ya leyo el ultimo registro
        } // si salgo de el proximo apunta a una direccion en d, ej: 3d
        
        // actualizo el registro
        fileIn.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * reg.posNext, ios::beg);
        fileIn.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
        cout << "reg: ";
        mostrarRegistro(reg);
        if (reg.posNext == -1){
            registros.push_back(reg);
            break;
        }
        // 5a
    };

    fileIn.close();

    ofstream fileOut("tempfile.bin", ios::binary | ios::trunc);
    if (!fileOut.is_open()) {
        throw runtime_error("No se pudo crear el archivo temporal.");
    }

    N = registros.size();
    fileOut.write(reinterpret_cast<char*>(&N), sizeof(int));
    fileOut.write(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));
    for (int i = 0; i < registros.size() - 1; ++i) {
        // actualizo los punteros mientras escribo
        registros[i].posNext = i + 1;
        registros[i].nextEspacioType = 'd';
        fileOut.write(reinterpret_cast<char*>(&registros[i]), sizeof(Registro));
    }

    // para el ultimo registro
    registros.back().posNext = -1;
    registros.back().nextEspacioType = 'd';
    fileOut.write(reinterpret_cast<char*>(&registros.back()), sizeof(Registro));

    fileOut.close();
    // eliminacion del registro y remplazo por el temp
    remove(fileData.c_str());
    rename("tempfile.bin", fileData.c_str());
}

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

        PK registroKey = getPrimaryKeyFromRegistro(reg, nameKey);

        if (registroKey == key) {
            return -2;
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
int SequentialFile<PK>::minRegistro(PK key) {
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

        PK registroKey = getPrimaryKeyFromRegistro(reg, nameKey);

        if (registroKey == key) {
            return mid;
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
int SequentialFile<PK>::minRegistroRemove(PK key) {
    ifstream file(fileData, ios::binary);
    int N;
    char nameKey[20];
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    int low = 0, high = N-1;
    int mid;
    Registro regDelete;
    PK registroKey;

    int posPrev = -1;

    while (low <= high) {
        mid = (low + high) / 2;

        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg); // me ubico al medio
        file.read(reinterpret_cast<char*>(&regDelete), sizeof(Registro));

        if (regDelete.nextEspacioType == 'e'){
            low = mid + 1;
            continue;
        }

        registroKey = getPrimaryKeyFromRegistro(regDelete, nameKey);
        if (registroKey == key) {
            return mid;
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
SequentialFile<PK>::SequentialFile(string _fileData, string nameKey) : nameKey(nameKey){
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
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo");
    }
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
            registro.posNext = 1;
            registro.nextEspacioType = 'd';
            r1.posNext = -1;
            r1.nextEspacioType = 'd';
            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
            file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
            // aproveche el haberme movido al final
            file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
        } else {
            // sino, el registro puede ser escrito al final
            r1.posNext = 1;
            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
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
    PK keyRegistro = getPrimaryKeyFromRegistro(registro, nameKey);
    int pos = posRegistro(keyRegistro); // siempre es el menor ultimo o a si esta libre, inseta ahi
    if (pos == -2){
        cout << "ERROR: REGISTRO YA EXISTE Y HA SIDO OMITIDO"<< endl;
        return false;
    }
    file.seekg(sizeof(int) + sizeof(char[20]) + pos*sizeof(Registro), ios::beg);
    file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));

    if (r1.nextEspacioType == 'a') {
        int prevPos = pos; // 1
        int nextPos = r1.posNext + N; // 3

        while (r1.nextEspacioType == 'a') {
            if (MayorQue(r1, registro, nameKey)) break;

            file.seekg(sizeof(int) + sizeof(char[20]) + nextPos * sizeof(Registro), ios::beg);
            file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));
            
            PK pk1, pk2;
            pk1 = getPrimaryKeyFromRegistro(r1, nameKey);
            pk2 = getPrimaryKeyFromRegistro(registro, nameKey);
            if(pk1 == pk2){
                cout << "ERROR: REGISTRO YA EXISTE Y HA SIDO OMITIDO"<< endl;
                return false;
            }
            
            prevPos = nextPos; // 3
            nextPos = r1.posNext + N; // 5
        }
        // conectamos al puntero del registro anterior
        // file.seekp(sizeof(int) + sizeof(char[20]) + prevPos * sizeof(Registro), ios::beg);
        // Registro rPrev;
        // file.read(reinterpret_cast<char*>(&rPrev), sizeof(Registro));

        // rPrev.posNext = nextPos - N;  // Apunto el anterior al nuevo registro
        // rPrev.nextEspacioType = 'a'; 
        file.seekp(0, ios::end);
        int newPos = ((file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro)) - N;
        
        // Escribir el nuevo registro
        registro.posNext = r1.posNext;
        registro.nextEspacioType = r1.nextEspacioType;
        file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));

        r1.posNext = newPos;
        r1.nextEspacioType = 'a';

        // Escribir el registro previo actualizado
        file.seekp(sizeof(int) + sizeof(char[20]) + prevPos * sizeof(Registro), ios::beg);
        file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));

    }
    else if (r1.nextEspacioType == 'd') {
        
        // actualizo el puntero del registro a insertar al siguiente registro en el archivo de datos ordenado

        registro.nextEspacioType = 'd';
        registro.posNext = r1.posNext;
        file.seekp(0, ios::end);
        if (r1.posNext == -1 && (N) == (file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro)) {
            N++;
            file.seekp(0, ios::beg);
            file.write(reinterpret_cast<char*>(&N), sizeof(N));
            file.seekp(0, ios::end);
            r1.posNext = (file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro);
            r1.nextEspacioType == 'd';
            
        } else {
            // actualizo el puntero al next del registro actual
            file.seekp(0, ios::end);
            r1.posNext = (file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro) - N; // posicion final del archivo
            r1.nextEspacioType = 'a';
        }
        // aprovecho que estoy al final e inserto el nuevo registro
        file.seekp(0, ios::end);
        file.write(reinterpret_cast<char*>(&registro), sizeof(registro));
        // actualizo el registro anterior
        file.seekp(sizeof(int) + sizeof(char[20]) + pos*sizeof(Registro), ios::beg);
        file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
    }

    // // no inserta en los e.

    // // obtengo los k elementos sin ordenar
    // // total de bytes - bytes de cabeceras / bytes por registro - N
    file.seekp(0, ios::end);
    int k = (int(file.tellp()) - int(sizeof(int) + sizeof(char[20]))) / int(sizeof(Registro)) - N;
    file.close();
    if(k > log2(N)) reconstruir();
    
    return false;
}

template <typename PK>
Registro SequentialFile<PK>::search(PK key) {
    // tengo que reconstruir para aprovechar la busqueda binaria
    reconstruir();
    ifstream file(fileData, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo");
    }

    int N;
    char nameKey[20];
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    int low = 0, high = N - 1;
    int mid;
    Registro reg;

    while (low <= high) {
        mid = (low+high)/2;

        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg);
        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));

        if (reg.nextEspacioType == 'e') {
            low = mid + 1;
            continue;
        }

        PK registroKey = getPrimaryKeyFromRegistro(reg, nameKey);

        if (registroKey == key){
            file.close();
            return reg;
        }
        else if (registroKey < key) {
            low = mid + 1;
        }
        else {
            high = mid - 1;
        }
    }
    
    file.close();
    cout << "ERROR: REGISTRO NO EXISTE" << endl;
    return Registro();
};

template <typename PK>
vector<Registro> SequentialFile<PK>::rangeSearch(PK keyStart, PK keyEnd) {
    vector<Registro> result;
    ifstream file(fileData, ios::binary);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo para rangeSearch.");
    }

    int N;
    char nameKey[20];
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    int pos = minRegistro(keyStart);
    if (pos == -1) {
        pos = 0;
    }

    Registro reg;
    file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * pos, ios::beg);
    file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
    PK registroKey;
    while (true)
    {
        registroKey = getPrimaryKeyFromRegistro(reg, nameKey);

        // Si la clave está dentro del rango, lo añadimos
        if (registroKey >= keyStart && registroKey <= keyEnd && reg.nextEspacioType != 'e') {
            result.push_back(reg);
        }

        // Si la clave es mayor que el keyEnd, terminamos la búsqueda
        if (registroKey > keyEnd) {
            break;
        }
        // Avanzar al siguiente registro
        if (reg.nextEspacioType == 'a' && reg.posNext != -1) {
            // Si apunta a la parte auxiliar
            int auxPos = N + reg.posNext;
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * auxPos, ios::beg);
        } else if (reg.posNext != -1) {
            // Si apunta a la parte ordenada
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * reg.posNext, ios::beg);
        } else {
            break;  // Si no hay más registros
        }

        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
    }

    file.close();
    return result;
}

template <typename PK>
bool SequentialFile<PK>::removeKey(PK key) {
    fstream file(fileData, ios::binary | ios::in | ios::out);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo para eliminar.");
    }

    int N;
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.seekg(sizeof(int) + sizeof(char[20]), ios::beg);

    // Encontrar la posición del registro más cercano
    int posPrev = minRegistroRemove(key);
    if (posPrev == -1) { // registro a eliminar no existe
        file.close();
        return false;
    }
    cout << "Posicion encontrada: " << posPrev << endl; 
    Registro prevReg, reg;
    int posCurrent;

    file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
    file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
    PK pk1;
    pk1 = getPrimaryKeyFromRegistro(prevReg, nameKey);
    if (pk1 != key){
        // el valor a eliminar no lo encontro
        if (prevReg.nextEspacioType == 'd'){
            // el registro no existe
            file.close();
            return false;
        }
        else if (prevReg.nextEspacioType == 'a'){
            // el registro puede existir en un espacio auxiliar
            int aux = posPrev; // para guardar la posicion anterior
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (prevReg.posNext + N), ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
            pk1 = getPrimaryKeyFromRegistro(prevReg, nameKey);
            while(pk1 != key){
                if (prevReg.nextEspacioType == 'd') {
                    // no lo encontro en la parte enlazada
                    file.close();
                    return false;
                }
                // lo busco en todos sus enlazados
                aux = prevReg.posNext + N;
                file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * aux, ios::beg);
                file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
                pk1 = getPrimaryKeyFromRegistro(prevReg, nameKey);
            }
            // devuelvo al prevReg a su anterior
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * aux, ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
        }
    } else {
        // si lo encontro en la parte ordenada, busco su anterior
        if (posPrev != 0) { // indica que el registro a encontrar es el registro y tengo que volver atras
            int aux=1;
            file.seekg(sizeof(int) + sizeof(char[20]) + (posPrev - aux)*sizeof(Registro), ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
            mostrarRegistro(prevReg);
            while(prevReg.nextEspacioType == 'e'){
                aux++;
                file.seekg(sizeof(int) + sizeof(char[20]) + (posPrev - aux)*sizeof(Registro), ios::beg);
                file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
            }
            posPrev = posPrev - aux;
        } else {
            // Leer el registro en posPrev
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
        }
    }

    
    mostrarRegistro(prevReg);
    // Proceso para encontrar el registro a eliminar:
    while (true) {
        if (prevReg.nextEspacioType == 'd' && prevReg.posNext != -1) {
            posCurrent = prevReg.posNext; // 3
        } else if (prevReg.nextEspacioType == 'a') {
            posCurrent = N + prevReg.posNext;
        } else {
            file.close();
            return false;  // No hay más registros que seguir o no se encontró
        }

        // Leer el registro apuntado por prevReg
        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posCurrent, ios::beg);
        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
        mostrarRegistro(reg);
        // Comparar la clave para ver si es el que queremos eliminar
        PK registroKey = getPrimaryKeyFromRegistro(reg, nameKey);
        if (registroKey == key) {
            break;  // Encontramos el registro a eliminar
        }
        else {
            if (prevReg.nextEspacioType == 'd'){
                break;
            }
        }

        // Si no es, seguimos recorriendo
        prevReg = reg;
        posPrev = posCurrent;
    }
    cout << "posPrev: "<< posPrev << endl;
    // Manejo del primer registro
    if (posPrev == 0) {
        mostrarRegistro(reg);
        // Si el registro a eliminar es el primero y hay uno auxiliar que lo puede reemplazar
        if (prevReg.nextEspacioType == 'a' && prevReg.posNext != -1) {
            // Cargar el registro auxiliar
            int auxIndex = N + reg.posNext;  // Índice del registro auxiliar
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * auxIndex, ios::beg);
            file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));

            // Sobrescribir el primer registro con el registro auxiliar
            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
            file.write(reinterpret_cast<char*>(&reg), sizeof(Registro));
            prevReg.posNext = reg.posNext;
            prevReg.nextEspacioType = reg.nextEspacioType;
            file.seekp(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
            file.write(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
        } else {
            // Si no hay un auxiliar, se chanca con el siguiente
            file.seekp(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
            file.write(reinterpret_cast<char*>(&reg), sizeof(Registro));
        }
    } else {
        // Actualizar el puntero del registro anterior (prevReg) para saltar el registro eliminado
        prevReg.posNext = reg.posNext;
        prevReg.nextEspacioType = reg.nextEspacioType;
        mostrarRegistro(prevReg);
        cout << "Posicion de escritura: " << posCurrent << endl;
        file.seekp(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
        file.write(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
    }

    // Ahora estamos en el registro a eliminar (reg) y prevReg es el registro anterior
    // Marcar el registro como eliminado ('e')
    reg.nextEspacioType = 'e';
    mostrarRegistro(reg);
    file.seekp(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posCurrent, ios::beg);
    file.write(reinterpret_cast<char*>(&reg), sizeof(Registro));

    file.close();
    return true;
}

template <typename PK>
void SequentialFile<PK>::displayRecords() {
    fstream file(fileData, ios::binary | ios::in);
    if (!file.is_open()) {
        throw runtime_error("No se pudo abrir el archivo para mostrar registros.");
    }

    int N;
    char nameKey[20];
    file.read(reinterpret_cast<char*>(&N), sizeof(int));
    file.read(reinterpret_cast<char*>(&nameKey), sizeof(char[20]));

    Registro reg;
    cout << "---------------------------------\n";
    cout << "Registros en la parte ordenada:\n";
    for (int i = 0; i < N; ++i) {
        
        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * i, ios::beg);
        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));


        cout << i << ": " << reg.codigo << ", " << reg.nombre << " puntero = " << reg.posNext << reg.nextEspacioType << endl;
    }

    cout << "Registros en la parte Auxiliar:\n";
    int i = 0;
    file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (N + i), ios::beg);
    while (file.read(reinterpret_cast<char*>(&reg), sizeof(Registro))) {  // Asumiendo que MAX_AUXILIAR es la cantidad de registros auxiliares
        cout << "Aux " << i << " : " << reg.codigo << ", " << reg.nombre << " puntero = " << reg.posNext << reg.nextEspacioType << std::endl;
        i++;
        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (N + i), ios::beg);
    }
    cout << "---------------------------------\n";

    file.close();
}

int main () 
{
    SequentialFile<string> seqFile("registros.dat", "codigo");

    // Registro reg1 = {"0001", "Analiz", "Perez", 5, -1, 'd'};
    // Registro reg2 = {"0003", "Ana", "Gomez", 3, -1, 'd'};
    // Registro reg3 = {"0010", "Carlos", "Mendez", 2, -1, 'd'};
    // Registro reg4 = {"0006", "Mario", "Zapata", 8, -1, 'd'};
    // Registro reg5 = {"0007", "Elsa", "Vargas", 8, -1, 'd'};

    // Registro reg6 = {"0015", "Paola", "Vargas", 8, -1, 'd'};
    // Registro reg7 = {"0004", "Diana", "Vargas", 8, -1, 'd'};
    // Registro reg8 = {"0005", "David", "Vargas", 8, -1, 'd'};

    // Registro reg9 = {"0020", "Mishelle", "Vargas", 8, -1, 'd'};
    // Registro reg10 = {"0030", "Joaquin", "Vargas", 8, -1, 'd'};
    // Registro reg11 = {"0017", "Kevin", "Vargas", 8, -1, 'd'};

    // // test de add
    // seqFile.add(reg1);
    // seqFile.add(reg2);
    // seqFile.add(reg3);
    // seqFile.add(reg4);
    // seqFile.displayRecords();
    // seqFile.add(reg5);
    // seqFile.displayRecords();
    // seqFile.add(reg6);
    // seqFile.displayRecords();
    // seqFile.add(reg7);
    // seqFile.displayRecords();
    // seqFile.add(reg8);
    // seqFile.displayRecords();
    // seqFile.add(reg9);
    // seqFile.displayRecords();
    // seqFile.add(reg10);
    // seqFile.displayRecords();
    // seqFile.add(reg11);
    seqFile.displayRecords();

    // tests de busqueda
    // cout << "\nTests de Busqueda\n";
    // Registro reg = seqFile.search("0009");
    // mostrarRegistro(reg);

    // tests de rangeSearch
    // cout << "\nTest de RangeSearch\n";
    // vector<Registro> regs = seqFile.rangeSearch("0005", "0018");
    // for (auto reg: regs)
    //     mostrarRegistro(reg);

    // test de eliminacion
    cout << "\nTest de remove\n";
    bool resultadoRemove = seqFile.removeKey("0017");
    if (resultadoRemove == false){
        cout << "Key no encontrada para el remove" << endl;
    }
    seqFile.displayRecords();

    return 0;
}