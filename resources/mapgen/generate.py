import struct
import os
from trace import trace

maps = next(os.walk('./maps'))[1]
flatten = lambda l: [item for sublist in l for item in sublist]

lookup = {}

for map in maps:
    print map

    f = open('./bin/' + map + '.bin', 'w')
    shapes = open('./maps/' + map + '/shapes', 'r')
    lights = open('./maps/' + map + '/lights', 'r')

    shapedat = ""
    
    for id,shape in enumerate(shapes.read()[:-1].split('\n')):
        line = shape.split(' ')
        base = './textures/' + line[0] + '/'
        path = base + 'texture.png'

        x = []
        
        if path in lookup:
            x = lookup[path]
            print "found"
        else:
            x = trace(path)
            x = [[point[0] + int(line[1]), point[1] + int(line[2])] for point in x]
            x = flatten(x)
            print "added"
            lookup[path] = x

        if len(x) == 0:
            continue

        data = struct.pack('B', 0) # pack
        data += struct.pack('i', id) # pack
        data += struct.pack('h', len(x))

        ## offset x and y
        data += struct.pack('i', int(line[1]))
        data += struct.pack('i', int(line[2]))

        for y in x:
            data += struct.pack('i', y) # pack

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
