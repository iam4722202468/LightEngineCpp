import struct
import os
from trace import trace

maps = next(os.walk('./maps'))[1]

for map in maps:
    print map

    f = open('./bin/' + map + '.bin', 'w')
    shapes = open('./maps/' + map + '/shapes', 'r')
    lights = open('./maps/' + map + '/lights', 'r')

    shapedat = ""
    
    for shape in shapes.read()[:-1].split('\n'):
        line = shape.split(' ')
        base = './textures/' + line[0] + '/'
        
        trace(line[1], line[2], base + 'texture.png')

    for id,x in enumerate(shapedat.split('\n')):
        if len(x) == 0:
            continue

        data = struct.pack('B', 0) # pack
        data += struct.pack('i', id) # pack
        data += struct.pack('h', len(x.split(' ')))

        for y in x.split(' '):
            data += struct.pack('i', int(y)) # pack

        data = struct.pack('i', len(data)+4) + data
        f.write(data)

    for id,x in enumerate(lights.read()[:-1].split('\n')):
        if len(x) == 0:
            continue

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
