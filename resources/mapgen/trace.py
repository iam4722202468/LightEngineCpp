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

def norm(vector):
    dist = (vector[0]**2 + vector[1]**2)**0.5
    return [vector[0]/dist, vector[1]/dist]

def dot(v1, v2):
    return v1[0]*v2[0] + v1[1]*v2[1]

def reducePoints(points):
  x = 0

  lastVec = None
  lines = []

  while x < len(points) - 1:
    p1 = points[x]
    p2 = points[(x + 1) % (len(points)-1)]
    p3 = points[(x + 2) % (len(points)-1)]
    p4 = points[(x + 3) % (len(points)-1)]

    s1Vec = norm([p1[0] - p2[0], p1[1] - p2[1]])
    s2Vec = norm([p1[0] - p3[0], p1[1] - p3[1]])
    s3Vec = norm([p1[0] - p4[0], p1[1] - p4[1]])

    if (lastVec != None):
      ## Dot product will show us the difference in lines
      ## If lines are too different, we want to add another point between them
      s1Dot = 1-dot(lastVec, s1Vec)
      s2Dot = 1-dot(lastVec, s2Vec)
      s3Dot = 1-dot(lastVec, s3Vec)

      ## If no consistent same slope line is detected, add a point
      if s2Dot != 0.0 and s2Dot >= 0.25 and s3Dot >= 0.25:
        lines.append(points[x])

    lastVec = s1Vec
    x += 1

  return lines

def trace(path):
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
  found[start[1]][start[0]] = 1

  ordered = []

  while (current != None):
    ordered.append(current)
    current = find(current[0], current[1], found)

  pixels = np.array([[[0, 0, 0]] * len(img[0])] * len(img))
  lines = reducePoints(ordered);

  for pix in lines:
    pixels[pix[1]][pix[0]] = (255, 255, 255)

  image = Image.fromarray(pixels.astype('uint8'), 'RGB')
  image.save('/'.join(path.split('/')[:-1]) + '/outline.png')

  return lines
