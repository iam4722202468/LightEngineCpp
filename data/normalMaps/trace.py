import cv2
import numpy as np
import PIL
from PIL import Image

import sys

if (len(sys.argv) < 2):
  print "Usage: ./trace.py <input file name>"
  exit(0)

offsetX = 0
offsetY = 0

if (len(sys.argv) >= 3):
  offsetX = int(sys.argv[2])

if (len(sys.argv) >= 4):
  offsetY = int(sys.argv[3])

img = np.asarray(PIL.Image.open(sys.argv[1]))
found = np.array([[[None, None]]*512]*512)

start = None

counter = 0
for iy, y in enumerate(img[:-1]):
  for ix, x in enumerate(img[iy][:-1]):
    if x[3] != 0 and (
      img[iy][ix+1][3] == 0 or
      img[iy][ix-1][3] == 0 or
      img[iy+1][ix][3] == 0 or
      img[iy-1][ix][3] == 0
      ):
        found[iy][ix] = [x, 0]
        if start == None:
            start = [ix, iy]

current = start

def find(x, y, arr):
    choices = [
            [x,y+1], [x,y-1], [x-1,y], [x+1,y], 
            [x+1, y+1], [x+1,y-1], [x-1, y+1], [x-1, y-1],

            [x+2, y+0], [x+2,y-0], [x-2, y+0], [x-2, y-0],
            [x+0, y+2], [x+0,y-2], [x-0, y+2], [x-0, y-2],

            [x+2, y+1], [x+2,y-1], [x-2, y+1], [x-2, y-1],
            [x+1, y+2], [x+1,y-2], [x-1, y+2], [x-1, y-2],

            [x+2, y+2], [x+2,y-2], [x-2, y+2], [x-2, y-2],
            ]
    ret = None

    for choice in choices:
        cur = arr[choice[1]][choice[0]]
        if (cur[1] == 0):
            cur[1] = 1

            if ret == None:
                ret = [choice[0], choice[1]]
                return ret

    return ret

ordered = []

while (current != None):
    ordered.append(current)
    current = find(current[0], current[1], found)

pixels = np.array([[[0,0,0]]*512]*512)

# 723

## calculate lines

def reducePoints(points):
  x = 0

  lastSlope = None
  lastSlopeVert = False
  lines = []

  while x < len(points)-2:
      p1 = points[x]
      p2 = points[x+1]

      s1Vert = False
      s1 = None

      if p1[0] - p2[0] == 0:
          s1Vert = True
      else:
          s1 = (p1[1] - p2[1]) / (p1[0] - p2[0])
          lastSlopeVert = False

      if lastSlopeVert and s1Vert:
          x += 1
          lastSlope = None
          continue

      if s1Vert:
          lastSlopeVert = True

      if lastSlope != None and s1 != None and abs(lastSlope - s1) < 0.1:
          x += 1
          lastSlope = s1
          continue

      lastSlope = s1
      lines.append(points[x])
      x += 1
    
  return lines

lines = reducePoints(reducePoints(ordered));


for pix in lines:
    pixels[pix[1]][pix[0]] = (255,255,255)
    print pix[0] + offsetX, pix[1] + offsetY,
print

print len(lines)

image = Image.fromarray(pixels.astype('uint8'), 'RGB')

image.save('asdf.png')
