import sys
import os
import read
import show
import dijkstra

def validations():
    # Validar versión de Python
    if sys.version_info[0] != 3: print("\n\t> No estás utilizando Python 3\n"); sys.exit(1)

    # Validar argumentos
    if len(sys.argv) < 2: print("\n\t> Use: python LinkState.py <param1>\n"); sys.exit(1)

    # Validar existencia del archivo
    global file_path 
    file_path = sys.argv[1]
    if not os.path.exists(file_path): print(f"\n\t> El archivo {file_path} no existe.\n"); sys.exit(1)


# Inicio de Programa 
if __name__ == "__main__":
    validations()

    # Lectura de Archivo TXT
    initNode, routeDict = read.routingFile(file_path)

    # Imprime la Tabla de Ruteo
    show.routeStruct(initNode, routeDict)

    ###################################################
    # Lo siguiente es una sugerenica de implementación

    # Retorna y ejecuta los Pasos con el Algoritmo Dijkstra
    stepsCalcs = dijkstra.init(initNode, routeDict)
    ##### Ejemplo de resultado de la función
    ##### se utilizó el archivo topo3.txt que es 
    ##### el mismo ejemplo 3 del documento del Laboratorio
    stepsCalcs = [
        (0, "7, a", "3, a", "7, a", "a"),
        (1, "7, a", "3, a", "5, c", "c"),
        (2, "6, d", "3, a", "5, c", "d"),
        (3, "6, d", "3, a", "5, c", "b"),
    ] ##### Puedes implementar tu propia estructura.

    # Imprime los Pasos generados del Algoritmo
    show.stepsCalculatingRoute(initNode, routeDict, stepsCalcs)












