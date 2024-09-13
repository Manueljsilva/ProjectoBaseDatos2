#define BUF_size 1024
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const int Page_size = 127 ; //es el numero de llaves que puede tener una pagina
const int Data_size = 127; //es el numero de registros que puede tener una pagina de datos
struct Registro {
    int codigo;
    char nombre[20];
    char apellidos[20];
    int ciclo;

    void setData(ifstream &file){
        file >> codigo; file.get();
        file.getline(nombre, 20, ',');
        file.getline(apellidos, 20, ',');
        file >> ciclo; file.get();
    }
};
struct PageIndex{
    int keys[Page_size];
    int pages[Page_size+1]; //posicion de la pagina 
    int count ;
    //constructor
    PageIndex(){
        count = 0;
    }
};
// m * peso de las llavves , digamos que es 4
//m*4 + (m+1)*4 + 4 = 1024 // esto es el tamaño de la pagina 

struct PageData{
    Registro records[Data_size];
    int count = 0;
    int nextPage;
};

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
            for (int i = 0; i < Data_size; i++) {
                d_file.write((char*)&data, sizeof(PageData));
            }
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
        if(data.count < Data_size){
            data.records[data.count] = registro;
            data.count++;
            d_file.seekp(index.pages[pageIndex] * sizeof(PageData), ios::beg);
            cout << "Escribiendo en la pagina de datos " << index.pages[pageIndex] << endl;
            d_file.write((char*)&data, sizeof(PageData));
            d_file.close();
            return true;
        } else {
            // por implementar
            cout << "No hay espacio en la pagina de datos - implementar" << endl;
            return false;
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
        //paso 1 : abrir el archivo de indices
        fstream index_file(index_file, ios::binary | ios::in | ios::out);
        if(!index_file.is_open()){
            cout << "Error al abrir el archivo de indices " << endl;
            exit(1);
        }
        //paso 2 : buscar la pagina donde se encuentra la llave
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
            cout << "Error al abrir el archivo de índices" << endl;
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
            
            cout << "Página " << i << " (Offset: " << index.pages[i] << ")" << endl;
            for (int j = 0; j < data.count; j++) {
                cout << "Código: " << data.records[j].codigo
                    << ", Nombre: " << data.records[j].nombre
                    << ", Apellidos: " << data.records[j].apellidos
                    << ", Ciclo: " << data.records[j].ciclo << endl;
            }
            cout << "------------------------" << endl; // Separador entre páginas
        }

        d_file.close();
    }

};


int main(){
    ISAMFile<int> isam("data.dat", "indice1.dat");
    Registro reg[4] = {
        {5, "Juan", "Perez", 1},
        {10, "Juan", "Perez", 1},
        {20, "Maria", "Lopez", 2},
        {30, "Pedro", "Garcia", 3}
    };
    for(int i = 0; i < 4; i++){
        isam.add(reg[i]);
    }
    isam.print();
    return 0 ;     
}