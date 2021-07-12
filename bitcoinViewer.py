#!/usr/bin/python3

import svgwrite

f = open('piX.txt')
piXLines = f.readlines()
f.close()

piXLinesLen = len(piXLines)

f = open('dataSortedAndShiftedAndNumbered.txt')
dataLines = f.readlines()
f.close()

dataLinesLen = len(dataLines)

timestampDifficulty = {}

def getBinZeroLeading(currentHash):
    s = bin(int(currentHash, 16))[2:]
    sLen = len(s)
    return 256 - sLen

for dataLinesIndex in range(dataLinesLen):
    dataLine = dataLines[dataLinesIndex]
    dataLineParts = dataLine.split()
    dataLinePartsLen = len(dataLineParts)
    timestampDifficulty[' '.join(dataLineParts[1:3])] = [int(dataLineParts[0]), getBinZeroLeading(dataLineParts[3])]

piX = {}

INFINITY = 10 ** 20
lastTimestamp = 0
deltaTimestampCounter, deltaTimestampNeg = 0, 0
minTimestamp, maxTimestamp, minDeltaTimestamp = INFINITY, 0, INFINITY
minDifficulty, maxDifficulty = INFINITY, 0
for piXLinesIndex in range(piXLinesLen):
    piXLine = piXLines[piXLinesIndex]
    if piXLine[-1] == "\n":
        piXLine = piXLine[:-1]
    if not piXLine in timestampDifficulty:
        print("piXLine not found in timestampDifficulty (" + piXLine + ")")
        continue
    timestampDifficultyBlock = timestampDifficulty[piXLine]
    timestamp = timestampDifficultyBlock[0]
    if timestamp < minTimestamp:
        minTimestamp = timestamp
    if timestamp > maxTimestamp:
        maxTimestamp = timestamp
    if lastTimestamp != 0:
        deltaTimestamp = timestamp - lastTimestamp
        deltaTimestampCounter += 1
        if deltaTimestamp <= 0:
            deltaTimestampNeg += 1
            #print("piX not sorted by time (" + str(deltaTimestamp) + ") !") # how is it possible ?!
        elif deltaTimestamp < minDeltaTimestamp:
            minDeltaTimestamp = deltaTimestamp
    lastTimestamp = timestamp
    difficulty = timestampDifficultyBlock[1]
    if difficulty < minDifficulty:
        minDifficulty = difficulty
    if difficulty > maxDifficulty:
        maxDifficulty = difficulty
    piX[timestamp] = difficulty

print(deltaTimestampNeg, deltaTimestampCounter)
print(minTimestamp, maxTimestamp, minDeltaTimestamp) # 13/2/2010 à 4:46:23, 11/6/2021 à 15:44:01, 4 - donc faire quelque chose à l'échelle chronologie semble pas être super 
print(minDifficulty, maxDifficulty) # 32, 86 exact !

#exit(0)

numberOfRects = 3969
rectSize = 53

width = str(numberOfRects * rectSize)
height = str(13 * (maxDifficulty - minDifficulty + 1))

print(width, height)
heightInt = int(height)

svg_document = svgwrite.Drawing(filename = "piX.svg", size = (width + "px", height + "px"))

lastTimestamp = 0

piXSorted = {k: piX[k] for k in sorted(piX)}

piXIndex = 0
for timestamp in piXSorted: # no problem with the sorted one
    if timestamp <= lastTimestamp:
        print("blocks not ordered by time !")
    lastTimestamp = timestamp
    realDifficulty = piXSorted[timestamp] - minDifficulty
    x = 2 * piXIndex * rectSize
    svg_document.add(svg_document.rect(insert = (x, heightInt - 13 * (realDifficulty + 1)),
                                    size = (str(rectSize) + "px", "13px"),
                                    stroke_width = "1",
                                    stroke = "black",
                                    fill = "rgb(255,255,0)"))

    # 0 to 54
    svg_document.add(svg_document.text(str(realDifficulty), insert = (x + 1, heightInt - 13 * (realDifficulty) - 1)))
    piXIndex += 1

svg_document.save()
