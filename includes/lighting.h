#include <vector>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>

#define SCREEN_X 1000
#define SCREEN_Y 800

#define MAP_X 10000
#define MAP_Y 8000

bool comparePoints(float angle1, float angle2);
float distance(sf::Vector2f a, sf::Vector2f b);
bool isAngleBetween(float angle, float start, float end);

class LightPoint {
  public:
    sf::Vector2f pos;
    sf::Glsl::Vec4 col;

    void *update;

    LightPoint(sf::Vector2f pos, sf::Glsl::Vec4 col, void *update = NULL):
      pos(pos), col(col), update(update) {}
};

class LightObject {
  public:
    std::vector<sf::Vector2f> *corners;
    bool directed = false;
    float directedAngle = 0;
    float directedWidth = 0;

    void draw(sf::RenderWindow *window, sf::Vector2f offset);

    // returns array of triangle points
    void getShadow(LightPoint *light, std::vector<sf::Vector2f> *shadowPoints, sf::Vector2f offset);

    LightObject(std::vector<sf::Vector2f> *corners): corners(corners) {}
};

sf::Vector2f extendLineSegment(LightPoint *light, sf::Vector2f *edge, float winxs, float winys, float winxe, float winye);
