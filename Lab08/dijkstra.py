'''
Diccionario para guardado de rutas y nodos previos en cada paso
'''
shortestPath = {}

'''
Función de costo desde w a v, donde w y v son vecinos.
Retorna  c(w, v) el calculo de resultante de los vecinos w a v
'''
def c(w: dict, v: str) -> int:
	return w[v]

'''
Función de Menor costo desde el nodo de origen hasta el destino v.
Retorna  D(v) su costo actual de v
'''
def D(v: str) -> int:
	if isinstance(shortestPath[v], tuple):
		return shortestPath[v][0]
	else:
		return int(1e12)

'''
Función de Nodo anterior vecino de v
Retorna  p(v)  el nombre del nodo anterior vecino de v
'''
def p(v: str) -> str:
	if isinstance(shortestPath[v], tuple):
		return shortestPath[v][1]
	else:
		return ""

'''
Función de utilidad para actualizar estructura de pasos
Convierte diccionario tupla de strings
'''
def step(ite: int, nodes: dict, n: str) -> tuple:
	lst = []
	lst.append(ite)
	for nd in nodes.values():
		if isinstance(nd, float):
			lst.append("∞")
		elif isinstance(nd, tuple) and len(nd) == 2:
			lst.append(f"{nd[0]}, {nd[1]}")
	lst.append(n)
	return tuple(lst)

'''
Función donde Inicia la Ejecución del Algoritmo de Dijkstra
Retorna una Estructura 
'''
def init (initNode: str, routeDict: dict) -> list:
	stepsCalcs = []
	N = []
	u = routeDict[initNode] # vecinos de u

	# Inicialización
	N.append(initNode)
	for v in sorted(routeDict):
		if v == initNode:
			continue # ignorar nodo inicial
		if v in u:
			# D(v) = c(u, v)
			# p(v) = u
			shortestPath[v] = (c(u, v), initNode)
		else:
			# D(v) = ∞
			shortestPath[v] = float('inf')
	stepsCalcs.append(step(0, shortestPath, initNode))

	# Loop
	nodes = len(routeDict)
	ite = len(N)
	while (ite < nodes):
		# Búsqueda de mínimo
		nxt = None
		lowest = None
		for n in shortestPath:
			if n in N:
				continue # saltar nodos en N
			if isinstance(shortestPath[n], tuple):
				if lowest == None or shortestPath[n][0] < lowest:
					lowest = shortestPath[n][0]
					nxt = n
		# Añadir w a N
		N.append(nxt)

		# Actualizar D(v) para cada vecino que no esté en N
		w = routeDict[nxt]
		for v in sorted(routeDict):
			if v in w and v not in N:
				# D(v) = min(D(v), D(w) + c(w, v))
				# p(v) minimo entre w y v
				if (D(v) <= D(nxt) + c(w, v)):
					shortestPath[v] = (D(v), p(v))
				else:
					shortestPath[v] = (D(nxt) + c(w, v), nxt)
		stepsCalcs.append(step(ite, shortestPath, nxt))
		ite = len(N)

	# Retornar pasos del algoritmo
	return stepsCalcs