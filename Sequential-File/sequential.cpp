#include <iostream>
#include <vector>
#include <cstring>
#include <cmath>
#include <fstream>

#include <chrono>

#include "registro.h"
using namespace std;

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
        if (nameKey == "title") {
            return string(reg.title);
        } else if (nameKey == "releaseYear") {
            return string(reg.releaseYear);
        }
        cout << "Error: clave no existe, se ha puesto title como clave" << endl;
        return string(reg.title);
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

                registros.push_back(reg);

                j = N + reg.posNext;
            }
            if (reg.posNext == -1) break; // quiere decir que ya leyo el ultimo registro
        } // si salgo de el proximo apunta a una direccion en d, ej: 3d
        
        // actualizo el registro
        fileIn.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * reg.posNext, ios::beg);
        fileIn.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
;
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
    
    // Si PK es un char[], usar strcmp
    if constexpr (std::is_same<PK, char*>::value) {
        int comparison = strcmp(key1, key2);  // strcmp para comparar char[]
        return comparison > 0;
    } 
    // Si PK es std::string
    else if constexpr (std::is_same<PK, std::string>::value) {
        return key1 > key2;  // El operador > funcionará con std::string
    }
    
    return false;  // Valor por defecto si no es ninguno de los dos tipos

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
    this->fileData = "seqFile_" + _fileData + "_" + nameKey;

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
        // cout << "N  == 1 :" << endl;
        // mostrarRegistro(r1);
        // mostrarRegistro(registro);
        
        // actalizacion de N a 2
        file.seekp(0, ios::beg);
        N = 2;
        file.write(reinterpret_cast<char*>(&N), sizeof(int));
        // cout << "Resultado de comparacion: " << MayorQue(r1, registro, nameKey) << endl;
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
    // cout << "keYRegistro obtenido: " << keyRegistro << endl;
    int pos = posRegistro(keyRegistro); // siempre es el menor ultimo o a si esta libre, inseta ahi

    if (pos == -2){
        cout << "ERROR: REGISTRO YA EXISTE Y HA SIDO OMITIDO"<< endl;
        return false;
    }
    if (pos == -1) pos = 0;
    // cout << "Pos obtenido: " << pos << endl;
    file.seekg(sizeof(int) + sizeof(char[20]) + pos*sizeof(Registro), ios::beg);
    file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));\

    // cout << "regLeido -> " << endl;
    // mostrarRegistro(r1);

    if (r1.nextEspacioType == 'a') {
        int prePrevPos = -1;
        int prevPos = pos; // 0
        int nextPos = r1.posNext + N; // 4

        while (r1.nextEspacioType == 'a') {
            if (MayorQue(r1, registro, nameKey)) break;

            file.seekg(sizeof(int) + sizeof(char[20]) + nextPos * sizeof(Registro), ios::beg);
            file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));

            // cout << "actualizacion del registro" << endl;
            // mostrarRegistro(r1);
            
            PK pk1, pk2;
            pk1 = getPrimaryKeyFromRegistro(r1, nameKey);
            pk2 = getPrimaryKeyFromRegistro(registro, nameKey);

            if(pk1 == pk2){
                cout << "ERROR: REGISTRO YA EXISTE Y HA SIDO OMITIDO"<< endl;
                return false;
            }

            prePrevPos = prevPos;
            prevPos = nextPos; // 5
            if(r1.nextEspacioType == 'd') nextPos = r1.posNext; // 1
            else nextPos = r1.posNext + N;
        }

        // conectamos al puntero del registro anterior
        file.seekp(0, ios::end);
        int newPos = ((file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro)) - N;
        
        if (prevPos == 0 && MayorQue(r1, registro, nameKey)) {
            registro.posNext = newPos;
            registro.nextEspacioType = 'a';

            // el r1 mantiene su enlace
            file.seekp(0, ios::end);
            file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));

            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
            file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
        }
        else {
            // Escribir el nuevo registro
            if (!MayorQue(r1, registro, nameKey)){         
                registro.posNext = r1.posNext;
                registro.nextEspacioType = r1.nextEspacioType;

                file.seekp(0, ios::end);
                file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));

                r1.posNext = newPos;
                r1.nextEspacioType = 'a';

                // Escribir el registro previo actualizado
                file.seekp(sizeof(int) + sizeof(char[20]) + prevPos * sizeof(Registro), ios::beg);
                file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
            } else {
                if (prevPos >= N) prevPos = prevPos - N;
                registro.posNext = prevPos;
                registro.nextEspacioType = 'a';

                // cout << "registro a escribir en la posicion:" << newPos + N << endl;
                // mostrarRegistro(registro);
                file.seekp(0, ios::end);
                file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));

                file.seekg(sizeof(int) + sizeof(char[20]) + prePrevPos * sizeof(Registro), ios::beg);
                file.read(reinterpret_cast<char*>(&r1), sizeof(Registro));
                
                // cout << "registro a escribir en la posicion:" << prePrevPos << endl;
                // mostrarRegistro(registro);
                r1.posNext = newPos;
                file.seekp(sizeof(int) + sizeof(char[20]) + prePrevPos * sizeof(Registro), ios::beg);
                file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
            }
        }
    }
    else if (r1.nextEspacioType == 'd') {
        if (pos == 0 && MayorQue(r1, registro, nameKey)){
            // sobreponer el registro en la posicion inicial
            file.seekp(0, ios::end);
            registro.posNext = (file.tellp() - sizeof(int) - sizeof(char[20])) / sizeof(Registro) - N;
            registro.nextEspacioType = 'a';
            
            file.write(reinterpret_cast<char*>(&r1), sizeof(Registro));
            file.seekp(sizeof(int) + sizeof(char[20]), ios::beg);
            file.write(reinterpret_cast<char*>(&registro), sizeof(Registro));
        }
        else {
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
    // reconstruir();
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
    PK registroKey;

    while (low <= high) {
        mid = (low+high)/2;

        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg);
        file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));

        if (reg.nextEspacioType == 'e') {
            low = mid + 1;
            continue;
        }

        registroKey = getPrimaryKeyFromRegistro(reg, nameKey);

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
    
    // si no lo encuentra, puede estar en los enlazados del anterior
    mid--;
    file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg);
    file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
    while(reg.nextEspacioType == 'e'){
        mid--;
        if(mid < 0){
            file.close();
            cout << "ERROR: REGISTRO NO EXISTE" << endl;
            return Registro();
        }
    }
    // mostrarRegistro(reg);
    if (reg.nextEspacioType == 'a') {
        // aun puede existir en sus enlazados siguientes
        while(reg.nextEspacioType != 'd') {
            registroKey = getPrimaryKeyFromRegistro(reg, nameKey);
            // busca en los enlazados
            if (registroKey == key) {
                file.close();
                return reg;
            }
            mid = reg.posNext + N;
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro)*mid, ios::beg);
            file.read(reinterpret_cast<char*>(&reg), sizeof(Registro));
            // mostrarRegistro(reg);
        }
        // busca en el ultimo enlazado
        registroKey = getPrimaryKeyFromRegistro(reg, nameKey);
        if (registroKey == key) {
            file.close();
            return reg;
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
    // cout << "Posicion encontrada: " << posPrev << endl; 
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
            int aux = posPrev, aux2 = prevReg.posNext + N; // para guardar la posicion anterior
            // cout << "aux inicial: " << aux << endl;
            // cout << "aux2 inicial: " << aux2 << endl;
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (prevReg.posNext + N), ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
            // mostrarRegistro(prevReg);
            pk1 = getPrimaryKeyFromRegistro(prevReg, nameKey);
            while(pk1 != key){
                if (prevReg.nextEspacioType == 'd') {
                    // no lo encontro en la parte enlazada
                    file.close();
                    return false;
                }
                // lo busco en todos sus enlazados
                aux = aux2;
                // cout << "aux: " << aux << endl;
                aux2 = prevReg.posNext + N;
                // cout << "aux2: " << aux2 << endl;
                file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * aux2, ios::beg);
                file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
                // mostrarRegistro(prevReg);
                pk1 = getPrimaryKeyFromRegistro(prevReg, nameKey);
            }
            // devuelvo al prevReg a su anterior
            file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * aux, ios::beg);
            file.read(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
            cout << "Registro previo en auxiliares" << endl;
            // mostrarRegistro(prevReg);
            posPrev = aux;
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

    
    // mostrarRegistro(prevReg);
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
        // mostrarRegistro(reg);
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
    // cout << "posPrev: "<< posPrev << endl;
    // Manejo del primer registro
    if (posPrev == 0) {
        // mostrarRegistro(reg);
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
        file.seekp(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * posPrev, ios::beg);
        file.write(reinterpret_cast<char*>(&prevReg), sizeof(Registro));
    }

    // Ahora estamos en el registro a eliminar (reg) y prevReg es el registro anterior
    // Marcar el registro como eliminado ('e')
    reg.nextEspacioType = 'e';
    // mostrarRegistro(reg);
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


        cout << i << ": " << reg.title << ", " << reg.releaseYear << " puntero = " << reg.posNext << reg.nextEspacioType << endl;
    }

    cout << "Registros en la parte Auxiliar:\n";
    int i = 0;
    file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (N + i), ios::beg);
    while (file.read(reinterpret_cast<char*>(&reg), sizeof(Registro))) {  // Asumiendo que MAX_AUXILIAR es la cantidad de registros auxiliares
        cout << "Aux " << i << ": " << reg.title << ", " << reg.releaseYear << " puntero = " << reg.posNext << reg.nextEspacioType << endl;
        i++;
        file.seekg(sizeof(int) + sizeof(char[20]) + sizeof(Registro) * (N + i), ios::beg);
    }
    cout << "---------------------------------\n";

    file.close();
}


template <typename Func>
auto medirTiempo(Func f) {
    auto start = chrono::high_resolution_clock::now();
    f();
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duracion = end - start;
    return duracion.count();
}
/*
int main () 
{
    SequentialFile<string> seqFile("registros.dat", "title");

    // cambiar el path
    vector<Registro> registros = readCSV("TV Series_modificado.csv", 25000);

    // --- Medir tiempo de inserción ---
    cout << "\nMedición de tiempos de inserción:\n";
    auto tiempoInsercion = medirTiempo([&]() {
        for (auto reg: registros){
            seqFile.add(reg);  // Inserta todos los registros
        }
    });
    cout << "Tiempo de inserción para " << registros.size() << " registros: " << tiempoInsercion << " ms\n";

    // seqFile.displayRecords();

    // --- Medir tiempo de búsqueda ---
    cout << "\nTests de Búsqueda\n";
    auto tiempoBusqueda = medirTiempo([&]() {
        Registro reg = seqFile.search("1899");
        mostrarRegistro(reg);
    });
    cout << "Tiempo de búsqueda: " << tiempoBusqueda << " ms\n";
    

        // --- Medir tiempo de búsqueda por rango ---
    int k = 0;
    cout << "\nTest de RangeSearch\n";
    auto tiempoRangeSearch = medirTiempo([&]() {
        vector<Registro> regs = seqFile.rangeSearch("Britannia", "Partner Track");
        
        for (auto reg: regs)
            k++;
    });
    cout << "Tiempo de búsqueda por rango: " << tiempoRangeSearch << " ms\n";
    cout << "K registros encontrados : " << k << endl;

    // --- Medir tiempo de eliminación ---
    cout << "\nTest de delete\n";
    auto tiempoEliminacion = medirTiempo([&]() {
        bool result = seqFile.removeKey("Britannia");
    });
    cout << "Tiempo de eliminación: " << tiempoEliminacion << " ms\n";

    return 0;
}
*/