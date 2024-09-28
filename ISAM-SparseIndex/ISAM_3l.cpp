#define BUF_size 8192
#include "ISAM_3l.h"

PageIndex::PageIndex(){this->count = 0;}

ISAMFile::ISAMFile(string data_file, string index1_file , string index2_file, string index3_file){
    this->data_file = data_file;
    this->index1_file = index1_file;
    this->index2_file = index2_file;
    this->index3_file = index3_file;

    // si el indice no existe, se crea uno nuevo
    if(!Exist_index(index1_file)){
        CreateIndex1(index1_file);
        CreateIndex2(index2_file);
        CreateIndex3(index2_file, index3_file);
    }

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
    }
}

ISAMFile::~ISAMFile(){cout << "Cerrando el archivo" << endl;}
void ISAMFile::CreateIndex1(string index_file){
    fstream i_file(index_file, ios::binary | ios::out | ios::in);
    PageIndex index;
    ValueKeys(index);
    i_file.write((char*)&index, sizeof(PageIndex));
    i_file.seekg(0, ios::end);
    i_file.close();
}
void ISAMFile::CreateIndex2(string index_file2){
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
void ISAMFile::CreateIndex3(string index_file2, string index_file3) {
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
void ISAMFile::ValueKeys(PageIndex &index) {
    for (int i = 0; i < Page_size; i++) {
        index.keys[i] = (i + 1) * 100; // Llaves inicializadas con un valor arbitrario
        index.pages[i] = i;           // Páginas apuntando a su respectiva posición
    }
    index.pages[Page_size] = Page_size; // Última página
    index.count = Page_size;            // Número de llaves en el índice
}
bool ISAMFile::Exist_index(string index_file){
    fstream i_file(index_file, ios::binary | ios::in | ios::out);
    if (!i_file.is_open()) {
        cout << "El archivo no abre_ole" << endl;
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
bool ISAMFile::Exist_data(string data_file){
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
string ISAMFile::procesarTitulo(string titulo){
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
int ISAMFile::FirstLetterToKey(char firstLetter) {
    // Normalización de mayúsculas a minúsculas.
    firstLetter = tolower(firstLetter);
    // Convertir la primera letra en una clave numérica.
    if (firstLetter >= 'a' && firstLetter <= 'c') {
        return 100;
    } else if (firstLetter >= 'd' && firstLetter <= 'f') {
        return 200;
    } else if (firstLetter >= 'g' && firstLetter <= 'i') {
        return 300;
    } else if (firstLetter >= 'j' && firstLetter <= 'l') {
        return 400;
    } else if (firstLetter >= 'm' && firstLetter <= 'o') {
        return 500;
    } else if (firstLetter >= 'p' && firstLetter <= 'r') {
        return 600;
    } else if (firstLetter >= 's' && firstLetter <= 'u') {
        return 700;
    } else if (firstLetter >= 'v' && firstLetter <= 'x') {
        return 800;
    } else if (firstLetter >= 'y' && firstLetter <= 'z') {
        return 900;
    } else {
        return 1000;}
}

int ISAMFile::SearchKey(const PageIndex &index, const string &key , int &nivelador ,  int & posicionLetra) {
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
void ISAMFile::SearchIndex(fstream &i_file, int &currentPage, const string &key, PageIndex &index, int &nivelador , int &posicionLetra) {
    //cout << "nivelador pero en el search: " << nivelador << endl;
    i_file.seekg(nivelador * sizeof(PageIndex), ios::beg);
    i_file.read((char*)&index, sizeof(PageIndex));

    int pageIndex = SearchKey(index, key , nivelador , posicionLetra);
    //cout << "pageIndex: " << pageIndex << endl;
    currentPage = index.pages[pageIndex]; // Actualizar la página actual
}
bool ISAMFile::add(Registro registro) {
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
Registro ISAMFile::search(const string &key){
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
                cout << "Registro encontrado: " << key << endl;
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
    return Registro();
}
vector <Registro> ISAMFile::rangeSearch(const string &begin, const string &end){
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
bool ISAMFile::remove(const string &key){
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
                cout << "Se elimino el registro: " << key << endl;
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
void ISAMFile::print(int limite){
    ifstream d_file(data_file, ios::binary | ios::in);
    if (!d_file.is_open()) {
        cout << "Error al abrir el archivo de datos" << endl;
        return;
    }

    PageData data;
    int pageCounter = 0;  // Contador de páginas
    streampos lastPosition;  // Guardar la última posición después de leer la página principal

    // Leer cada página en el archivo
    while (d_file.read((char*)&data, sizeof(PageData)) && pageCounter < limite) {
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
void ISAMFile::printEstructura() {
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
void ISAMFile::CantAllRegistros(){
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
    }

    d_file.close();
    cout <<"----------------------------------------------" <<endl;
    cout << "Cantidad de registros: " << contador << endl;
}



void test_search(ISAMFile &isam){

}
void testrangeSearch(ISAMFile &isam){
    vector<Registro> result = isam.rangeSearch("Alice in Borderland", "Stranger Things");
    for (int i = 0; i < result.size(); i++) {
        cout << "Tittle: " << result[i].Tittle << endl;
        cout << "ReleaseYear: " << result[i].ReleaseYear << endl;
        cout << "Runtime: " << result[i].Runtime << endl;
        cout << "Genre: " << result[i].Genre << endl;
        cout << "Rating: " << result[i].Rating << endl;
        cout << "Cast: " << result[i].Cast << endl;
        cout << "Synopsis: " << result[i].Synopsis << endl;
        cout << "------------------------------------" << endl;
    }
}
void CargarDatos(ISAMFile &isam, string ArchivoCSV, int limiteRegistro){
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
            cout << "Se han cargado " << contador << " registros" << endl;
            break;
        }
    }
}
