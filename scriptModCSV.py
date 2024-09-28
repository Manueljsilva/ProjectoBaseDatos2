import pandas as pd
import random
import string

# Función para generar una palabra o número aleatorio
def generar_sufijo_aleatorio():
    # Genera un sufijo alfanumérico de 10 caracteres
    sufijo = ''.join(random.choices(string.ascii_letters + string.digits, k=25))
    return sufijo

# Cargar el archivo CSV
df = pd.read_csv('Sequential-File/TV Series.csv')

# Diccionario para llevar el conteo de títulos únicos
titulos_vistos = {}

# Iterar sobre cada fila para asegurarse de que los títulos sean únicos
for idx, row in df.iterrows():
    titulo_original = row['Series Title']
    
    # Si el título ya fue visto, agregar un sufijo aleatorio
    if titulo_original in titulos_vistos:
        nuevo_titulo = titulo_original + '_' + generar_sufijo_aleatorio()
        while nuevo_titulo in titulos_vistos:  # Asegurarse de que el nuevo título no exista
            nuevo_titulo = titulo_original + '_' + generar_sufijo_aleatorio()
        df.at[idx, 'Series Title'] = nuevo_titulo
        titulos_vistos[nuevo_titulo] = 1
    else:
        titulos_vistos[titulo_original] = 1

# Guardar el archivo CSV con los títulos modificados
df.to_csv('TV Series_modificado.csv', index=False)

print("El archivo CSV ha sido modificado y guardado como 'TV Series_modificado.csv'")
