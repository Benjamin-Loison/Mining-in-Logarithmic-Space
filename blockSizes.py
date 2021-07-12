#!/usr/bin/python3

f = open('blockSizes.txt')
lines = f.readlines()
f.close()

blockSizes = []

linesLen = len(lines)
for linesIndex in range(linesLen):
    line = lines[linesIndex]
    blockSize = int(line)
    blockSizes += [blockSize]

print(sum(blockSizes) / linesLen)
blockSizes.sort()

f = open('blockSizesSorted.txt', 'w')

for linesIndex in range(linesLen):
    blockSize = blockSizes[linesIndex]
    f.write(str(blockSize))
    if linesIndex < linesLen - 1:
        f.write("\n")

f.close()
