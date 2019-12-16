#ifndef LOADER_H
#define LOADER_H

#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <SFML/Graphics.hpp>
#include "consts.h"
#include "lighting.h"

LightPoint *loadLightPoint(BufferReader *file) {
  int start = (int)file->getPlace() - 5;

  int id = file->getInt();
  int x = file->getInt();
  int y = file->getInt();

  unsigned char r = file->getByte();
  unsigned char g = file->getByte();
  unsigned char b = file->getByte();
  unsigned char a = file->getByte();

  int current = file->getPlace();

  char *bytes = new char[current-start];
  file->setPlace(start);
  file->read(bytes, current-start);

  LightPoint *newLight = new LightPoint(id, sf::Vector2f(x,y), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0), bytes);
  return newLight;
}

LightObject *loadLightObject(BufferReader *file) {
  int start = file->getPlace() - 5;

  std::vector<sf::Vector2f> *tempVec = new std::vector<sf::Vector2f>;

  int id = file->getInt();
  int length = file->getShort();
  int offx = file->getInt();
  int offy = file->getInt();

  for (int i = 0; i < length/2; ++i) {
    int x = file->getInt();
    int y = file->getInt();

    tempVec->push_back(sf::Vector2f(x,y));
  }

  int current = file->getPlace();

  char *bytes = new char[current-start];
  file->setPlace(start);
  file->read(bytes, current-start);

  if (tempVec->size() > 0)
    return new LightObject(id, tempVec, sf::Vector2f(offx, offy*2), bytes);

  delete bytes;
  delete tempVec;
  return NULL;
}

#endif
