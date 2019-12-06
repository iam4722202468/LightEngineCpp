import struct

f = open('data/format', 'w')
shapes = open('data/map1.shapes', 'r')
lights = open('data/map1.lights', 'r')

for x in shapes.read().split('\n'):
  data = struct.pack('b', 0) # pack
  f.write(data)
  data = struct.pack('h', len(x.split(' ')))
  f.write(data)

  for y in x.split(' '):
    data = struct.pack('i', int(y)) # pack
    f.write(data)

for x in lights.read().split('\n'):
  data = struct.pack('b', 1) # pack
  f.write(data)

  for i,y in enumerate(x.split(' ')):
    if i <= 1:
      data = struct.pack('i', int(y)) # pack
      f.write(data)
    else:
      data = struct.pack('B', int(y)) # pack
      f.write(data)

f.close()
