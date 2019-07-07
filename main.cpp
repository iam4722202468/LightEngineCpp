#include <vector>
#include <iostream>

#include <math.h>

#include <SFML/Graphics.hpp>

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

bool isAngleBetween(float angle, float start, float end)
{
  // If angle is outside the range [0,360), convert it to the
  // equivalent angle inside that range.
  angle = fmod(angle, PI*2);

  // If start>end, then the angle range wraps around 0.
  return (start<=end) ? (angle>=start && angle<=end)
    : (angle>=start || angle<=end);
}


class LightObject {
  public:
    std::vector<sf::Vector2f> *corners;

    void draw(sf::RenderWindow *window) {
      sf::VertexArray object(sf::TrianglesFan, corners->size());
      for (unsigned int x = 0; x < corners->size(); ++x) {
        object[x].position = corners->at(x);
        object[x].color = sf::Color(150,150,150,255);
      }

      window->draw(object, sf::BlendAdd);
    }

    // returns array of triangle points
    void getShadow(sf::Vector2f light, std::vector<sf::Vector2f> *shadowPoints) {
      float winxs = 0;
      float winxe = 1000;
      float winys = 0;
      float winye = 800;

      sf::Vector2f *inner = NULL;
      sf::Vector2f *outer = NULL;
      float innerAngle = 0;
      float outerAngle = 0;

      // Find inner and outer points
      for (unsigned int x = 0; x < corners->size(); ++x) {
        sf::Vector2f corner = corners->at(x);

        if (inner == NULL) {
          inner = &corners->at(x);
          innerAngle = atan2(corner.y - light.y, corner.x - light.x);
        }

        if (outer == NULL) {
          outer = &corners->at(x);
          outerAngle = atan2(corner.y - light.y, corner.x - light.x);
        }

        float cornerAngle = atan2(corner.y - light.y, corner.x - light.x);

        if (comparePoints(innerAngle, cornerAngle)) {
          innerAngle = cornerAngle;
          inner = &corners->at(x);
        }

        if (comparePoints(cornerAngle, outerAngle)) {
          outerAngle = cornerAngle;
          outer = &corners->at(x);
        }

      }

      sf::Vector2f *edges[2] = {inner, outer};
      sf::Vector2f extended[2];

      // Extend points to edge
      for (int j = 0; j < 2; ++j) {
        for (int i = 0; i < 4; ++i) {
          if (i == 0) {
            extended[j].x = winxs;
            extended[j].y = (winxs-light.x)/(edges[j]->x-light.x)*(edges[j]->y-light.y) + light.y;
          } else if (i == 1) {
            extended[j].x = winxe;
            extended[j].y = (winxe-light.x)/(edges[j]->x-light.x)*(edges[j]->y-light.y) + light.y;
          } else if (i == 2) {
            extended[j].y = winys;
            extended[j].x = (winys-light.y)/(edges[j]->y-light.y)*(edges[j]->x-light.x) + light.x;
          } else if (i == 3) {
            extended[j].y = winye;
            extended[j].x = (winye-light.y)/(edges[j]->y-light.y)*(edges[j]->x-light.x) + light.x;
          }

          // Solution will only be correct if object is between wall and light source
          //
          // p1 -> extended[j]
          // p2 -> edges[j]
          // p3 -> light
          //
          float distance1 = distance(extended[j], *edges[j]);
          float distance2 = distance(extended[j], light);
          float distance3 = distance(light, *edges[j]);

          if (fabs(distance2 - (distance1 + distance3)) < 0.01) {
            if (i >= 2 && extended[j].x >= winxs && extended[j].x <= winxe)
              break;

            if (i <= 1 && extended[j].y >= winys && extended[j].y <= winye)
              break;
          }
        }
      }

      // Check if corner is between points
      // xor here because point is between when one returns true and one returns false
      // edges[0].angle - winCorners[i].angle

      sf::Vector2f winCorners[4];
      winCorners[0] = {winxs, winys};
      winCorners[1] = {winxs,winye};
      winCorners[2] = {winxe,winys};
      winCorners[3] = {winxe,winye};

      std::vector<sf::Vector2f> cornersCaught;
      for (int i = 0; i < 4; ++i) {
        // Make sure light is between correct angles
        float cornerAngle = atan2(winCorners[i].y - light.y, winCorners[i].x - light.x);

        if (isAngleBetween(cornerAngle, outerAngle, innerAngle)) {
          // Check if light is behind corner. Make sure distance to edge of object > distance from light to corner
          if (distance(light, *edges[0]) < distance(light, winCorners[i]))
            cornersCaught.push_back(winCorners[i]);
        }
      }

      if (cornersCaught.size() == 0) {
        shadowPoints->push_back(*edges[0]);
        shadowPoints->push_back(extended[0]);
        shadowPoints->push_back(extended[1]);

        shadowPoints->push_back(*edges[0]);
        shadowPoints->push_back(extended[1]);
        shadowPoints->push_back(*edges[1]);
        return;
      }

      cornersCaught.push_back(extended[1]);
      cornersCaught.insert(cornersCaught.begin(), extended[0]);

      for (unsigned int x = 0; x < cornersCaught.size() - 1; ++x) {
        shadowPoints->push_back(*edges[0]);
        shadowPoints->push_back(cornersCaught[x]);
        shadowPoints->push_back(cornersCaught[x+1]);
      }

      shadowPoints->push_back(*edges[0]);
      shadowPoints->push_back(cornersCaught.back());
      shadowPoints->push_back(*edges[1]);
    }

    LightObject(std::vector<sf::Vector2f> *corners): corners(corners) {}
};


int main() {
  sf::RenderWindow window(sf::VideoMode(1000, 800), "GPU Lighting Test",
    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

  //window.setFramerateLimit(60);

  std::vector<sf::Vector2f> a = {sf::Vector2f(100, 100), sf::Vector2f(200, 50), sf::Vector2f(50, 100)};
  LightObject b(&a);

  std::vector<sf::Vector2f> c = {sf::Vector2f(900, 700), sf::Vector2f(850, 750), sf::Vector2f(950, 700)};
  LightObject d(&c);

  std::vector<sf::Vector2f> e = {sf::Vector2f(500, 100), sf::Vector2f(700, 50), sf::Vector2f(450, 100)};
  LightObject f(&e);

  std::vector<sf::Vector2f> g = {sf::Vector2f(500, 500), sf::Vector2f(500, 600), sf::Vector2f(600, 600), sf::Vector2f(600, 500)};
  LightObject h(&g);

  sf::RenderTexture mainTexture;
  mainTexture.create(1000,800);
  sf::RenderTexture mainTextureSwap;
  mainTextureSwap.create(1000,800);

  sf::RenderTexture shadowTexture;
  shadowTexture.create(1000,800);

  sf::Sprite mainSprite(mainTexture.getTexture());
  sf::Sprite mainSpriteSwap(mainTextureSwap.getTexture());
  sf::Sprite shadowSprite(shadowTexture.getTexture());

  sf::Shader lightShader;
  lightShader.loadFromFile("light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", mainTexture.getTexture());
  lightShader.setUniform("mask", shadowTexture.getTexture());

  sf::Shader blurShader;
  blurShader.loadFromFile("blur.frag", sf::Shader::Fragment);
  blurShader.setUniform("texture", mainTexture.getTexture());

  sf::Clock clock;
  int frames = 0;
  int counter = 0;

  std::vector<LightObject*> lightObjects = {
    &b, &d, &f, &h
  };

  std::vector<sf::Vector2f> lightPoints;
  std::vector<sf::Glsl::Vec4> lightColors;

  lightPoints.push_back(sf::Vector2f(800,800));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,0.0,200.0,200.0)));

  lightPoints.push_back(sf::Vector2f(250,250));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(0.0,200.0,200.0,200.0)));

  lightPoints.push_back(sf::Vector2f(800,100));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,0.0,200.0)));

  lightPoints.push_back(sf::Vector2f(100,400));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,200.0,200.0)));

  lightPoints.push_back(sf::Vector2f(1000,500));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,200.0,200.0)));
  lightPoints.push_back(sf::Vector2f(1000,500));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,200.0,200.0)));
  lightPoints.push_back(sf::Vector2f(1000,500));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,200.0,200.0)));
  lightPoints.push_back(sf::Vector2f(1000,500));
  lightColors.push_back(sf::Glsl::Vec4(sf::Color(200.0,200.0,200.0,200.0)));

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    std::vector<sf::Vector2f> shadows;

    window.clear();
    mainTexture.clear(sf::Color(0,0,0,255));

    lightShader.loadFromFile("light.frag", sf::Shader::Fragment);
    lightShader.setUniform("texture", mainTexture.getTexture());
    lightShader.setUniform("mask", shadowTexture.getTexture());

    for(unsigned int light = 0; light < lightPoints.size(); ++light) {
      lightShader.setUniform("lightpos", sf::Vector2f(lightPoints[light].x, 800-lightPoints[light].y));
      lightShader.setUniform("lightcol", lightColors[light]);

      shadowTexture.clear(sf::Color(255,255,255,255));
      shadows.clear();

      // Get shadows
      for (auto x:lightObjects)
        x->getShadow(lightPoints[light], &shadows);

      // Convert shadows to drawable triangles
      sf::VertexArray triangle(sf::Triangles, shadows.size());
      for (unsigned int x = 0; x < shadows.size(); ++x) {
        triangle[x].position = shadows[x];
        triangle[x].color = sf::Color(140,140,140,255);
      }

      // Draw shadow triangles to shadow texture
      shadowTexture.draw(triangle);

      // Draw back shapes on shadow texture
      for (auto x:lightObjects) {
        sf::VertexArray objects(sf::TrianglesFan, x->corners->size());
        for (unsigned int y = 0; y < x->corners->size(); ++y) {
          objects[y].position = x->corners->at(y);
          objects[y].color = sf::Color(255,255,255,255);
        }

        shadowTexture.draw(objects);
      }

      shadowTexture.display();
      mainTextureSwap.draw(mainSprite, &lightShader);
      mainTextureSwap.display();
      mainTexture.draw(mainSpriteSwap, sf::BlendAlpha);
      mainTexture.display();
    }

    blurShader.setUniform("direction", sf::Vector2f(1.0, 0.0));
    mainTextureSwap.draw(mainSprite, &blurShader);
    blurShader.setUniform("direction", sf::Vector2f(0.0, 1.0));

    window.draw(mainSpriteSwap, &blurShader);

    for (auto x: lightObjects) {
      x->draw(&window);
    }

    window.display();

    frames++;
    counter++;

    sf::Time time = clock.getElapsedTime();
    if (time.asSeconds() >= 1) {
      std::cout << frames << std::endl;
      frames = 0;
      clock.restart();
    }
  }

  return 0;
}
