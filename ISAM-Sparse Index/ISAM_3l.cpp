#define BUF_size 1024
#include <iostream>
#include <fstream>
#include <vector>
/*preugnta al profe, cada index debe tener la misma size?*/


using namespace std;

//const int IndexSize2 = 2 ;
//const int IndexSize3 = 4 ;
const int Page_size = 2 ; //es el numero de llaves que puede tener una pagina //127
const int Data_size = 3; //es el numero de registros que puede tener una pagina de datos // 31

struct Registro {
    int codigo;
    char nombre[12];
    char apellidos[12];
    int ciclo;
};
struct PageIndex{
    int keys[Page_size];
    int pages[Page_size+1]; //posicion de la pagina 
    int count ;
    //constructor
    PageIndex(){
        this->count = 0;
    }
    PageIndex(int Page_size){
        this->keys[Page_size];
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
        if(!Exist_index(i_file)){
            fstream i_file(index1_file, ios::binary | ios::in);
            fstream i_file2(index2_file, ios::binary | ios::in);
            fstream i_file3(index3_file, ios::binary | ios::in);
            fstream d_file(data_file, ios::binary | ios::in);
            if(!i_file.is_open() && !i_file2.is_open() && !i_file3.is_open() && !d_file.is_open()){
                cout << "Error al abrir los archivos" << endl;
                return;
            }
            CreateIndex1(i_file);
            CreateIndex2(i_file , i_file2);
            CreateIndex3(i_file2 , i_file3);
            //cout << "i_file.tellg(): " << i_file.tellg() << endl;
            //cout << "sizeof(PageIndex): " << sizeof(PageIndex) << endl;
            i_file.close();
            i_file2.close();
            i_file3.close();

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
            d_file.close();
        } else {cout << "El archivo de datos ya existe" << endl;}

    }
    void CreateIndex1(fstream &i_file){
        PageIndex index;
        index.keys[0] = 1000 ; 
        index.pages[1] = 2000;
        i_file.write((char*)&index, sizeof(PageIndex));
        i_file.seekg(0, ios::end);
    }

    void CreateIndex2(fstream &i_file , fstream &i_file2){
        PageIndex index;
        PageIndex index2;
        index.seekg(0, ios::beg);
        index.read((char*)&index, sizeof(PageIndex));
        int div = index.keys[0]/Page_size;
        for(int i = 0; i < Page_size ; i++){
            for(int j = 0; j < Page_size; j++){
                index.keys[j] = div;
                div = div + div;  
            }
            i_file2.write((char*)&index2, sizeof(PageIndex));
        }
        i_file2.seekg(0, ios::end);
    }

    void CreateIndex3(fstream &i_file2, fstream &i_file3) {
        PageIndex index2;  // Estructura para leer el nivel 2
        PageIndex index3;  // Estructura para escribir en el nivel 3
        
        int currentPage = 0; // Contador de páginas para el nivel 3
        i_file2.seekg(0, ios::beg);  // Posicionarse al principio del archivo de índice 2
        
        while (i_file2.read((char*)&index2, sizeof(PageIndex))) {
            // Vamos a generar claves para el nivel 3 a partir de las claves del nivel 2
            for (int i = 0; i < index2.count; i++) {
                // Generar las claves del nivel 3 basadas en las claves del nivel 2
                index3.keys[i % Page_size] = index2.keys[i];
                index3.pages[i % Page_size] = currentPage;

                // Si hemos llenado una página de nivel 3, la escribimos en el archivo
                if ((i + 1) % Page_size == 0) {
                    index3.count = Page_size;
                    i_file3.write((char*)&index3, sizeof(PageIndex));
                    currentPage++; // Incrementar el contador de páginas
                }
            }

            // Si quedan claves que no llenan una página completa, escribirlas
            if (index2.count % Page_size != 0) {
                index3.count = index2.count % Page_size;
                i_file3.write((char*)&index3, sizeof(PageIndex));
                currentPage++;
            }
        }

        // Mover el cursor del archivo de índice 3 al final
        i_file3.seekg(0, ios::end);
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
            index.keys[i] = (i + 1) * 5; // Llaves inicializadas con un valor arbitrario
            index.pages[i] = i;           // Páginas apuntando a su respectiva posición
        }
        index.pages[Page_size] = Page_size; // Última página
        index.count = Page_size;            // Número de llaves en el índice
    }
    bool Exist_data(fstream &d_file){
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
        int currentPage = index.pages[pageIndex];
        d_file.seekg(index.pages[pageIndex] * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));
        // paso 5 : verificar si la pagina tiene espacio
        while(true){
            if(data.count < Data_size){
                // agregar el registro a la pagina de datos
                data.records[data.count] = registro;
                data.count++; //aumentar el contador de registros
                // escribir la pagina de datos
                //cout << "el current page es : " << currentPage << endl;
                d_file.seekp(currentPage * sizeof(PageData), ios::beg);
                //cout << "Escribiendo en la pagina de datos " << index.pages[pageIndex] << endl;
                d_file.write((char*)&data, sizeof(PageData));
                d_file.close();
                return true;
            } else if (data.count == Data_size && data.nextPage != -1){
                currentPage = data.nextPage;
                // buscar una pagina enlazada con espacio
                cout << "data.nextPage : " << data.nextPage << endl;
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
                // paso 7: escribir la nueva pagina de datos
                d_file.write((char*)&newData, sizeof(PageData));
                //cout << "d_file.tellp()"<< d_file.tellp() << endl;
                //cout << "sizeof(PageData)" <<sizeof(PageData) << endl;
                cout << "el nextpage es : "<<d_file.tellp() / sizeof(PageData) << endl;
                // paso 8: Enlazar la nueva pagina de datos
                data.nextPage = (d_file.tellp() / sizeof(PageData))-1; // obtener la posicion de la pagina de datos
                d_file.seekp(currentPage * sizeof(PageData), ios::beg);
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
                    cout << "Se encontro el registro: " << data.records[i].codigo << " " << data.records[i].nombre << " " << data.records[i].apellidos << " " << data.records[i].ciclo << endl;
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
        // 1. Abrir el archivo de índices
        fstream i_file(index_file, ios::binary | ios::in);
        if (!i_file.is_open()) {
            cout << "Error al abrir el archivo de índices" << endl;
            return result;
        }

        //2. Leer la estructura de la página de índices
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();

        // 3. Buscar la página donde podría estar la llave de inicio
        int pageIndex = search_key(index, begin);

        //4. abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in);
        if (!d_file.is_open()) {
            cout << "Error al abrir el archivo de datos" << endl;
            return result;
        }

        // 5. Leer la página de datos correspondiente
        PageData data;
        d_file.seekg(index.pages[pageIndex] * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));

        // 6. Buscar el registro en la página
        while (true) {
            for (int i = 0; i < data.count; i++) {
                if (data.records[i].codigo >= begin && data.records[i].codigo <= end) {
                    result.push_back(data.records[i]);
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


        return result;
    }
    // proponer un algoritmo de elimnacion 
    bool remove(T key){
        fstream i_file(index_file, ios::binary | ios::in | ios::out);
        if(!i_file.is_open()){
            cout << "Error al abrir el archivo de indices" << endl;
            return false;
        }
        // paso 1 : leer la estructura de la pagina de indices
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();
        // paso 2 : buscar la pagina donde se encuentra la llave
        int pageIndex = search_key(index, key);
        // paso 3 : abrir el archivo de datos
        fstream d_file(data_file, ios::binary | ios::in | ios::out);
        if(!d_file.is_open()){
            cout << "Error al abrir el archivo de datos" << endl;
            return false;
        }
        // paso 4 : leer la estructura de la pagina de datos
        PageData data;
        int currentPage = index.pages[pageIndex];
        d_file.seekg(index.pages[pageIndex] * sizeof(PageData), ios::beg);
        d_file.read((char*)&data, sizeof(PageData));
        // paso 5 : buscar el registro a eliminar
        while(true){
            for(int i = 0; i < data.count; i++){
                if(data.records[i].codigo == key){
                    // reemplazar el registro a eliminar con el ultimo registro
                    data.records[i] = data.records[data.count-1];
                    data.count--; // disminuir el tamaño de la pagina de datos
                    // reescribir la pagina de datos
                    d_file.seekp(currentPage * sizeof(PageData), ios::beg);
                    d_file.write((char*)&data, sizeof(PageData));
                    d_file.close();
                    return true;
                }
            }
            // si no se encuentra el registro en la pagina actual, verificar las paginas enlazadas
            if(data.nextPage != -1){
                currentPage = data.nextPage;
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
            }else{
                break;
            }
        }
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
            cout << "--------------Pagina ---------------" << index.pages[i] << endl;
            cout << "Count: " << data.count << endl;
            for (int j = 0; j < data.count; j++) {
                cout << "Registro " << j << ": " << data.records[j].codigo << " " << data.records[j].nombre << " " << data.records[j].apellidos << " " << data.records[j].ciclo << endl;
            }
            cout << "-------Siguiente pagina: " << data.nextPage << endl;
            //impresion de las paginas enlazadas
            while (data.nextPage != -1) {
                int pos = data.nextPage;
                d_file.seekg(data.nextPage * sizeof(PageData), ios::beg);
                d_file.read((char*)&data, sizeof(PageData));
                cout << "--------------sub Pagina ---------------" << pos << endl;
                cout << "Count: " << data.count << endl;
                for (int j = 0; j < data.count; j++) {
                    cout << "Registro " << j << ": " << data.records[j].codigo << " " << data.records[j].nombre << " " << data.records[j].apellidos << " " << data.records[j].ciclo << endl;
                }
                cout << "-------Siguiente sub pagina: " << data.nextPage << endl;
            }
            cout << endl;
        }

        d_file.close();
    }


};




void test1(ISAMFile <int> &isam){
        Registro reg[4] = {
        {5, "Juan", "Perez", 1},
        {6, "Juan", "Perez", 1},
        {7, "Maria", "Lopez", 2},
        {8, "Pedro", "Garcia", 3}
    };
    Registro reg2[4] = {
        {11 , "matias", "perez", 1},
        {12 , "manuel", "cassa", 2},
        {13 , "jose", "dsada", 1},
        {14 , "carlos", "werty", 5}
    };
    Registro regis = {9, "pepe", "elmago", 1};
    Registro regi3[4] = {
        {21 , "hjkl", "perez", 1},
        {22 , "cvb", "cassa", 2},
        {23 , "fds", "dfghj", 1},
        {24 , "fdsa", "asdfgh", 5}
    };

    for(int i = 0; i < 4; i++){
        isam.add(reg[i]);
    }
    for(int i = 0; i < 4; i++){
        isam.add(reg2[i]);
    }
    //volver a instar un registro
    isam.add(regis);
}

void testrangeSearch(ISAMFile<int> &isam){
    vector<Registro> registros = isam.rangeSearch(5, 8);
    for(Registro reg : registros){
        cout << reg.codigo << " " << reg.nombre << " " << reg.apellidos << " " << reg.ciclo << endl;
    }
}

int main(){
    ISAMFile<int> isam("data.dat", "indice1.dat" , "indice2.dat", "indice3.dat");
    //test1(isam);

    isam.print();
    //probando el search
    isam.search(9);
    //probando el rangeSearch
    testrangeSearch(isam);

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