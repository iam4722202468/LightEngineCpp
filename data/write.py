import struct

f = open('data/format', 'w')
shapes = open('data/map1.shapes', 'r')
lights = open('data/map1.lights', 'r')

for id,x in enumerate(shapes.read().split('\n')):
  data = struct.pack('B', 0) # pack
  data += struct.pack('i', id) # pack
  data += struct.pack('h', len(x.split(' ')))

  for y in x.split(' '):
    data += struct.pack('i', int(y)) # pack

  data = struct.pack('i', len(data)+4) + data
  f.write(data)

for id,x in enumerate(lights.read().split('\n')):
  data = struct.pack('B', 1) # pack
  data += struct.pack('i', id) # pack

  for i,y in enumerate(x.split(' ')):
    if i <= 1:
      data += struct.pack('i', int(y)) # pack
    else:
      data += struct.pack('B', int(y)) # pack

  data = struct.pack('i', len(data)+4) + data
  f.write(data)

f.close()
