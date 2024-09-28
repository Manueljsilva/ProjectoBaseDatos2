#include <pybind11/pybind11.h>
#include "../ISAM_3l.h"


namespace py = pybind11;

void CargarDatosWrapper(ISAMFile &isam, const std::string &ArchivoCSV, int limiteRegistro) {
    CargarDatos(isam, ArchivoCSV, limiteRegistro);  // Llama a tu función original
}
vector<vector<string>> Beetween(ISAMFile &isam, const std::string &begin, const std::string &end) {
    vector<vector<string>> result;
    vector<Registro>  vect;
    vect = isam.rangeSearch(begin, end);
    return result;
}

vector<string> Buscar(ISAMFile &isam, const std::string &key) {
    vector<Registro>  vect;
    //vect = isam.search(key);
    //return result;
}



PYBIND11_MODULE(isam3l, m) {
    py::class_<ISAMFile>(m, "ISAMFile")
        .def(py::init<string, string, string, string>())
        .def("add", &ISAMFile::add)
        .def("search", &ISAMFile::search)
        .def("rangeSearch", &ISAMFile::rangeSearch)
        .def("print", &ISAMFile::print)
        .def("CantAllRegistros", &ISAMFile::CantAllRegistros);
    m.def("CargarDatos", &CargarDatosWrapper);
}
