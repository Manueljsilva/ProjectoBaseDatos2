#include "ISAM_3L.h"
#include <chrono>

int main(){
    ISAMFile isam("data.dat", "indice1.dat", "indice2.dat", "indice3.dat");
    //test2(isam);
    //isam.printEstructura();
    //tomando tiempo con chrono
    auto start = std::chrono::high_resolution_clock::now();
    ///////////////////////////////cargar datos
    CargarDatos(isam, "TVSeries.csv", 50000);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Tiempo de carga: " << elapsed_seconds.count() << "s\n";

    ///////////////////////////////probando el search
    //test_search(isam);
    ///////////////////////////////todos los registros
    isam.CantAllRegistros();
    ///////////////////////////////probando el rangeSearch
    //testrangeSearch(isam);
    //isam.print(10);
    return 0 ;     
}
// ejecutar g++ -o mi_programa main.cpp isam_3l.cpp
