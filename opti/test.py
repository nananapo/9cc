N = 7
c_use = [set() for i in range(N)]
c_def = [set() for i in range(N)]
succ = [set() for i in range(N)]

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
	succ[i].add(i+1)

succ[5].add(1)


In = [set() for i in range(N)]
Out = [set() for i in range(N)]


loopcount = 0
updated = True
while updated:
	updated = False
	for i in range(N - 1, -1, -1):
		# update out
		for s in succ[i]:
			for e in In[s]:
				if e in Out[i]:
					continue
				Out[i].add(e)
				updated = True
		# update in
		for e in c_use[i] | (Out[i] - c_def[i]):
			if e in In[i]:
				continue
			In[i].add(e)
			updated = True

	loopcount += 1
	print(loopcount, "回目のループ")
	for i in range(N - 1, -1, -1):
		print(i + 1, Out[i], In[i])

print("生存区間")
for i in range(N):
	print(i, *sorted(In[i] | Out[i]))
