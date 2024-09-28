import isam3l
import csv 

# python setup.py build_ext --inplace

try:
    isam_file = isam3l.ISAMFile("../data/data.dat" , "../data/indice1.dat"  , "../data/indice2.dat", "../data/indice3.dat")
    print("ISAM File created")
    #isam3l.CargarDatos(isam_file, "TVSeries.csv" , 50000)
    isam_file.CantAllRegistros()

except Exception as e:
    print(e)
    print("Error creating ISAM File")



    
