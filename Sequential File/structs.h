#include <iostream>

using namespace std;

struct Registro
{
    // estructura estatica de un registro prueba
    char codigo[5];
    char nombre[20];
    char apellidos[20];
    int ciclo;

    int posNext; // pos siguiente 0>= : posicion en aux o en dat,
    char espacioFile; // puede ser a: auxiliar o d: datos, e:elimnado
};