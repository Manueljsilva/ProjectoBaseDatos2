import sqlparse
import isam3l

# Instancia de ISAMFile correctamente inicializada
isam_file = isam3l.ISAMFile("data.dat", "indice1.dat", "indice2.dat", "indice3.dat")

def parse_sql(query):
    # Parsear la consulta SQL
    parsed = sqlparse.parse(query)[0]
    statement_type = parsed.get_type()

    if statement_type == 'SELECT':
        handle_select(parsed)
    else:
        print(f"Sentencia no soportada: {statement_type}")

def handle_select(parsed):
    print("Procesando SELECT statement")

    table_name = None
    where_clause = None

    # Imprimir todos los tokens y sus tipos para depurar
    for token in parsed.tokens:
        print(f"TOKEN: {token}, TYPE: {type(token)}, VALUE: {token.value}")
        pass

    # Intentamos extraer la tabla y la cláusula WHERE
    for token in parsed.tokens:
        # Buscar el nombre de la tabla después de FROM
        if token.ttype is sqlparse.tokens.Keyword and token.value.upper() == 'FROM':
            next_token = parsed.token_next(parsed.token_index(token))
            table_name = next_token[1].value if next_token else None
            print(f"Tabla: {table_name}")
        
        # Buscar la cláusula WHERE
        if isinstance(token, sqlparse.sql.Where):  # Cambiamos la condición aquí
            where_clause = token.value
            print(f"Condición WHERE: {where_clause}")

            # Procesar la condición WHERE (e.g., Tittle = "nombre del titulo")
            field_name, field_value = process_where_clause(where_clause)
            print(f"Condición WHERE procesada: {field_name} = {field_value}")

            # Realizar la búsqueda en la estructura ISAM
            if field_name == "Tittle":
                print(field_value)
                result = isam_file.search(field_value)
                print(result)
            
            else:
                print("Campo WHERE no soportado")

def process_where_clause(where_clause):
    # Eliminar la parte "WHERE " y procesar la cláusula WHERE para extraer el campo y el valor
    # Divide en '=' y quita espacios extra
    tokens = where_clause.split('=')
    field_name = tokens[0].replace("WHERE", "").strip()  # Campo como "Tittle"
    field_value = tokens[1].strip()  # Valor como '"nombre del titulo"'
    return field_name, field_value

# Ejemplo de uso
query = 'SELECT * FROM PELICULAS WHERE Tittle = Jack Ryan'
parse_sql(query)
