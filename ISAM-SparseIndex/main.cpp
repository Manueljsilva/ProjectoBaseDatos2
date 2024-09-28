#include "ISAM_3L.h"
#include <chrono>

int main(){
    ISAMFile isam("data/data.dat", "data/indice1.dat", "data/indice2.dat", "data/indice3.dat");
    //test2(isam);
    //isam.printEstructura();
    //------------------------------tomando tiempo con chrono
    auto start = std::chrono::high_resolution_clock::now();
    //----------------------------------Cargar datos
    CargarDatos(isam, "TVSeries.csv", 50000);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Tiempo de carga: " << elapsed_seconds.count() << "s\n";

    //--------------------------------probando el search
    //test_search(isam);
    //--------------------------------probando el rangeSearch
    //testrangeSearch(isam);
    //--------------------------------imprimir cierta cantidad de registros(no cuenta desbordamientos)
    //isam.print(100);
    //--------------------------------todos los registros
    isam.CantAllRegistros();
    return 0 ;     
}
// ejecutar g++ -o mi_programa main.cpp isam_3l.cpp
