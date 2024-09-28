# BD2_Proyecto1
## Integrantes:
- Manuel Silva Anampa
- Anderson Carcamo Vargas
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

Se han planteado tres estructuras de datos para almacenar registros bajo una llave que se recibe como imput. De las cuales tenemos el Sequential File, que se basa en una busqueda binaria en una parte ordenada perfecta para busquedas como rangeSearch; extendible Hashing para optimizar insercciones, y un ISAM que optimiza accesos a memoria y cargas de buffer.

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

### Consideraciones:

Para la implementaciòn se han considerado:
- Registros estáticos de la forma:

``` c++
struct Registro {
    char title[100];         // Título de la serie
    char releaseYear[10];    // Año de lanzamiento
    char runtime[10];        // Duración
    char genre[100];          // Género
    char rating[5];          // Calificación
    char cast[200];          // Reparto
    char synopsis[400];      // Sinopsis

    int posNext;             // Posición del siguiente registro
    char nextEspacioType;    // Puede ser 'a' (auxiliar) o 'd' (datos), 'e' (eliminado)
};
```

***Se asume que si pasan del registro limitado se acorta el campo.***

- Archivos guardados con el formato: **seqFile_<nombre_archivo>_<nombre_key>.dat**

- Usa una lista enlazada simple entre registros:

```
Reg1 -> Reg2 -> Reg3 -> ... -> RegN
```

- Lìmite de registros ``k:``  $log_2N$

- Puede recibir solo como pk el title por condiciones de la tabla, pero pueden añadirse otras opciones.

- No considera repetidos

### Funciones Implementadas:

| **Funciones**      | **Parametros**  | **Funcionalidad**|
|------------------|--------------------------------------|-----------------|
| ``add(Registro registro)``           |  recibe un tipo Registro | añade un registro al SeqFile |
| ``search(PK key)``      |    recibe una key de tipo PK          |  busca un registro |
| ``rangeSearch(PK beginKey, PK endKey)``         |     recibe un key entre beginKey y endKey          |  obtiene registros entre las llaves input|
| ``removeKey(PK key)``        |      recive la llave de busqueda para eliminar      | remueve un registro |
| ``reconstruir()``         |     -              | reconstuye el archivo usando la lista enlazada |
| ``getPrimaryKeyFromRegistro(Registro& reg, string nameKey)``      |        recibe el registro y nombre de la llave              |  retorna la llave de nombre nameKey del registro |


#### **Add**

Complejidad estimada: BusquedaBinaria + Busqueda Lineal: $O(log_2N + k)$

- Optmizaciones:
  - Para N=1: insercccion al ultimo y actulizacion de N
  - Para N=2: aumento de N y añadido del registro
  - Para N>=3:

    Se obtiene el registro mas cercano al registro que quiero obtener en la parte ordenada. Considera las salidas de ``-2`` y ``-1``. El primero significa que el registro ya existe, el segundo, que el registro anterior es el inicial.
    
    Se inserta segun el registro obtenido.

- Pseodocodigo:

``` c++
add(Registro registro):

  // lee N y nameKey de la cabecera del registro

  if N == 0:
    N++;
    registro -> -1d;
    // escritura del registro

  else if N == 1:
    r1 -> carga el registro en el archivo
    N++;
    if r1.nameKey > registro.nameKey:
      registro -> 1d
      r1 -> -1d
      // escritura de ambos registros
    else
      r1 -> 1d
      registro -> -1d
      // escritura de registros
  
  pos -> posicion cercana

  currentReg -> registro en posicion cercana

  if currentReg.nextEspacioKey == 'a':
    // existen registros auxiliares proximos al registro
    // punteros para guardar enlazados previo, current y next
    prevPrevPos
    prevPos
    nextPos
    while currentReg == 'a':      
      if currentReg.nameKey > registro.nameKey:
        // el registro se debe añadir con los enlazados
        break;

      if currentReg.nameKey == registro.nameKey :
        break;

      currentReg = currentReg.next;
      prevPrevPos = prevPos;
      prevPos = nextPos;
      nextPos = currentReg.next;

    // realizo el enlazamiento del registro

  else if currentReg.nextEspacioKey == 'd':
    // inserccion en la parte auxiliar directa

  k -> total de registros auxiliares
  if k >= log_2(N) reconstruir();

```

#### **Search**

Uso de busqueda binaria en parte ordenada y busqueda lineal en registrados auxiliares.

Comlejidad: $O(log_2N + k)$, k: cantidad de registros auxiliares

- Pseudocodigo:

``` c++
search(PK key):
  N -> cantidad de registros
  nameKey -> nombre de la llave

  low -> 0
  high -> N -1

  while (low <= high) :
    mid = (low+high)/2

    reg -> lectura de registro en posicion mid

    if reg.nextEspacioType == 'e': 
      low = mid+1
      saltar

    if reg.nameKey == key: return reg
    else if registro < key: low = mid + 1
    else high = mid +1

  // en caso no lo encuentra puede existir en sus enlazados
  mid--;
  while(reg.nextEspacioType == 'e'):
    // retroceder
  
  if reg.nextEspacioType == 'a':
    while reg.nextEspacioType != 'd':
      if reg.nameKey == key: return reg
      mid = reg.posNext + N;

    // busqueda en el ultimo registro

  // no lo encontro
```

#### **RangeSearch**

Busqueda binaria del beginKey, o del mas proximo. Luego de eso se itera hasta encontrar uno que pase el endKey.

Complejidad: $O(log_2N + R)$, donde R son los registros entre el rango.

- Pseudocodigo:

``` c++
rangeSearch(PK keyStart, PK keyEnd):
  N -> cantidad de registros
  nameKey -> nombre de key en la bd

  pos -> registro mas cercado a keyStart
  if pos == -1 : pos = 0

  reg -> reg en la posicion pos
  registros -> vector de registros

  while true:
    if reg.namekey >= keyStart and reg.nameKey <= keyEnd and reg.nexEspacioType != 'e':
      resultad.add(reg)
    
    if reg.nameKey > keyEnd: break

    if reg.nextEspacioType == 'a' and reg.posNext != -1:
      auxPos = N + reg.posNext
      posicion en archivo -> auxPos
    else if reg.posNext != -1:
      posicion en archivo -> reg.posNext
    else break

    reg -> reg en la posicion del archivo

  return registros

```

#### **RemoveKey**

Remueve el archivo. Para ello lo ubica, y actualiza los punteros los enlazamientos de los registros conectados.

Complejidad: $O(log_2N) + C$, C es el tiempo en actualizar sus punteros conectados, se asume O(1)

- pseudocodigo:

``` c++
removeKey(PK key):
  N -> cantidad de registros
  
  posPrev -> registro previo para eliminar

  Registros prevReg, reg
  posCurrent

  prevReg -> registro en posPrev

  if prevReg.nameKey != key:
    if prevReg.nextEspacioType == 'd': return false

    else if prevReg.nextEspacioType == 'a':
      aux -> posPrev
      aux2 -> prevReg.posNext + N
      prevReg -> registro en (prevReg.posNext + N)

      while prevReg.nameKey != key:
        if prevReg.nextEspacioType == 'd': return false

        aux = aux2
        aux2 = prevReg.posNext + N
        prevReg -> registro en aux2
      
      prevReg -> registro en aux
      posPrev = aux;
  else 
    if posPrev != 0:
        // indica que el registro a encontrar es el registro y tengo que volver atras
    else:
      // leo el registro en posPrev

  while (true):
    if prevReg.nextEspacioType -> 'd' and prevReg.posNext -> -1:
      posCurrent = prevReg.posNext
    else if prevReg.nextEspacioType == 'a':
      posCurrent = N + prevReg.posNext
    else
      return false
    
    reg -> registro en posCurrent

    if reg.nameKey == key: break

    else
      if prevReg.nextEspacioType == 'd': break

    prevReg = reg;
    posPrev = posCurrent

  if posPrev == 0:
    // eliminare un registro inicial
    if prevReg.nextEspacioType == 'a' and prevReg.posNext != -1
      // existe un auxliar que lo puede reemplazar
    
    else 
      // no existe se sobrepone con el siguiente registro
  else:
    // Actualizo el puntero anterior para saltar el eliminado

  reg.nextEspacioType -> e // indica eliminado
  // escribo el registro en posCurrent

  return true

```


#### **Reconstruir**

Reconstuye usando la lista enlazada. Lee los registros del archivo y los escribe en otro. Luego este toma su lugar. De esta manera se pierden los eliminados por siempre.

Complejidad: $O(N)$

- pseudocodigo:

``` c++
reconstruir():
  N -> cantidad de registros en la parte ordenada
  nameKey -> nombre de la llave en la bd
  registros -> lista de registros
  reg -> registro current

  while true:
    if reg NO es Eliminado:
      registros.add(reg)
    if reg.next es un auxiliar
      // recorrer todos sus axuliares
      while (reg.nextEspacioType != 'd')
        // actualizo el reg en cada enlamiento
        registros.add(reg)
      if reg.posNext == -1: break
    
    if reg.posNext == -1: // esta en el final
      registros.add()
    
  // abro archivos
  for reg: registros:
    // escribo en archivos, pues ya esta ordenado
  
  remove(fileData)
  rename(tempfile, fileData)


```

#### **GetPrimaryKeyFromRegistro**

Funciona al igual que un diccionario, recibe el nombre de la key que quiero obtener y lo extraigo del registro.

Complejidad: ``O(1)``

- codigo:

``` c++
PK getPrimaryKeyFromRegistro(Registro& reg, string nameKey) {
    if (nameKey == "title") {
        return string(reg.title);
    } else if (nameKey == "releaseYear") {
        return string(reg.releaseYear);
    }
    cout << "Error: clave no existe, se ha puesto title como clave" << endl;
    return string(reg.title);
}

```

### Ventajas y Desventajas:

1. ``Reconstruir`` demasiado caro. Las probabiliades de que se llene la parte auxiliar es alta.
2. Uso de espacio adicional para puntero
3. Facil abstraccion de logica
4. Uso de busqueda lineal

### Optimizaciones posibles:

1. Uso de un double List para acceso anterior y siguiente.
2. Uso de metadata para registros variables.

### Tabla de tiempos:

| N | 1k | 5k | 10k | 25k |

| **N**      | **1K**  | **5K** | **10K** | **25K** | **50K** |
|------------|---------|--------|---------|---------|---------|
|   insert(N registros) | 262.157 ms | 3800.75 ms | 3518.45 ms | 78941.2 ms | 297351 ms |
|   search(1 registro) | 0.044204 ms | 0.037797 ms | 0.043386 ms | 0.078036 ms | 0.045971 ms |
|   rangeSearch(R registros) | 1.40291 ms (415 registros) | 6.69814 ms (2250 registros) | 24.9292 ms (8640 registros) | 27.13 ms (8484 registros) | 37.0832 ms (14984registros) |
|   removeKey(1 registros) | 0.076774 ms| 0.069757 ms | 0.080299 ms | 0.100157 ms | 0.121785 ms|


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