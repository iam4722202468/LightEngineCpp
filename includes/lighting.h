#include <vector>
#include <iostream>
#include <math.h>

#include <SFML/Graphics.hpp>

#define SCREEN_X 1000
#define SCREEN_Y 800

bool comparePoints(float angle1, float angle2);
float distance(sf::Vector2f a, sf::Vector2f b);
bool isAngleBetween(float angle, float start, float end);

class LightObject {
  public:
    std::vector<sf::Vector2f> *corners;
    bool directed = false;
    float directedAngle = 0;
    float directedWidth = 0;

    void draw(sf::RenderWindow *window);

    // returns array of triangle points
    void getShadow(sf::Vector2f light, std::vector<sf::Vector2f> *shadowPoints);

    LightObject(std::vector<sf::Vector2f> *corners): corners(corners) {}
};

sf::Vector2f extendLineSegment(sf::Vector2f light, sf::Vector2f *edge, float winxs, float winys, float winxe, float winye);
