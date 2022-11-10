import copy

def create_intereference_graph(values, time, uses, defs, succs):
	In = [set() for i in range(time)]
	Out = [set() for i in range(time)]

	loopcount = 0
	updated = True
	while updated:
		updated = False
		for i in range(time - 1, -1, -1):
			# update out
			for s in succs[i]:
				for e in In[s]:
					if e not in values or e in Out[i]:
						continue
					Out[i].add(e)
					updated = True
			# update in
			for e in uses[i] | (Out[i] - defs[i]):
				if e not in values or e in In[i]:
					continue
				In[i].add(e)
				updated = True

		loopcount += 1
		#print(loopcount, "回目のループ")
		#for i in range(time - 1, -1, -1):
		#	print(i + 1, Out[i], In[i])

	#print("生存区間")
	#for i in range(time):
	#	print(i, *sorted(In[i] | Out[i]))

	graph = {i:{j:False for j in values} for i in values}
	for i in range(time):
		alive = list((In[i] | Out[i]) - defs[i])
		for i in range(len(alive)):
			for j in range(i):
				graph[alive[i]][alive[j]] = True
				graph[alive[j]][alive[i]] = True

	return graph

def get_degrees(graph):
	ng = dict()
	for (k,v) in graph.items():
		ng[k] = sum(v.values())
	return ng

# 不可能ならNone
# 可能なら結果
def WelshPowell(colNum, values, graph):
	degrees = get_degrees(graph)
	degrees = list(sorted(degrees.items(),reverse=True,key=lambda x:x[1]))

	result = {v:None for v in values}
	for i in range(len(degrees)):
		k = degrees[i][0]

		colors = [False] * colNum
		for (t, is_neighbor) in graph[k].items():
			if is_neighbor and result[t] != None:
				colors[result[t]] = True

		for col in range(colNum):
			if colors[col]:
				continue
			result[k] = col
			break
		else:
			return None
	return result

N = 7
R = 3
c_use = [set() for i in range(N)]
c_def = [set() for i in range(N)]
c_succ = [set() for i in range(N)]

c_values = ["a", "b", "c", "d", "e"]
c_use[6] = {"c"}
c_use[5] = {"e"}
c_use[4] = {"a","d"}
c_use[3] = {"a","b"}
c_use[2] = {"b","c"}

c_def[4] = {"e"}
c_def[3] = {"d"}
c_def[2] = {"c"}
c_def[1] = {"b"}
c_def[0] = {"a"}

for i in range(N-1):
	c_succ[i].add(i+1)
c_succ[5].add(1)

graph = create_intereference_graph(c_values, N, c_use, c_def, c_succ);
degs = get_degrees(graph)
degs = list(sorted(degs.items(),reverse=True,key=lambda x:x[1]))

print(*graph.items(), sep="\n")
print(degs)

new_values = copy.copy(c_values)
for i in range(len(c_values)):
	graph = create_intereference_graph(new_values, N, c_use, c_def, c_succ);
	result = WelshPowell(R, new_values, graph)
	if result is not None:
		print("レジスタ数: ", R)
		print("割り当て  : ", result)
		print("spill     : ", *(set(c_values) - set(new_values)))
		exit()
	new_values.remove(degs[i][0])
