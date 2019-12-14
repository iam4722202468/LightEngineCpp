#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <SFML/Graphics.hpp>
#include "consts.h"
#include "loader.h"
#include "map.h"

// Shouldn't need this in final version. Replace calls in ServerConn with ssl requests
#include "server.h"

class ArrayBufferReader : public BufferReader {
  char *data;
  int size;
  int place = 0;

  public:
    ArrayBufferReader(char *data, int size): data(data), size(size) {}

    void setPlace(int newPlace) { place = newPlace; }
    int getPlace() { return place; }
    int getSize() { return size; }

    int getInt() {
      int toReturn = 0;
      toReturn = (unsigned char)data[place + 0]<<0 | (unsigned char)data[place + 1]<<8 | (unsigned char)data[place + 2]<<16 | (unsigned char)data[place + 3]<<24;
      place += 4;

      return toReturn;
    }

    short getShort() {
      short toReturn = (unsigned char)data[place + 0] << 0 | (unsigned char)data[place + 1] << 8;
      place += 2;
      return toReturn;
    }

    unsigned char getByte() {
      unsigned char toReturn = (unsigned char)data[place + 0] << 0;
      place += 1;
      return toReturn;
    }

    void read(char *buffer, int bytes) {
      std::memcpy(buffer, &data[place], bytes);
      place += bytes;
    }
};

void loadIntoChunk(
    char *data,
    int size,
    MapChunk *chunk
  ) {
    ArrayBufferReader file(data, size);

    while (file.getPlace() < size && file.getPlace() >= 0) {
      int leng = file.getInt();
      unsigned char type = file.getByte();

      if (type == 0x00) {
        LightObject *newObject = loadLightObject(&file);

        if (newObject != NULL)
          chunk->lightObjects.push_back(newObject);
      }

      if (type == 0x01) {
        LightPoint *newLight = loadLightPoint(&file);
        chunk->lightPoints.push_back(newLight);
      }
    }
  }

class ServerConn {
  public:
    int port;
    std::string address;

    // Temp Server
    Server server;

    void loadChunks(Map* map, int x, int y) {
      // We need a way to figure out which chunks are super old and can be removed
      for (int j = x - RENDER_DISTANCE; j <= x + RENDER_DISTANCE; ++j) {
        for (int i = y - RENDER_DISTANCE; i <= y + RENDER_DISTANCE; ++i) {
          // Don't need to reload if we already have it
          if (map->getChunk(j, i)->loaded)
            continue;

          int length = 0;
          char *data = server.getChunk(j,i, &length);

          if (length > 0)
            loadIntoChunk(data, length, map->getChunk(j, i));

          delete data;

          map->getChunk(j, i)->loaded = true;
        }
      }
    }
};

#endif
