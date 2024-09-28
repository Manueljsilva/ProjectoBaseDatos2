#ifndef ISAM_3L_H
#define ISAM_3L_H
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
    char Tittle[40];     //30 a 40   
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
    PageIndex();
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



class ISAMFile{
    string data_file;
    string index1_file;
    string index2_file;
    string index3_file;
    public:
    ISAMFile(string data_file, string index1_file , string index2_file, string index3_file);
    //destrcutor
    ~ISAMFile();
    void CreateIndex1(string index_file);
    void CreateIndex2(string index_file2);
    void CreateIndex3(string index_file2, string index_file3);
    void ValueKeys(PageIndex &index);
    bool Exist_index(string index_file);
    bool Exist_data(string data_file);
    string procesarTitulo(string titulo);
    int FirstLetterToKey(char firstLetter);
    int SearchKey(const PageIndex &index, const string &key , int &nivelador ,  int & posicionLetra);
    // Función que busca la página correcta en un índice (recorriendo varios niveles).
    void SearchIndex(fstream &i_file, const string &key, PageIndex &index, int &nivelador , int &posicionLetra);
    bool add(Registro registro);
    //La busqueda espesiifica puede retornar mas de un elemento que coincida con la key 
    Registro search(const string &key);
    // La busqueda por rango retorna todos los registros que se encuentran entre las dos llaves de búsqueda
    vector <Registro> rangeSearch(const string &begin, const string &end);
    // proponer un algoritmo de elimnacion 
    bool remove(const string &key);
    void print(int limite);
    void printEstructura();
    void CantAllRegistros();
};

void test_search(ISAMFile &isam);
void testrangeSearch(ISAMFile &isam);
void CargarDatos(ISAMFile &isam, string ArchivoCSV, int limiteRegistro);

//g++ -o mi_programa main.cpp isam_3l.cpp

#endif