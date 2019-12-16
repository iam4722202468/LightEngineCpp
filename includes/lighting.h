#ifndef LIGHTING_H
#define LIGHTING_H

#include <vector>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>

class Map;

#define SCREEN_X 1000
#define SCREEN_Y 800

#define MAP_X 10000
#define MAP_Y 8000

bool comparePoints(float angle1, float angle2);
float distance(sf::Vector2f a, sf::Vector2f b);
bool isAngleBetween(float angle, float start, float end);

class LightPoint {
  public:
    int id;
    sf::Vector2f pos;
    sf::Glsl::Vec4 col;

    void *update;
    char *bytes; // src for generated objects

    LightPoint(int id, sf::Vector2f pos, sf::Glsl::Vec4 col, char *bytes, void *update = NULL):
      pos(pos), col(col), update(update), bytes(bytes) {}
};

class LightObject {
  public:
    int id;
    std::vector<sf::Vector2f> *corners;
    char *bytes; // src for generated objects
    int renderMethod = 2;

    int spriteOffsetX, spriteOffsetY;

    // Store largest and small x/y positions
    sf::Vector2f posPoint;
    sf::Vector2f negPoint;

    void draw(
        sf::RenderTexture *spriteRenderTexture,
        sf::RenderTexture *normalRenderTexture,
        sf::Vector2f offset,
        sf::Texture *texture,
        sf::Texture *normalTexture,
        sf::Shader *normalShader
      );

    // returns array of triangle points
    void getShadow(LightPoint *light, std::vector<sf::Vector2f> *shadowPoints, sf::Vector2f offset, float scalingFactor);
    LightObject(int id, std::vector<sf::Vector2f> *corners, int spriteOffsetX, int spriteOffsetY, char *bytes): id(id), corners(corners), bytes(bytes) {
      bool foundNegx = false;
      bool foundPosx = false;
      bool foundNegy = false;
      bool foundPosy = false;

      for (auto corner:*corners) {
        if (!foundNegx || corner.x < negPoint.x) {
          negPoint.x = corner.x;
          foundNegx = true;
        }

        if (!foundNegy || corner.y < negPoint.y) {
          negPoint.y = corner.y;
          foundNegy = true;
        }

        if (!foundPosx || corner.x > posPoint.x) {
          posPoint.x = corner.x;
          foundPosx = true;
        }

        if (!foundPosy || corner.y > posPoint.y) {
          posPoint.y = corner.y;
          foundPosy = true;
        }
      }
    
    }
};

sf::Vector2f extendLineSegment(LightPoint *light, sf::Vector2f *edge, float winxs, float winys, float winxe, float winye);

#endif
