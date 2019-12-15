import cv2
import numpy as np
import PIL
from PIL import Image

def find(x, y, arr):
  choices = [
    [x, y + 1],
    [x, y - 1],
    [x - 1, y],
    [x + 1, y],

    [x + 1, y + 1],
    [x + 1, y - 1],
    [x - 1, y + 1],
    [x - 1, y - 1],

    [x + 2, y + 0],
    [x + 2, y - 0],
    [x - 2, y + 0],
    [x - 2, y - 0],
    [x + 0, y + 2],
    [x + 0, y - 2],
    [x - 0, y + 2],
    [x - 0, y - 2],

    [x + 2, y + 1],
    [x + 2, y - 1],
    [x - 2, y + 1],
    [x - 2, y - 1],
    [x + 1, y + 2],
    [x + 1, y - 2],
    [x - 1, y + 2],
    [x - 1, y - 2],

    [x + 2, y + 2],
    [x + 2, y - 2],
    [x - 2, y + 2],
    [x - 2, y - 2],
  ]

  ret = None

  for choice in choices:
    if choice[1] >= len(arr):
        continue

    if choice[0] >= len(arr[0]):
        continue

    cur = arr[choice[1]][choice[0]]
    if (cur[1] == 0):
      cur[1] = 1

      if ret == None:
        ret = [choice[0], choice[1]]
        return ret

  return ret

def reducePoints(points):
  x = 0

  lastSlope = None
  lastSlopeVert = False
  lines = []

  while x < len(points) - 2:
    p1 = points[x]
    p2 = points[x + 1]

    s1Vert = False
    s1 = None

    if p1[0] - p2[0] == 0:
      s1Vert = True
    else :
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

def trace(offsetX, offsetY, path):
  offsetX = int(offsetX)
  offsetY = int(offsetY)

  img = np.asarray(PIL.Image.open(path))
  found = np.array([[[None, None]] * len(img[0])] * len(img))

  start = None

  counter = 0

  for iy, y in enumerate(img[: -1]):
    for ix, x in enumerate(img[iy][: -1]):
      if x[3] != 0 and(
        img[iy][ix + 1][3] == 0 or
        img[iy][ix - 1][3] == 0 or
        img[iy + 1][ix][3] == 0 or
        img[iy - 1][ix][3] == 0
      ):
        found[iy][ix] = [x, 0]
        if start == None:
          start = [ix, iy]

  current = start
  ordered = []

  while (current != None):
    ordered.append(current)
    current = find(current[0], current[1], found)

  pixels = np.array([[[0, 0, 0]] * len(img[0])] * len(img))
  lines = reducePoints(ordered);

  for pix in lines:
    pixels[pix[1]][pix[0]] = (255, 255, 255)
    print pix[0] + offsetX, pix[1] + offsetY,
  
  print
  print len(lines)

  image = Image.fromarray(pixels.astype('uint8'), 'RGB')
  image.save('/'.join(path.split('/')[:-1]) + '/outline.png')
