import svgwrite, os

os.chdir("C:\\Users\\Benjamin\\Desktop\\BensFolder\\School\\ENS\\Saclay\\Stage\\Emmanuelle Anceaume\\Stage\\important\\Bitcoin viewer\\Stats\\")

fileName = "bin" # bin

f = open(fileName + '.txt')
lines = f.readlines()
f.close()
linesLen = len(lines)

maxAmount = 0
for linesIndex in range(linesLen):
    line = lines[linesIndex]
    if line[-1] == "\n":
        line = line[:-1]
    lineParts = line.split()
    hashLevel, amount = [int(linePart) for linePart in lineParts]
    if amount > maxAmount:
        maxAmount = amount

width = 1000

svg_document = svgwrite.Drawing(filename = fileName + "HashesStats.svg", size = (str(width) + "px", str(linesLen * 15) + "px"))

def space(n):
    if n < 1000:
        return str(n)
    if n < 10000:
        return str(n)[0] + " " + str(n)[1:]
    if n < 100000:
        return str(n)[:2] + " " + str(n)[2:]
    if n < 1000000:
        return str(n)[:3] + " " + str(n)[3:]
    #return str(n)[:3] + " " + str(n)[3:]

for linesIndex in range(linesLen):
    line = lines[linesIndex]
    if line[-1] == "\n":
        line = line[:-1]
    lineParts = line.split()
    hashLevel, amount = [int(linePart) for linePart in lineParts]
    rectSize = width * (amount / maxAmount)
    svg_document.add(svg_document.rect(insert = (0, linesIndex * 15), size = (str(rectSize) + "px", "15px"), stroke_width = "1", stroke = "black", fill = "rgb(255,255,0)"))

    svg_document.add(svg_document.text(str(hashLevel) + " (" + space(amount) + ")", insert = (1, (linesIndex + 1) * 15 - 2)))

svg_document.save()