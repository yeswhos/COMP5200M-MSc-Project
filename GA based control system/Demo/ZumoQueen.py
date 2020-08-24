import matplotlib
matplotlib.use('Agg')

import matplotlib.pyplot as plt
import numpy as np

a = np.loadtxt("ZumoQueen.txt")
best = []
ave = []
tmp = np.zeros(5)
c = -1
total = 0
for i in range(0, len(a)):
    if((i % 5 == 0) and (i != 0)):
        maximum = max(tmp)
        best.append(maximum)
        for j in range(0, 5):
            total = total + tmp[j]
        average = total / 5
        ave.append(average)
        c = -1
        tmp = np.zeros(5)
        total = 0
    
    c = c + 1
    tmp[c] = a[i]
    if(c == 4):
        c = -1
    
# for i in range(0, len(best)):
#     #print(best[i])
# for i in range(0, len(ave)):
#     #print(ave[i])

fig = matplotlib.pyplot.gcf()

plt.xlabel("epoch")
plt.ylabel("fitness")

l1, = plt.plot(ave, color = 'red', linestyle = '-', label = "average fitness")
l2, = plt.plot(best, color = 'green', linestyle = '-', label = "best fitness")
#l3, = plt.plot(distance, color = 'blue', linestyle = '-', label = "distance")
plt.plot(ave, best, linewidth = 0.5)
plt.legend(loc = 'upper right')
#plt.plot(ave)
plt.title("ZumoQueen", fontsize = 10)
#plt.xlim(0, 610)
fig.set_size_inches(18.5, 10.5, forward = True)
plt.savefig("1000 epoch prey")
