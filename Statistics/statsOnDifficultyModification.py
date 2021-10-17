import os, matplotlib.pyplot as plt

os.chdir('C:\\Users\\Benjamin\\Desktop\\BensFolder\\School\\ENS\\Saclay\\Stage\\Emmanuelle Anceaume\\Stage\\important\\difficultyChanges\\')

f = open('btc.com_diff_2021-06-14_14 00 58.csv') # https://btc.com/stats/diff/export
lines = f.readlines()
f.close()

l = []
difficultyChanges = {}
difficulties = {}

X, Y = [], []
X0, Y0 = [], []

maxDifficultyChange, maxDifficultyIndex = 0, 0
#difficulty = 1

linesLen = len(lines)
for linesIndex in range(1, linesLen):
    line = lines[linesIndex]
    lineParts = line.split(',')
    linePartsLen = len(lineParts)
    if not linePartsLen in l:
        l += [linePartsLen]
    difficulty = float(lineParts[2])
    difficultyChange = float(lineParts[3])
    #print(difficultyChange)
    X += [linesIndex]
    Y += [difficultyChange]
    X0 += [linesIndex]
    Y0 += [0]
    difficultyChangeAbs = abs(difficultyChange)
    #print(difficultyChangeAbs)
    if difficultyChangeAbs > maxDifficultyChange:
        maxDifficultyIndex = linesIndex
        maxDifficultyChange = difficultyChangeAbs
    if difficultyChangeAbs in difficultyChanges:
        difficultyChanges[difficultyChangeAbs] += 1
    else:
        difficultyChanges[difficultyChangeAbs] = 1
    if difficulty in difficulties:
        difficulties[difficulty] += 1
    else:
        difficulties[difficulty] = 1

difficultyChanges = dict(sorted(difficultyChanges.items()))
difficulties = dict(sorted(difficulties.items()))

#for difficultyChange in difficultyChanges:
#    print(difficultyChange, difficultyChanges[difficultyChange])

for difficulty in difficulties:
    print(difficulty, difficulties[difficulty])

plt.plot(X, Y)
plt.plot(X0, Y0)
plt.show()

# 25 046 487 590 083 harder than initially

#print(l)
#print(maxDifficultyChange, maxDifficultyIndex)

