#define BUF_size 1024
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

const int Page_size = 127 ; // m es el numero de llaves que puede tener una pagina
const int Data_size = 127; // M es el numero de registros que puede tener una pagina de datos
struct Registro {
    int codigo;
    char nombre[20];
    char apellidos[20];
    int ciclo;
};

struct PageIndex{
    int keys[Page_size];
    int pages[Page_size+1]; 
    int count ;
};
// m * peso de las llavves , digamos que es 4
//m*4 + (m+1)*4 + 4 = 28 // esto es el tamaño de la pagina 
// que es un buffer? un buffer es un espacio de memoria que se usa para almacenar datos temporalmente

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
    }
    bool add(Registro registro){
        // paso 0 : abrir el archivo indice 
        fstream i_file(index_file, ios::binary | ios::in | ios::out);
        if(!file.is_open()){
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
        d_file.read((char*)&data, sizeof(PageData)); // leer la pagina de datos
        // paso 5 : verificar si la pagina tiene espacio
        if(data.count < Data_size){
            data.records[data.count] = registro;
            data.count++;
            d_file.seekp(index.pages[pageIndex] * sizeof(PageData), ios::beg);
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
            if(key <= index.keys[i]){
                return index.pages[i]; // retorna la pagina donde se encuentra la llave
            }
        }
        return index.pages[index.count]; // retorna la ultima pagina
    }

    //La busqueda espesiifica puede retornar mas de un elemento que coincida con la key 
    Registro search(T key){
        //paso 1 : abrir el archivo de indices
        fstream index_file(index_file, ios::binary | ios::in | ios::out);
        if(!index_file.is_open()){
            cout << "Error al abrir el archivo de indices " << endl;
            return false;
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
    }
};


int main(){

    return 0 ;     
}