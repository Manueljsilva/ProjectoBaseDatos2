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

## Sequential File
![seqFile](.png) \



## ISAM con Índice Disperso
![Isam](.png) \

Se realizo la técnica de indexación ISAM (Indexed Sequential Access Method) en donde se pudo implementar las operaciones de búsqueda, inserción y eliminación en grandes volúmenes de datos.

#### Funcionamiento de ISAM

El funcionamiento básico de ISAM consiste en:
1. **Estructura de Páginas**: Los datos se organizan en páginas, donde cada página contiene un número fijo de registros. Para este proyecto, se asumió un tamaño de página de 8192 bytes. 
   - **Cálculos de Tamaño de Página**:
     - Para el índice:<br> 
       Page_size * 4 + (Page_size + 1) * 4 + 4 = 8192 <br>
       Lo que resultó en un tamaño de página de 1023 para el tercer nivel, por ende en el primer nivel se tiene como tamaño de página igual a 10.
     - Para las páginas de datos:<br>
     Sabiendo que cada registro tiene un tamaño de 354 bytes, se tiene que:<br>
       8192 / 354 ≈ 23 registros por página
        

2. **Inserción**: Cuando se inserta un nuevo registro, se busca la posición correspondiente en el índice. Si la página de destino está llena, se maneja el overflow creando páginas enlazadas, lo que permite almacenar más datos sin perder la capacidad de búsqueda.

3. **Búsqueda**: La búsqueda se realiza utilizando el índice para localizar rápidamente la página que contiene el registro deseado. También se implementó la búsqueda por rangos, lo que permite obtener todos los registros que se encuentran dentro de un rango específico de claves.

4. **Eliminación**: La eliminación de registros se lleva a cabo de manera similar a la inserción; Se hizo uso de la tecnica de mover el ultimo registro de la pagina a la posicion del registro a eliminar y el espacio donde estaba el ultimo registro sera reutilizado para futuras inserciones.


#### Avances y Resultados

Hasta el momento, he logrado implementar con éxito:
- **Inserción**: El registro se inserta adecuadamente, y el manejo del overflow mediante páginas enlazadas está funcionando correctamente.
- **Búsqueda**: Se ha implementado la búsqueda simple y la búsqueda por rangos.
- **Eliminación**: La eliminación de registros ha sido completada.

Sin embargo, no se ha logrado completar el parser en Python ni la interfaz gráfica. A pesar de esto, se pudo realizar una integración de Python con C++, lo que establece una base sólida para futuras mejoras.
#### Ejecución del Programa

Para compilar y ejecutar el programa, utiliza el siguiente comando:

```bash
cd ISAM-SparseIndex
g++ -o mi_programa main.cpp isam_3l.cpp
./mi_programa
```

## Extendible Hashing
![extendibleHashing](.png) \


### Uso
### 1. Clonar el repositorio