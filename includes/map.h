#ifndef MAP_H
#define MAP_H

#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"

class MapChunk {
  public:
    int x, y;
    bool loaded = false;

    std::vector<LightObject*> lightObjects;
    std::vector<LightPoint*> lightPoints;

    MapChunk(int x, int y): x(x), y(y) {}
};

class Map {
  public:
    std::vector<MapChunk*> loadedChunks;

    MapChunk *getChunk(int x, int y) {
      for (auto chunk:loadedChunks) {
        if (chunk->x == x && chunk->y == y)
          return chunk;
      }

      MapChunk *chunk = new MapChunk(x,y);
      loadedChunks.push_back(chunk);

      return chunk;
    }
};

#endif
