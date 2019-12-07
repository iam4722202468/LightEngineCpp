#include <vector>
#include <iostream>

#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"

#define PI 3.141592659

bool comparePoints(float angle1, float angle2)
{
  if (angle1 < 0 && angle2 > 0 && fabs(angle1 - angle2) >= PI)
    return (angle1 + 2*PI < angle2);
  if (angle2 < 0 && angle1 > 0 && fabs(angle1 - angle2) >= PI)
    return (angle1 < angle2 + 2*PI);
  return (angle1 < angle2);
}

float distance(sf::Vector2f a, sf::Vector2f b) {
  return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

float distance(LightPoint *a, LightPoint *b) {
  return sqrt(pow(a->pos.x - b->pos.x, 2) + pow(a->pos.y - b->pos.y, 2));
}

float distance(LightPoint *a, sf::Vector2f b) {
  return sqrt(pow(a->pos.x - b.x, 2) + pow(a->pos.y - b.y, 2));
}

// Not needed
float distance(sf::Vector2f a, LightPoint *b) {
  return sqrt(pow(a.x - b->pos.x, 2) + pow(a.y - b->pos.y, 2));
}

bool isAngleBetween(float angle, float start, float end)
{
  // If angle is outside the range [0,360), convert it to the
  // equivalent angle inside that range.
  angle = fmod(angle, PI*2);

  // If start>end, then the angle range wraps around 0.
  return (start<=end) ? (angle>=start && angle<=end)
    : (angle>=start || angle<=end);
}

sf::Vector2f extendLineSegment(LightPoint *light, sf::Vector2f *edge, float winxs, float winys, float winxe, float winye) {
  sf::Vector2f extended;

  for (int i = 0; i < 4; ++i) {
    if (i == 0) {
      extended.x = winxs;
      extended.y = (winxs-light->pos.x)/(edge->x-light->pos.x)*(edge->y-light->pos.y) + light->pos.y;
    } else if (i == 1) {
      extended.x = winxe;
      extended.y = (winxe-light->pos.x)/(edge->x-light->pos.x)*(edge->y-light->pos.y) + light->pos.y;
    } else if (i == 2) {
      extended.y = winys;
      extended.x = (winys-light->pos.y)/(edge->y-light->pos.y)*(edge->x-light->pos.x) + light->pos.x;
    } else if (i == 3) {
      extended.y = winye;
      extended.x = (winye-light->pos.y)/(edge->y-light->pos.y)*(edge->x-light->pos.x) + light->pos.x;
    }

    // Solution will only be correct if object is between wall and light source
    //
    // p1 -> extended[j]
    // p2 -> edges[j]
    // p3 -> light
    //
    float distance1 = distance(extended, *edge);
    float distance2 = distance(extended, light);
    float distance3 = distance(light, *edge);

    if (fabs(distance2 - (distance1 + distance3)) < 0.01) {
      if (i >= 2 && extended.x >= winxs && extended.x <= winxe)
        break;

      if (i <= 1 && extended.y >= winys && extended.y <= winye)
        break;
    }
  }

  return extended;
}

void LightObject::draw(sf::RenderWindow *window, sf::Vector2f offset) {
  sf::VertexArray object(sf::TrianglesFan, corners->size());
  for (unsigned int x = 0; x < corners->size(); ++x) {
    object[x].position = corners->at(x) + offset;
    object[x].color = sf::Color(150,150,150,255);
  }

  window->draw(object, sf::BlendAdd);
}

// returns array of triangle points
void LightObject::getShadow(LightPoint *light, std::vector<sf::Vector2f> *shadowPoints, sf::Vector2f offset, float scalingFactor) {
  float winxs = 0;
  float winxe = MAP_X;
  float winys = 0;
  float winye = MAP_Y;

  sf::Vector2f inner;
  sf::Vector2f outer;
  float innerAngle = 0;
  float outerAngle = 0;
  bool notFoundInner = true;
  bool notFoundOuter = true;

  // Find inner and outer points
  for (unsigned int x = 0; x < corners->size(); ++x) {
    sf::Vector2f corner = corners->at(x);

    float cornerAngle = atan2(corner.y - light->pos.y, corner.x - light->pos.x);

    if (comparePoints(innerAngle, cornerAngle) || notFoundInner) {
      innerAngle = cornerAngle;
      inner = corner;
      notFoundInner = false;
    }

    if (comparePoints(cornerAngle, outerAngle) || notFoundOuter) {
      outerAngle = cornerAngle;
      outer = corner;
      notFoundOuter = false;
    }
  }

  sf::Vector2f edges[2] = {inner, outer};
  sf::Vector2f extended[2];

  // Extend points to edge
  extended[0] = extendLineSegment(light, &edges[0], winxs, winys, winxe, winye);
  extended[1] = extendLineSegment(light, &edges[1], winxs, winys, winxe, winye);

  // Check if corner is between points
  // xor here because point is between when one returns true and one returns false
  // edges[0].angle - winCorners[i].angle

  sf::Vector2f winCorners[4];
  winCorners[0] = {winxs, winys};
  winCorners[1] = {winxs, winye};
  winCorners[2] = {winxe, winys};
  winCorners[3] = {winxe, winye};

  std::vector<sf::Vector2f> cornersCaught;
  for (int i = 0; i < 4; ++i) {
    // Make sure light is between correct angles
    float cornerAngle = atan2(winCorners[i].y - light->pos.y, winCorners[i].x - light->pos.x);

    if (isAngleBetween(cornerAngle, outerAngle, innerAngle)) {
      // Check if light is behind corner. Make sure distance to edge of object > distance from light to corner
      if (distance(light, edges[0]) < distance(light, winCorners[i]))
        cornersCaught.push_back(winCorners[i]);
    }
  }

  if (cornersCaught.size() == 0) {
    shadowPoints->push_back(edges[0]);
    shadowPoints->push_back(extended[0]);
    shadowPoints->push_back(extended[1]);

    shadowPoints->push_back(edges[0]);
    shadowPoints->push_back(extended[1]);
    shadowPoints->push_back(edges[1]);
    return;
  }

  cornersCaught.push_back(extended[1]);
  cornersCaught.insert(cornersCaught.begin(), extended[0]);

  for (unsigned int x = 0; x < cornersCaught.size() - 1; ++x) {
    shadowPoints->push_back(edges[0]);
    shadowPoints->push_back(cornersCaught[x]);
    shadowPoints->push_back(cornersCaught[x+1]);
  }

  shadowPoints->push_back(edges[0]);
  shadowPoints->push_back(cornersCaught.back());
  shadowPoints->push_back(edges[1]);
}
