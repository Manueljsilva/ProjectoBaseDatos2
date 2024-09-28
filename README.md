# BD2_Proyecto1
## Integrantes:
- Manuel Silva Anampa
-
-
## Tiempo
### ISAM-Sparse Index
- 50k = 18.8515s
- 25k = 7.55716s
- 10k = 2.50212s
- 5 k = 1.01528s
- 1 k = 0.19148s


## Introducción
El proyecto **BD2_Proyecto1** tiene como objetivo desarrollar un sistema de gestión de bases de datos que implemente técnicas de indexación eficientes para el almacenamiento y recuperación de datos.

La importancia de la indexación radica en su capacidad para optimizar el acceso a grandes volúmenes de datos. En este proyecto, utilizaremos un conjunto de datos que contiene información sobre series de televisión, lo que nos permitirá evaluar el impacto de estas técnicas de indexación en el rendimiento del sistema.



## Dominio de datos
Dentro del proyecto, trabajaremos con el siguiente dataset definido para la carga de archivos .csv:
### TVSeries


### 500 k
#### Descripción de las variables (7 en total)
| **Columna**      | **Descripción**                                       |
|------------------|-------------------------------------------------------|
| ** **           |                  |
| ** **      |                            |
| ** **         |               |
| ** **        |                |
| ** **          |                        |
| ** **        |                      |
| ** **         |                              |

*Source:* link del dataset

## Resultados esperados

## Funciones implementadas
```c++
Record search(T key);
vector <Record> rangeSearch(T begin-key, T end-key);
void insert(Record registro);
void remove(T key);
```
## Organización de Archivos

Las estrategias usadas son las siguientes:
+ Sequential File
+ ISAM
+ Extendible Hashing

### Sequential File
![seqFile](.png) \



### ISAM
![Isam](.png) \

### Extendible Hashing
![extendibleHashing](.png) \


### Uso
### 1. Clonar el repositorio