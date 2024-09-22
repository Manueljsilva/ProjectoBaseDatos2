#define BUF_size 8192
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
/*preugnta al profe, cada index debe tener la misma size?*/


using namespace std;

//const int IndexSize2 = 2 ;
//const int IndexSize3 = 4 ;
const int Page_size = 2; //es el numero de llaves que puede tener una pagina //127
const int Data_size = 3; //es el numero de registros que puede tener una pagina de datos // 31

// Page_size * peso de las llavves , digamos que es 4
// Page_size*4 + (Page_size+1)*4 + 4 = 8192 // esto es el tamaño de la pagina
// page_size = 10
// page_size = 1000 en el 3er nivel

// hallando el tamaño de la pagina
//  PageData=(tamano de Registro × Data_size)+4(count)+4(nextPage)
//  calculando el tamaño de la página de     datos
//  tamaño de registro = 4+12+12+4 = 32 bytes
//  32*data_size + 4 + 4 = 1024 ; data_size = 32 o era 23? , **verificar  bien **  

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
            cout<<"Creando indices"<<endl;
            CreateIndex1(index1_file);
            CreateIndex2(index1_file, index2_file);
            CreateIndex3(index2_file, index3_file);
            //cout << "i_file.tellg(): " << i_file.tellg() << endl;
            //cout << "sizeof(PageIndex): " << sizeof(PageIndex) << endl;

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

    void CreateIndex2(string index_file, string index_file2){
        fstream i_file(index_file, ios::binary | ios::in | ios::out);
        PageIndex index;
        i_file.seekg(0, ios::beg);
        i_file.read((char*)&index, sizeof(PageIndex));
        i_file.close();
        fstream i_file2(index_file2, ios::binary | ios::out | ios::in);
        
        cout << "Creando el archivo de indices 2" << endl;
        //asignando valores a las llaves del nivel 2
        int ValorAumento = index.keys[0]/Page_size;
        int ValorEstatic = index.keys[0]/Page_size;
        for(int i = 0; i < Page_size ; i++){
            PageIndex index2;
            index2.count = 0;
            for(int j = 0; j < Page_size; j++){
                index2.keys[j] = ValorAumento;
                index2.pages[j] = j;
                index2.count++;
                ValorAumento = ValorAumento + ValorEstatic;
            }
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
        //Asignando valores a las llaves del nivel 3
        int ValorAumento = index2.keys[0] / Page_size;
        int ValorEstatic = index2.keys[0] / Page_size;
        i_file2.close();

        //crear las paginas de nivel 3 para cada key del nivel 2
        for(int i = 0; i < CantKeys; i++){
            PageIndex index3;
            index3.count = 0;
            for(int j = 0; j < Page_size; j++){
                index3.keys[index3.count] = ValorAumento;
                index3.pages[index3.count] = j;
                index3.count++;
                ValorAumento = ValorAumento + ValorEstatic;
            }
            i_file3.write((char*)&index3, sizeof(PageIndex));
        }

        i_file3.close();
    }



    void ValueKeys(PageIndex &index) {
        for (int i = 0; i < Page_size; i++) {
            index.keys[i] = (i + 1) * 1000; // Llaves inicializadas con un valor arbitrario
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
        
        if (firstLetter >= 'a' && firstLetter <= 'e') {
            return 1000;
        } else if (firstLetter >= 'f' && firstLetter <= 'j') {
            return 2000;
        } else if (firstLetter >= 'k' && firstLetter <= 'o') {
            return 3000;
        } else if (firstLetter >= 'p' && firstLetter <= 't') {
            return 4000;
        } else if (firstLetter >= 'u' && firstLetter <= 'z') {
            return 5000;
        } else {
            return -1; // En caso de un carácter no soportado.
        }
    }
    int SearchKey(const PageIndex &index, const string &key) {
        // Procesar la clave para eliminar palabras comunes.
        string newKey = procesarTitulo(key);
        // Obtener la primera letra de la clave.
        char firstLetter = newKey[0];
        // Convertir la primera letra en una clave numérica.
        int keyNumber = FirstLetterToKey(firstLetter);
        // Buscar la clave en el índice.
        for (int i = 0; i < index.count; i++) {
            if (keyNumber <= index.keys[i]) {
                return i;
            }
        }
        return index.count;
    }

    bool add(Registro registro){
        // paso 0 : abrir el archivo indice 
        fstream i_file(index1_file, ios::binary | ios::in | ios::out);
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
        int pageIndex = SearchKey(index, registro.Tittle); // buscar la pagina donde se encuentra la llave
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

    //La busqueda espesiifica puede retornar mas de un elemento que coincida con la key 
    Registro search(T key){
        // 1. Abrir el archivo de índices
        fstream i_file(index1_file, ios::binary | ios::in);
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
                if (data.records[i].Tittle == key) {
                    d_file.close();
                    cout << "Se encontro el registro: " << data.records[i].Tittle << endl;
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
        fstream i_file(index1_file, ios::binary | ios::in);
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
                if (data.records[i].Tittle >= begin && data.records[i].Tittle <= end) {
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
        fstream i_file(index1_file, ios::binary | ios::in | ios::out);
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
        int pageIndex = SearchKey(index, key);
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
                if(data.records[i].Tittle == key){
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
        ifstream i_file(index1_file, ios::binary | ios::in);
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
            cout << "--------------Pagina ---------------" << index.pages[i] << "key: " << index.keys[i] << endl;
            cout << "Count: " << data.count << endl;
            for (int j = 0; j < data.count; j++) {
                cout << "Registro " << j << ": " << data.records[j].Tittle <<endl;
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
                    cout << "Registro " << j << ": " << data.records[j].Tittle << endl;
                }
                cout << "-------Siguiente sub pagina: " << data.nextPage << endl;
            }
            cout << endl;
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
            for(int i = 0; i < index3.count; i++){
                contador2++;
                cout << "Key: " << index3.keys[i] << " Page: " << index3.pages[i] << " Count: " << index3.count << endl;
            }
        }
        i_file3.close();
        cout << "contador de paginas: " << contador << endl;
        cout << "contador de keys: " << contador2 << endl;
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

void testrangeSearch(ISAMFile<int> &isam){
    //vector<Registro> registros = isam.rangeSearch("Melicual1", "Melicual2");
    //for(Registro reg : registros){
    //    cout <<reg.Tittle << endl;
    //}
}




int main(){
    ISAMFile<int> isam("data.dat", "indice1.dat" , "indice2.dat", "indice3.dat");
    //test1(isam);
    Registro regis = {"The Witcher", "2019– ", "60 min", "Action, Adventure, Drama", "8.2", "Henry Cavill, Freya Allan, Anya Chalotra, Mimi Ndiweni", "Geralt of Rivia, a solitary monster hunter, struggles to find his place in a world where people often prove more wicked than beasts."};
    //isam.add(regis);
    //isam.print();
    isam.printEstructura();
    //probando el search
    //isam.search(9);
    //probando el rangeSearch
    //testrangeSearch(isam);

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