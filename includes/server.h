#ifndef SERVER_H
#define SERVER_H

#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"
#include "consts.h"
#include "map.h"
#include "loader.h"

class FileBufferReader : public BufferReader {
  std::ifstream file;
  int size;

  public:
    FileBufferReader(std::string mapName) {
      file = std::ifstream(mapName, std::ios::in | std::ios::binary);
      file.seekg(0, std::ios::end);
      size = file.tellg();
      std::cout << size << std::endl;
      file.seekg(0, std::ios::beg);
    }

    unsigned char getByte() {
      unsigned char data;
      file.read((char*)&data, sizeof(data));
      return data;
    }

    short getShort() {
      short data;
      file.read((char*)&data, sizeof(data));
      return data;
    }

    int getInt() {
      int data;
      file.read((char*)&data, sizeof(data));
      return data;
    }

    int getSize() {
      return size;
    }

    int getPlace() {
      return file.tellg();
    }

    void setPlace(int newPlace) {
      file.seekg(newPlace, std::ios::beg);
    }

    void read(char *buffer, int bytes) {
      file.read(buffer, bytes);
    }
};

void loadMap(
    std::string mapName,
    Map *map
  ) {
    FileBufferReader file(mapName);

    while (file.getPlace() < file.getSize() && file.getPlace() >= 0) {
      int leng = file.getInt();
      unsigned char type = file.getByte();

      if (type == 0x00) {
        LightObject *newObject = loadLightObject(&file);

        if (newObject != NULL) {
          int loadx = newObject->corners->at(0).x/CHUNK_SIZE_X;
          int loady = newObject->corners->at(0).y/CHUNK_SIZE_Y;

          MapChunk *addTo = map->getChunk(loadx, loady);
          addTo->lightObjects.push_back(newObject);
        }
      }

      if (type == 0x01) {
        LightPoint *newLight = loadLightPoint(&file);
        int loadx = newLight->pos.x/CHUNK_SIZE_X;
        int loady = newLight->pos.y/CHUNK_SIZE_Y;

        MapChunk *addTo = map->getChunk(loadx, loady);
        addTo->lightPoints.push_back(newLight);
      }
    }
  }

int getFirstInt(char *i) {
  return (unsigned char)i[0]<<0 | (unsigned char)i[1]<<8 | (unsigned char)i[2]<<16 | (unsigned char)i[3]<<24;
}

class Server {
  public:
    Map map;

    Server() {
      loadMap("data/format", &map);
      std::cout << "Loaded map " << map.loadedChunks.size() << std::endl;;
    }

    char *getChunk(int x, int y, int *length) {
      std::vector<char*> *bytes = new std::vector<char*>;
      *length = 0;

      for (auto chunk:map.loadedChunks) {
        if (chunk->x == x && chunk->y == y) {
          for (auto light:chunk->lightPoints) {
            *length += getFirstInt(light->bytes);
            bytes->push_back(light->bytes);
          }
          for (auto object:chunk->lightObjects) {
            *length += getFirstInt(object->bytes);
            bytes->push_back(object->bytes);
          }
        }
      }

      char *returnBytes = new char[*length];

      int place = 0;
      for (auto i:*bytes) {
        int length = getFirstInt(i);
        std::cout << length << std::endl;
        std::memcpy(&returnBytes[place], i, length);
        place += length;
      }

      delete bytes;
      return returnBytes;
    }
};

#endif
