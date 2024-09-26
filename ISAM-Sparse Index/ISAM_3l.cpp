#define BUF_size 8192
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <string.h>
#include <sstream>

using namespace std;

const int Page_size = 10; //es el numero de llaves que puede tener una pagina //127
const int Data_size = 23; //es el numero de registros que puede tener una pagina de datos // 31

//hallando el tamaño de la pagina index
// Page_size*4 + (Page_size+1)*4 + 4 = 8192 // esto es el tamaño de la pagina 
// page_size = 1023 , pero en el 3 nivel 
// page_size = 10  , asi que masomenos deberian aver 10 registros por pagina
// page_size = 1000 en el 3er nivel

// hallando el tamaño de la pagina de datos
    // 8192/354 = 23.16 o aprox 23 , 23 registros por pagina

struct Registro {
    char Tittle[30];        
    char ReleaseYear[12];
    char Runtime[7];
    char Genre[25];
    char Rating[5];
    char Cast[75];
    char Synopsis[200];
    // el total de bits es 354
};
struct PageIndex{
    int keys[Page_size];
    int pages[Page_size+1]; //posicion de la pagina 
    int count ;
    //constructor
    PageIndex(){
        this->count = 0;
    }
};
struct PageData{
    Registro records[Data_size]; 
    int count;                  
    int nextPage;               
    //constructor
    PageData(){
        this->count = 0;
        this->nextPage = -1;
    }
};

template <typename T>
class ISAMFile{
    string data_file;
    string index1_file;
    string index2_file;
    string index3_file;
    public:
    ISAMFile(string data_file, string index1_file , string index2_file, string index3_file){
        this->data_file = data_file;
        this->index1_file = index1_file;
        this->index2_file = index2_file;
        this->index3_file = index3_file;

        // si el indice no existe, se crea uno nuevo
        if(!Exist_index(index1_file)){
            CreateIndex1(index1_file);
            CreateIndex2(index2_file);
            CreateIndex3(index2_file, index3_file);
            //cout << "i_file.tellg(): " << i_file.tellg() << endl;
            //cout << "sizeof(PageIndex): " << sizeof(PageIndex) << endl;

        } else {cout << "El archivo de indices ya existe" << endl;}

        // si el archivo de datos no existe, se crea uno nuevo
        if(!Exist_data(data_file)){
            fstream i_index3(index3_file, ios::binary | ios::in);
            if(!i_index3.is_open()){
                cout << "Error al abrir el archivo de indices 3" << endl;
                return;
            }
            PageIndex index3;
            i_index3.seekg(0, ios::beg);
            i_index3.read((char*)&index3, sizeof(PageIndex));
            //cantidad de paginas en el nivel 3
            i_index3.seekg(0, ios::end);
            int CantPages = i_index3.tellg() / sizeof(PageIndex);
            int CantKeys = CantPages * Page_size;
            //valor de la paginas
            int Valor = index3.keys[0];
            i_index3.close();

            fstream d_file(data_file, ios::binary | ios::out);
            if(!d_file.is_open()){
                cout << "Error al crear el archivo de datos" << endl;
                return;
            }
            PageData data;
            for (int i = 0; i < CantKeys; i++) {
                d_file.write((char*)&data, sizeof(PageData));
                //cout << "Escribiendo en la pagina de datos " << i << endl;
            }
            d_file.seekg(0, ios::end);
            d_file.close();
        } else {cout << "El archivo de datos ya existe" << endl;}
    

    }
    //destrcutor
    ~ISAMFile(){
        cout << "Cerrando el archivo" << endl;
    }
    void CreateIndex1(string index_file){
        fstream i_file(index_file, ios::binary | ios::out | ios::in);
        PageIndex index;
        ValueKeys(index);
        i_file.write((char*)&index, sizeof(PageIndex));
        i_file.seekg(0, ios::end);
        i_file.close();
    }
    void CreateIndex2(string index_file2){
        fstream i_file2(index_file2, ios::binary | ios::out | ios::in);
        
        cout << "Creando el archivo de indices 2" << endl;
        //asignando valores a las llaves del nivel 2
        for(int i = 0; i < Page_size ; i++){
            PageIndex index2;
            index2.count = 0;
            ValueKeys(index2);
            i_file2.write((char*)&index2, sizeof(PageIndex));
        }
        i_file2.seekg(0, ios::end);
        i_file2.close();
    }
    void CreateIndex3(string index_file2, string index_file3) {
        fstream i_file2(index_file2, ios::binary | ios::in);
        fstream i_file3(index_file3, ios::binary | ios::out | ios::in);
        
        PageIndex index2;  // Estructura para leer el nivel 2
        //PageIndex index3;  // Estructura para escribir en el nivel 3
        cout << "Creando el archivo de indices 3" << endl;
        i_file2.seekg(0, ios::end);
        //cantidad de paginas en el nivel 2
        int CantPages = i_file2.tellg() / sizeof(PageIndex);
        int CantKeys = CantPages * Page_size;
        i_file2.seekg(0, ios::beg);
        i_file2.read((char*)&index2, sizeof(PageIndex));
        i_file2.close();

        //crear las paginas de nivel 3 para cada key del nivel 2
        for(int i = 0; i < CantKeys; i++){
            PageIndex index3;
            index3.count = 0;
            ValueKeys(index3);
            i_file3.write((char*)&index3, sizeof(PageIndex));
        }

        i_file3.close();
    }
    void ValueKeys(PageIndex &index) {
        for (int i = 0; i < Page_size; i++) {
            index.keys[i] = (i + 1) * 100; // Llaves inicializadas con un valor arbitrario
            index.pages[i] = i;           // Páginas apuntando a su respectiva posición
        }
        index.pages[Page_size] = Page_size; // Última página
        index.count = Page_size;            // Número de llaves en el índice
    }
    bool Exist_index(string index_file) {
        fstream i_file(index_file, ios::binary | ios::in | ios::out);
        if (!i_file.is_open()) {
            cout << "El archivo no abre" << endl;
            return false;
        }
        PageIndex index;
        i_file.seekg(0, ios::beg);
        if (!i_file.read((char*)&index, sizeof(PageIndex))) {
            // si no se pudo leer la estructura de la pagina de indices, significa que el archivo esta vacio
            cout << "El archivo esta vacio" << endl;
            i_file.close();
            return false;
        }
        i_file.close();
        cout << "El archivo ya existe" << endl;
        return true;
    }
    bool Exist_data(string data_file){
        fstream d_file(data_file, ios::binary | ios::in);
        if(!d_file.is_open()){
            cout << "El archivo de datos no abre" << endl;
            return false;
        }
        PageData data;
        d_file.seekg(0, ios::beg);
        if(!d_file.read((char*)&data, sizeof(PageData))){
            d_file.close();
            return false;
        }
        d_file.close();
        return true;
    }
    string procesarTitulo(string titulo){
        //convertimos el titulo a minisculas para poder comparar
        transform(titulo.begin(), titulo.end(), titulo.begin(), ::tolower);
        //si el titulo comienza con the, a , an, eliminamos estas palabras
        if(titulo.find("the ") == 0){
            titulo = titulo.substr(4);
        } else if(titulo.find("a ") == 0){
            titulo = titulo.substr(2);
        } else if(titulo.find("an ") == 0){
            titulo = titulo.substr(3);
        }
        return titulo;

    }
    int FirstLetterToKey(char firstLetter) {
        // Normalización de mayúsculas a minúsculas.
        firstLetter = tolower(firstLetter);
        // Convertir la primera letra en una clave numérica.
        if (firstLetter >= 'a' && firstLetter <= 'c') {
            return 200;
        } else if (firstLetter >= 'd' && firstLetter <= 'f') {
            return 400;
        } else if (firstLetter >= 'g' && firstLetter <= 'i') {
            return 600;
        } else if (firstLetter >= 'j' && firstLetter <= 'l') {
            return 800;
        } else if (firstLetter >= 'm' && firstLetter <= 'o') {
            return 1000;
        } else if (firstLetter >= 'p' && firstLetter <= 'r') {
            return 1200;
        } else if (firstLetter >= 's' && firstLetter <= 'u') {
            return 1400;
        } else if (firstLetter >= 'v' && firstLetter <= 'x') {
            return 1600;
        } else if (firstLetter >= 'y' && firstLetter <= 'z') {
            return 1800;
        } else {
            return 2000;}
    }
    int FirstLetterToKey2(char firstLetter) {
        // Normalización de mayúsculas a minúsculas.
        firstLetter = tolower(firstLetter);
        // Convertir la primera letra en una clave numérica.
        if (firstLetter >= 'a' && firstLetter <= 'n') {
            return 100;
        } else if (firstLetter >= 'm' && firstLetter <= 'z') {
            return 200;
        } else {return 0 ;}
    }
    int SearchKey(const PageIndex &index, const string &key , int &nivelador ,  int & posicionLetra) {
        // Procesar la clave para eliminar palabras comunes.
        string newKey = procesarTitulo(key);
        // Obtener la primera letra de la clave.
        char firstLetter = newKey[posicionLetra];
        posicionLetra++;
        // Convertir la primera letra en una clave numérica.
        int keyNumber = FirstLetterToKey(firstLetter);
        // Buscar la clave en el índice.

        for (int i = 0; i < Page_size; i++) {
            if (keyNumber <= index.keys[i]) {
                nivelador = (nivelador*Page_size) + i;
                return i;
            }
        }
        return -1;
    }
    // Función que busca la página correcta en un índice (recorriendo varios niveles).
    void SearchIndex(fstream &i_file, int &currentPage, const string &key, PageIndex &index, int &nivelador , int &posicionLetra) {
        //cout << "nivelador pero en el search: " << nivelador << endl;
        i_file.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
    
        int pageIndex = SearchKey(index, key , nivelador , posicionLetra);
        //cout << "pageIndex: " << pageIndex << endl;
        currentPage = index.pages[pageIndex]; // Actualizar la página actual
    }
    bool add(Registro registro) {
        // Paso 0: abrir el archivo de índice de nivel 1
        fstream i_file(index1_file, ios::binary | ios::in | ios::out);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 1" << endl;
            return false;
        }
        PageIndex index;
        int nivelador = 0; 
        int posicionLetra = 0; 
        // Paso 1: Leer el nivel 1 y buscar el índice adecuado
        int currentPage = 0; // comenzamos en el nivel 1
        SearchIndex(i_file, currentPage, registro.Tittle, index, nivelador, posicionLetra);
        i_file.close();
        // Abrir el archivo del índice del nivel 2
        fstream i_file2(index2_file, ios::binary | ios::in | ios::out);
        if (!i_file2.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 2" << endl;
            return false;
        }
        i_file2.seekg(0, ios::end);
        // Paso 2: Leer el nivel 2 y buscar el índice adecuado
        SearchIndex(i_file2, currentPage, registro.Tittle, index, nivelador, posicionLetra);
        i_file2.close();
        // Abrir el archivo del índice del nivel 3
        fstream i_file3(index3_file, ios::binary | ios::in | ios::out);
        if (!i_file3.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 3" << endl;
            return false;
        }
        // Paso 3: Leer el nivel 3 y buscar el índice adecuado
        SearchIndex(i_file3, currentPage, registro.Tittle, index, nivelador , posicionLetra);
        i_file3.close();

        // Paso 4: abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in | ios::out);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return false;
        }
        // Leer la página de datos correspondiente desde el nivel 3
        PageData data;
        d_file.seekg(nivelador * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));
        // Paso 5: insertar en la página de datos
        while (true) {
            if (data.count < Data_size) {
                // agregar el registro en la página de datos
                data.records[data.count] = registro;
                data.count++; // aumentar el contador de registros
                // reescribir la página de datos
                d_file.seekp(nivelador * sizeof(PageData), ios::beg);
                d_file.write((char*)&data, sizeof(PageData));
                //cout << "Se agregó el registro: " << registro.Tittle << endl;
                d_file.close();
                return true;
            } else if (data.count == Data_size && data.nextPage != -1) {
                // página está llena, pero tiene una siguiente página
                nivelador = data.nextPage;
                //cout << " la sigueinte pagina es : " << data.nextPage << endl;
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            } else if (data.count == Data_size && data.nextPage == -1) {
                // página está llena y no tiene siguiente página: crear una nueva página
                PageData newData;
                newData.records[0] = registro;
                newData.count = 1;
                newData.nextPage = -1;

                d_file.seekp(0, ios::end);                                  // posicionarse al final del archivo de datos
                d_file.write((char*)&newData, sizeof(PageData));            // escribir la nueva página de datos

                data.nextPage = d_file.tellp() / sizeof(PageData) - 1;      // enlazar la nueva página de datos
                d_file.seekp(nivelador * sizeof(PageData), ios::beg);     // reescribir la página actual
                d_file.write((char*)&data, sizeof(PageData));               // escribir la página actual
                d_file.close();
                return true;
            }
        }
        return false;
    }
    //La busqueda espesiifica puede retornar mas de un elemento que coincida con la key 
    Registro search(const string &key){
        // 1. Abrir el archivo de indice de nivel 1
        fstream i_file1(index1_file, ios::binary | ios::in);
        if (!i_file1.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 2. Leer la estructura de la página de índices
        PageIndex index;
        int nivelador = 0;
        int posicionLetra = 0;
        i_file1.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file1.read((char*)&index, sizeof(PageIndex));

        // 3. Buscar la página donde podría estar la llave
        int pageIndex = SearchKey(index, key , nivelador , posicionLetra);
        i_file1.close();
        //abrir el archivo de indice de nivel 2
        fstream i_file2(index2_file, ios::binary | ios::in);
        if (!i_file2.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 4. Leer la estructura de la página de índices
        i_file2.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file2.read((char*)&index, sizeof(PageIndex));
        // 5. Buscar la página donde podría estar la llave
        pageIndex = SearchKey(index, key , nivelador , posicionLetra);
        i_file2.close();
        //abrir el archivo de indice de nivel 3
        fstream i_file3(index3_file, ios::binary | ios::in);
        if (!i_file3.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 6. Leer la estructura de la página de índices
        i_file3.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file3.read((char*)&index, sizeof(PageIndex));
        // 7. Buscar la página donde podría estar la llave
        pageIndex = SearchKey(index, key , nivelador , posicionLetra);
        i_file3.close();
        // 8. abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            exit(1);
        }
        // 9. Leer la página de datos correspondiente
        PageData data;
        d_file.seekg(nivelador * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));

        // 10. Buscar el registro en la página
        while (true) {
            for (int i = 0; i < data.count; i++) {
                if (data.records[i].Tittle == key) {
                    d_file.close();
                    return data.records[i];
                }
            }
            // Si no se encuentra en la página actual, verificar las páginas enlazadas
            if (data.nextPage != -1) {
                nivelador = data.nextPage;
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            } else {
                cout << "No se encontró el registro" << endl;
                break;
            }
        }
        d_file.close();
        Registro empty;
        return empty;
    }
    // La busqueda por rango retorna todos los registros que se encuentran entre las dos llaves de búsqueda
    vector <Registro> rangeSearch(const string &begin, const string &end){
        vector<Registro> result;
     // 1. Abrir el archivo de indice de nivel 1
        fstream i_file1(index1_file, ios::binary | ios::in);
        if (!i_file1.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 2. Leer la estructura de la página de índices
        PageIndex index;
        int nivelador = 0;
        int posicionLetra = 0;
        i_file1.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file1.read((char*)&index, sizeof(PageIndex));

        // 3. Buscar la página donde podría estar la llave
        int pageIndex = SearchKey(index, begin , nivelador , posicionLetra);
        i_file1.close();
        //abrir el archivo de indice de nivel 2
        fstream i_file2(index2_file, ios::binary | ios::in);
        if (!i_file2.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 4. Leer la estructura de la página de índices
        i_file2.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file2.read((char*)&index, sizeof(PageIndex));
        // 5. Buscar la página donde podría estar la llave
        pageIndex = SearchKey(index, begin , nivelador , posicionLetra);
        i_file2.close();
        //abrir el archivo de indice de nivel 3
        fstream i_file3(index3_file, ios::binary | ios::in);
        if (!i_file3.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }
        // 6. Leer la estructura de la página de índices
        i_file3.seekg(nivelador * sizeof(PageIndex), ios::beg);
        i_file3.read((char*)&index, sizeof(PageIndex));
        // 7. Buscar la página donde podría estar la llave
        pageIndex = SearchKey(index, begin , nivelador , posicionLetra);
        i_file3.close();
        // 8. abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            exit(1);
        }
        // 9. Leer la página de datos correspondiente
        PageData data;
        d_file.seekg(nivelador * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));

        bool rangeStart = false;
        // 11. Buscar el registro en la página
        while (true) {
            for (int i = 0; i < data.count; i++) {
                //cout << "Tittle: " << data.records[i].Tittle << endl;
                //cout << "begin: " << begin << endl;
                //cout << "end: " << end << endl;
                if (data.records[i].Tittle == begin){rangeStart = true;}
                if (rangeStart) {result.push_back(data.records[i]);}
                if(data.records[i].Tittle == end){
                    d_file.close();
                    return result;
                }
            }
            // Si no se encuentra en la página actual, verificar las páginas enlazadas
            if (data.nextPage != -1) {
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            } else {
                nivelador = nivelador + 1;
                d_file.seekg(nivelador * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            }
        }

        d_file.close();
        return result;
    }
    // proponer un algoritmo de elimnacion 
    bool remove(T key){
        // Paso 0: abrir el archivo de índice de nivel 1
        fstream i_file(index1_file, ios::binary | ios::in | ios::out);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 1" << endl;
            return false;
        }
        PageIndex index;
        int nivelador = 0; 
        int posicionLetra = 0; 
        // Paso 1: Leer el nivel 1 y buscar el índice adecuado
        int currentPage = 0; // comenzamos en el nivel 1
        SearchIndex(i_file, currentPage, key, index, nivelador, posicionLetra);
        i_file.close();
        // Abrir el archivo del índice del nivel 2
        fstream i_file2(index2_file, ios::binary | ios::in | ios::out);
        if (!i_file2.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 2" << endl;
            return false;
        }
        i_file2.seekg(0, ios::end);
        // Paso 2: Leer el nivel 2 y buscar el índice adecuado
        SearchIndex(i_file2, currentPage, key, index, nivelador, posicionLetra);
        i_file2.close();
        // Abrir el archivo del índice del nivel 3
        fstream i_file3(index3_file, ios::binary | ios::in | ios::out);
        if (!i_file3.is_open()) {
            cout << "Error al abrir el archivo de índice nivel 3" << endl;
            return false;
        }
        // Paso 3: Leer el nivel 3 y buscar el índice adecuado
        SearchIndex(i_file3, currentPage, key, index, nivelador , posicionLetra);
        i_file3.close();

        // Paso 4: abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in | ios::out);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return false;
        }
        // Leer la página de datos correspondiente desde el nivel 3
        PageData data;
        d_file.seekg(nivelador * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));
        // Paso 5: buscar el registro a eliminar
        while (true) {
            for (int i = 0; i < data.count; i++) {
                if (data.records[i].Tittle == key) {
                    // eliminar el registro en la página de datos
                    for (int j = i; j < data.count - 1; j++) {
                        data.records[j] = data.records[j + 1];
                    }
                    data.count--; // disminuir el contador de registros
                    // reescribir la página de datos
                    d_file.seekp(nivelador * sizeof(PageData), ios::beg);
                    d_file.write((char*)&data, sizeof(PageData));
                    cout << "Se eliminó el registro: " << key << endl;
                    d_file.close();
                    return true;
                }
            }
            // Si no se encuentra en la página actual, verificar las páginas enlazadas
            if (data.nextPage != -1) {
                nivelador = data.nextPage;
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            } else {
                cout << "No se encontró el registro" << endl;
                break;
            }
        }
        return false;


    }
    void print() { 
        ifstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return;
        }

        PageData data;
        int pageCounter = 0;  // Contador de páginas
        streampos lastPosition;  // Guardar la última posición después de leer la página principal

        // Leer cada página en el archivo
        while (d_file.read((char*)&data, sizeof(PageData))) {
            cout << "------------------------------------" << endl;
            cout << "Página: " << pageCounter << endl;  // Indicar el número de página actual

            // Imprimir los registros de la página actual
            for (int i = 0; i < data.count; i++) {
                cout << "Tittle: " << data.records[i].Tittle << endl;
                cout << "------------------------------------" << endl;
            }

            // Guardar la posición después de leer la página principal
            lastPosition = d_file.tellg();

            // Si hay una página enlazada (overflow), seguir imprimiendo esa página
            int nextPage = data.nextPage;
            while (nextPage != -1) {
                cout << "Siguiente sub página: " << nextPage << endl;
                
                // Posicionar el puntero en la página enlazada (overflow)
                d_file.seekg(nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));

                // Imprimir los registros de la página enlazada
                for (int i = 0; i < data.count; i++) {
                    cout << "Tittle: " << data.records[i].Tittle << endl;
                    cout << "------------------------------------" << endl;
                }

                // Ir a la siguiente página enlazada si existe
                nextPage = data.nextPage;
            }

            // Regresar a la posición original en el archivo para seguir leyendo las páginas principales
            d_file.seekg(lastPosition);

            pageCounter++;
        }

        d_file.close();
    }
    void printEstructura() {
        // Leer PageIndex nivel 1
        fstream i_file(index1_file, ios::binary | ios::in);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de indices (nivel 1)" << endl;
            return;
        }
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();

        cout << "------------------------------------PageIndex_1: " << endl;
        for (int i = 0; i < index.count; i++) {
            cout << "Key: " << index.keys[i] << " Page: " << index.pages[i] << " Count: " << index.count << endl;
        }

        // Leer PageIndex nivel 2
        int contador_2 = 0;
        int contador_3 = 0;
        fstream i_file2(index2_file, ios::binary | ios::in);
        if (!i_file2.is_open()) {
            cout << "Error al abrir el archivo de indices (nivel 2)" << endl;
            return;
        }
        PageIndex index2;
        cout << "------------------------------------PageIndex_2: " << endl;
        while (i_file2.read((char*)&index2, sizeof(PageIndex))) {
            contador_2++;
            for(int i = 0; i < index2.count; i++){
                contador_3++;
                cout << "Key: " << index2.keys[i] << " Page: " << index2.pages[i] << " Count: " << index2.count << endl;
            }
        }
        cout << "contador de paginas: " << contador_2 << endl;
        cout << "contador de keys: " << contador_3 << endl;
        i_file2.close();

        // Leer PageIndex nivel 3
        int contador = 0 ; 
        int contador2= 0 ; 
        fstream i_file3(index3_file, ios::binary | ios::in);
        if (!i_file3.is_open()) {
            cout << "Error al abrir el archivo de indices (nivel 3)" << endl;
            return;
        }
        PageIndex index3;
        cout << "--------------------------------------PageIndex_3: " << endl;
        while (i_file3.read((char*)&index3, sizeof(PageIndex))) {
            contador++;
            for(int i = 0; i < Page_size; i++){
                contador2++;
                cout << "Key: " << index3.keys[i] << " Page: " << index3.pages[i] << " Count: " << index3.count << endl;
            }
        }
        i_file3.close();
        cout << "contador de paginas: " << contador << endl;
        cout << "contador de keys: " << contador2 << endl;
    }
    void CantAllRegistros(){
        fstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return;
        }
        PageData data;
        int pageCounter = 0;  // Contador de páginas
        int contador = 0;
        streampos lastPosition;  // Guardar la última posición después de leer la página principal

        // Leer cada página en el archivo
        while (d_file.read((char*)&data, sizeof(PageData))) {
            contador += data.count;
            // Guardar la posición después de leer la página principal
            lastPosition = d_file.tellg();

            // Si hay una página enlazada (overflow), seguir imprimiendo esa página
            int nextPage = data.nextPage;
            while (nextPage != -1) {
                // Posicionar el puntero en la página enlazada (overflow)
                d_file.seekg(nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
                contador += data.count;
                // Ir a la siguiente página enlazada si existe
                nextPage = data.nextPage;
            }

            // Regresar a la posición original en el archivo para seguir leyendo las páginas principales
            d_file.seekg(lastPosition);

            pageCounter++;
        }

        d_file.close();
        cout << "Cantidad de registros: " << contador << endl;
    }
};

void test1(ISAMFile <int> &isam){
        Registro reg[4] = {
        {"the ednesday", "2019– ", "60 min", "Action, Adventure, Drama", "8.2", "Henry Cavill, Freya Allan, Anya Chalotra, Mimi Ndiweni", "Geralt of Rivia, a solitary monster hunter, struggles to find his place in a world where people often prove more wicked than beasts."},
        {"The Mandalorian", "2019– ", "40 min", "Action, Adventure", "8.8", "Pedro Pascal, Carl Weathers, Gina Carano, Giancarlo Esposito", "The travels of a lone bounty hunter in the outer reaches of the galaxy, far from the authority of the New Republic."},
        {"Melicual1", "2019– ", "60 min", "Action, Adventure, Drama", "8.2", "Henry Cavill, Freya Allan, Anya Chalotra, Mimi Ndiweni", "Geralt of Rivia, a solitary monster hunter, struggles to find his place in a world where people often prove more wicked than beasts."},
        {"Melicual2", "2019– ", "40 min", "Action, Adventure", "8.8", "Pedro Pascal, Carl Weathers, Gina Carano, Giancarlo Esposito", "The travels of a lone bounty hunter in the outer reaches of the galaxy, far from the authority of the New Republic."},
            
    };
    Registro reg2[4] = {// mas registro , no repitas registros
        {"The Witwwqacher", "2019– ", "60 min", "Action, Adventure, Drama", "8.2", "Henry Cavill, Freya Allan, Anya Chalotra, Mimi Ndiweni", "Geralt of Rivia, a solitary monster hunter, struggles to find his place in a world where people often prove more wicked than beasts."},
        {"The dsad", "2019– ", "40 min", "Action, Adventure", "8.8", "Pedro Pascal, Carl Weathers, Gina Carano, Giancarlo Esposito", "The travels of a lone bounty hunter in the outer reaches of the galaxy, far from the authority of the New Republic"},
    };
    Registro regis = {"Star Wars: The Clone Wars", "2008–2020", "23 min", "AdventFi", "8.2", "Tom Kane, Matt Lanter, Dee Bradley Baker, James Arnold Taylor", "Jedi Knights lead the Grand Army of the Republic against the droid army of the Separatists."};

    for(int i = 0; i < 4; i++){
        isam.add(reg[i]);
    }
    for(int i = 0; i < 4; i++){
        isam.add(reg2[i]);
    }
    //volver a instar un registro
    isam.add(regis);
}
void test2(ISAMFile <int> &isam){
        Registro reg[8] = {
            {"The aaacher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The aaztcher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The azacher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The azzcher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The zzzcher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The zzacher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The zazcher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            {"The zaacher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."},
            
    };
    for(int i = 0; i < 8; i++){
        isam.add(reg[i]);
    }
}
void test_search(ISAMFile<int> &isam){
    Registro reg = isam.search("The aitcher");
    cout <<"Se encontro el registro: " <<reg.Tittle << endl;
    Registro reg2 = isam.search("The aaacher");
    cout <<"Se encontro el registro: " <<reg2.Tittle << endl;
    Registro reg3 = isam.search("The aaztcher");
    cout <<"Se encontro el registro: " <<reg3.Tittle << endl;
    Registro reg4 = isam.search("The azacher");
    cout <<"Se encontro el registro: " <<reg4.Tittle << endl;
    Registro reg5 = isam.search("The azzcher");
    cout <<"Se encontro el registro: " <<reg5.Tittle << endl;
    Registro reg6 = isam.search("The zzzcher");
    cout <<"Se encontro el registro: " <<reg6.Tittle << endl;
    Registro reg7 = isam.search("The zzacher");
    cout <<"Se encontro el registro: " <<reg7.Tittle << endl;
    Registro reg8 = isam.search("The zazcher");
    cout <<"Se encontro el registro: " <<reg8.Tittle << endl;
}
void testrangeSearch(ISAMFile<int> &isam){
    vector<Registro> result = isam.rangeSearch("The aitcher", "The zzzcher");
    cout << "Registros encontrados: " << result.size() << endl;
    for(int i = 0; i < result.size(); i++){
        cout << "Titulo: " << result[i].Tittle << endl;
    }
}

void CargarDatos(ISAMFile<int> &isam, string ArchivoCSV, int limiteRegistro){
    ifstream archivo(ArchivoCSV);
    if(!archivo.is_open()){
        cout << "No se pudo abrir el archivo" << endl;
        return;
    }
    string linea;
    int contador = 0;
    //leer la primeara linea que son los titulos
    if(getline(archivo, linea)){
        cout << linea << endl;
    }
    while(getline(archivo, linea)){
        istringstream stream(linea); //convertir la linea en un stream
        string titulo, ReleaseYear, Runtime, genre, rating, Cast, Synopsis;
        getline(stream, titulo, ',');
        getline(stream, ReleaseYear, ',');
        getline(stream, Runtime, ',');
        getline(stream, genre, ',');
        getline(stream, rating, ',');
        getline(stream, Cast, ',');
        getline(stream, Synopsis, ',');
        //crear el registro
        Registro reg;
        //copiar los valores a los arrays de caracteres del struct
        strcpy(reg.Tittle, titulo.c_str());
        strcpy(reg.ReleaseYear, ReleaseYear.c_str());
        strcpy(reg.Runtime, Runtime.c_str());
        strcpy(reg.Genre, genre.c_str());
        strcpy(reg.Rating, rating.c_str());
        strcpy(reg.Cast, Cast.c_str());
        strcpy(reg.Synopsis, Synopsis.c_str());
        //agregar el registro al archivo
        isam.add(reg);

        contador++;
        if(contador == limiteRegistro){
            break;
        }
    }
}


int main(){
    ISAMFile<int> isam("data.dat", "indice1.dat" , "indice2.dat", "indice3.dat");
    //test1(isam);
    //test2(isam);
    Registro regis = {"The aitcher", "2019– ", "60 min", "Action", "8.2", "Henry Cavill", "Geralt of Rsts."};
    //isam.add(regis);
    //isam.printEstructura();
    ///////////////////////////////cargar datos
    CargarDatos(isam, "TVSeries.csv", 50000);
    ///////////////////////////////probando el search
    //test_search(isam);
    ///////////////////////////////todos los registros
    //isam.CantAllRegistros();
    ///////////////////////////////probando el rangeSearch
    //testrangeSearch(isam);
    isam.print();
    return 0 ;     
}


/*
// Todo esto depende de la cantidad de registros que se tengan en el archivo de datos (falta encontrar un .csv con mas registros)
    //suponiendo que se tienen 10 000 registros
ideas para 3 niveles:
    -Tamaño de página: 8192 bytes.
    -Page_size: 1023 (claves por página de índice).
    -Data_size: 15 (registros por página de datos).
Paginas: 
    - 15 registros por página de datos.
Nivel3: 
    - PageIndex: 1023 claves, 1024 páginas.
nivel2:
    - PageIndex: 31 claves, 32 páginas.
nivel1:
    - PageIndex: 31 claves, 32 páginas.
*/