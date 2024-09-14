#define BUF_size 1024
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const int Page_size = 127 ; //es el numero de llaves que puede tener una pagina
const int Data_size = 3; //es el numero de registros que puede tener una pagina de datos // 31
struct Registro {
    int codigo;
    char nombre[12];
    char apellidos[12];
    int ciclo;

    void setData(ifstream &file){
        file >> codigo; file.get();
        file.getline(nombre, 12, ',');
        file.getline(apellidos, 12, ',');
        file >> ciclo; file.get();
    }
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
// m * peso de las llavves , digamos que es 4
//m*4 + (m+1)*4 + 4 = 1024 // esto es el tamaño de la pagina 
// tallando el tamaño de la pagina
//  PageData=(tamano de Registro × Data_size)+4(count)+4(nextPage)
//  calculando el tamaño de la página de     datos
//  tamaño de registro = 4+12+12+4 = 32 bytes
//  32*data_size + 4 + 4 = 1024 ; data_size = 31
struct PageData{
    Registro records[Data_size]; //peso = 32
    int count;                  //peso = 4
    int nextPage;               //peso = 4
    //constructor
    PageData(){
        this->count = 0;
        this->nextPage = -1;
    }
};

void operator >> (ifstream &file, PageData &data){
    file.read((char*)&data, sizeof(PageData));
}

void operator << (ofstream &file, PageData &data){
    file.write((char*)&data, sizeof(PageData));
}

template <typename T>
class ISAMFile{
    string data_file;
    string index_file;
    public:
    ISAMFile(string data_file, string index_file){
        this->data_file = data_file;
        this->index_file = index_file;
        // si el indice no existe, se crea uno nuevo
        if(!Exist_index(index_file)){
            fstream i_file(index_file, ios::binary | ios::out);
            if(!i_file.is_open()){
                cout << "Error al crear el archivo de indices" << endl;
                return;
            }
            PageIndex index;
            ValueKeys(index);
            i_file.write((char*)&index, sizeof(PageIndex));
            i_file.seekg(0, ios::end);
            cout << "i_file.tellg(): " << i_file.tellg() << endl;
            cout << "sizeof(PageIndex): " << sizeof(PageIndex) << endl;
            i_file.close();
        } else {cout << "El archivo de indices ya existe" << endl;}
        // si el archivo de datos no existe, se crea uno nuevo
        if(!Exist_data(data_file)){
            fstream d_file(data_file, ios::binary | ios::out);
            if(!d_file.is_open()){
                cout << "Error al crear el archivo de datos" << endl;
                return;
            }
            PageData data;
            for (int i = 0; i < Page_size; i++) {
                d_file.write((char*)&data, sizeof(PageData));
            }
            d_file.seekg(0, ios::end);
            cout << "i_file.tellg(): " << d_file.tellg() << endl;
            cout << "sizeof(PageIndex): " << sizeof(PageData) << endl;
            d_file.close();
        } else {cout << "El archivo de datos ya existe" << endl;}
        
        

    }

    bool Exist_index(string index_file) {
        fstream i_file(index_file, ios::binary | ios::in);
        if (!i_file.is_open()) {
            cout << "El archivo no abre" << endl;
            return false;
        }
        PageIndex index;
        i_file.seekg(0, ios::beg);
        if (!i_file.read((char*)&index, sizeof(PageIndex))) {
            // si no se pudo leer la estructura de la pagina de indices, significa que el archivo esta vacio
            i_file.close();
            return false;
        }
        i_file.close();
        return true;
    }

    void ValueKeys(PageIndex &index){

        for (int i = 0; i < Page_size; i++) {
            index.keys[i] = (i + 1) * 10; // Llaves inicializadas con un valor arbitrario
            index.pages[i] = i;           // Páginas apuntando a su respectiva posición
        }
        index.pages[Page_size] = Page_size; // Última página
        index.count = Page_size;            // Número de llaves en el índice
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


    bool add(Registro registro){
        // paso 0 : abrir el archivo indice 
        fstream i_file(index_file, ios::binary | ios::in | ios::out);
        if(!i_file.is_open()){
            cout << "Error al agregar datos " << endl;
            return false;
        }
        // paso 1 : leer la estructura de la pagina de indices
        PageIndex index;
        i_file.seekg(0 , ios::beg); // posicionarse al inicio del archivo
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();
        // paso 2 : buscar la pagina donde se encuentra la llave
        int pageIndex = search_key(index, registro.codigo); // buscar la pagina donde se encuentra la llave
        // paso 3 : abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in | ios::out);
        if(!d_file.is_open()){
            cout << "Error al abrir el archivo de datos " << endl;
            return false;
        }
        // paso 4 : leer la estructura de la pagina de datos
        PageData data;
        d_file.seekg(index.pages[pageIndex] * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));
        // paso 5 : verificar si la pagina tiene espacio
        while(true){
            if(data.count < Data_size){
                // agregar el registro a la pagina de datos
                data.records[data.count] = registro;
                data.count++; //aumentar el contador de registros
                // escribir la pagina de datos
                d_file.seekp(index.pages[pageIndex] * sizeof(PageData), ios::beg);
                cout << "Escribiendo en la pagina de datos " << index.pages[pageIndex] << endl;
                d_file.write((char*)&data, sizeof(PageData));
                d_file.close();
                return true;
            } else if (data.count == Data_size && data.nextPage != -1){
                // buscar una pagina enlazada con espacio
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            }else if (data.count == Data_size && data.nextPage == -1){
                // Crear una pagina enlazada 
                PageData newData;
                newData.records[0] = registro;
                newData.count = 1;
                newData.nextPage = -1;
                // paso 6: posicionarse al final del archivo de datos
                d_file.seekp(0, ios::end);
                long posFinal = d_file.tellp();
                cout << posFinal << endl;
                cout << sizeof(PageData) << endl;
                // paso 7: escribir la nueva pagina de datos
                d_file.write((char*)&newData, sizeof(PageData));
                // paso 8: Enlazar la nueva pagina de datos
                data.nextPage = d_file.tellp() / sizeof(PageData); // obtener la posicion de la pagina de datos
                d_file.seekp(index.pages[pageIndex] * sizeof(PageData), ios::beg); // posicionarse en la pagina de datos
                d_file.write((char*)&data, sizeof(PageData));
                d_file.close();
                return true;
            }
        }
        return false;

    }
    // analizarlo mejor 
    int search_key(PageIndex index, T key){
        for(int i = 0; i < index.count; i++){
            if(key < index.keys[i]){
                return i;
            }
        }
        return index.count - 1;
    }

    //La busqueda espesiifica puede retornar mas de un elemento que coincida con la key 
    Registro search(T key){
        // 1. Abrir el archivo de índices
        fstream i_file(index_file, ios::binary | ios::in);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            exit(1);
        }

        // 2. Leer la estructura de la página de índices
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();

        // 3. Buscar la página donde podría estar la llave
        int pageIndex = search_key(index, key);

        // 4. Abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            exit(1);
        }

        // 5. Leer la página de datos correspondiente
        PageData data;
        d_file.seekg(index.pages[pageIndex] * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));

        // 6. Buscar el registro en la página
        while (true) {
            for (int i = 0; i < data.count; i++) {
                if (data.records[i].codigo == key) {
                    d_file.close();
                    return data.records[i];
                }
            }
            // Si no se encuentra en la página actual, verificar las páginas enlazadas
            if (data.nextPage != -1) {
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            } else {
                break;
            }
        }
        d_file.close();
        
        throw runtime_error("Registro no encontrado");
    }
    // La busqueda por rango retorna todos los registros que se encuentran entre las dos llaves de búsqueda
    vector <Registro> rangeSearch(T begin, T end){
        vector<Registro> result;
        //falta implementar
        return result;
    }
    // proponer un algoritmo de elimnacion 
    bool remove(T key){
        //falta implementar
        return false;
    }

    void print() { 
        // 1. Abrir el archivo de índices
        ifstream i_file(index_file, ios::binary | ios::in);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de indices" << endl;
            return;
        }

        // 2. Leer la estructura de la página de índices
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();

        // 3. Abrir el archivo de datos
        ifstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return;
        }

        // 4. Leer y mostrar el contenido de las páginas de datos
        PageData data;
        for (int i = 0; i < index.count; i++) {
            d_file.seekg(index.pages[i] * sizeof(PageData), ios::beg);
            d_file.read((char*)&data, sizeof(PageData));
            cout << "Pagina " << index.pages[i] << endl;
            cout << "Count: " << data.count << endl;
            for (int j = 0; j < data.count; j++) {
                cout << "Registro " << j << ": " << data.records[j].codigo << " " << data.records[j].nombre << " " << data.records[j].apellidos << " " << data.records[j].ciclo << endl;
            }
            cout << "Siguiente pagina: " << data.nextPage << endl;
            cout << endl;
        }

        d_file.close();
    }


};


int main(){
    ISAMFile<int> isam("data.dat", "indice1.dat");
    Registro reg[4] = {
        {5, "Juan", "Perez", 1},
        {6, "Juan", "Perez", 1},
        {7, "Maria", "Lopez", 2},
        {8, "Pedro", "Garcia", 3}
    };
    for(int i = 0; i < 4; i++){
        isam.add(reg[i]);
    }
    isam.print();
    return 0 ;     
}