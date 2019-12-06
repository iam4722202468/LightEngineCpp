#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"

void loadMap(
    std::string mapName,
    std::vector<LightObject*> *lightObjects,
    std::vector<LightPoint*> *lightPoints
  ) {
    std::ifstream file(mapName, std::ios::in | std::ios::binary);

    file.seekg(0, std::ios::end);
    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    short length;
    int i;
    char type;
    unsigned char r,g,b,a;

    while (file.tellg() < size && file.tellg() >= 0) {
      file.read((char*)&type, sizeof(type));

      if (type == 0x00) {
        int x, y;
        std::vector<sf::Vector2f> *tempVec = new std::vector<sf::Vector2f>;

        file.read((char*)&length, sizeof(length));

        for (int i = 0; i < length/2; ++i) {
          file.read((char*)&x, sizeof(x));
          file.read((char*)&y, sizeof(y));

          tempVec->push_back(sf::Vector2f(x,y));
        }

        lightObjects->push_back(new LightObject(tempVec));
      }
      if (type == 0x01) {
        int x,y;
        file.read((char*)&x, sizeof(x));
        file.read((char*)&y, sizeof(y));

        file.read((char*)&r, sizeof(r));
        file.read((char*)&g, sizeof(g));
        file.read((char*)&b, sizeof(b));
        file.read((char*)&a, sizeof(a));

        lightPoints->push_back(new LightPoint(sf::Vector2f(x,y), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0)));
        lightPoints->push_back(new LightPoint(sf::Vector2f(x+500,y+500), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0)));
        lightPoints->push_back(new LightPoint(sf::Vector2f(x+1000,y+1000), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0)));
      }
    }
  }

void drawLight(
    sf::RenderTexture *mainTexture,
    sf::RenderTexture *shadowTexture,
    sf::Shader *lightShader,
    sf::Sprite *mainSprite,
    sf::Sprite *shadowSprite,
    LightPoint *light,
    std::vector<LightObject*> *lightObjects,
    sf::Vector2f offset
  ) {
  lightShader->setUniform("lightpos", sf::Vector2f(light->pos.x + offset.x, SCREEN_Y-light->pos.y - offset.y));
  lightShader->setUniform("lightcol", light->col);

  std::vector<sf::Vector2f> shadows;

  shadowTexture->clear(sf::Color(255,255,255,255));

  // Get shadows
  for (auto x:*lightObjects)
    x->getShadow(light, &shadows, offset);

  // Convert shadows to drawable triangles
  sf::VertexArray triangle(sf::Triangles, shadows.size());
  for (unsigned int x = 0; x < shadows.size(); ++x) {
    triangle[x].position = shadows[x] + offset;
    triangle[x].color = sf::Color(100,100,100,255);
  }

  // Draw shadow triangles to shadow texture
  shadowTexture->draw(triangle);

  // Draw back shapes on shadow texture
  for (auto x:*lightObjects) {
    sf::VertexArray objects(sf::TrianglesFan, x->corners->size());
    for (unsigned int y = 0; y < x->corners->size(); ++y) {
      objects[y].position = x->corners->at(y) + offset;
      objects[y].color = sf::Color(255,255,255,255);
    }

    shadowTexture->draw(objects);
  }

  shadowTexture->display();

  mainTexture->draw(*mainSprite, lightShader);
  mainTexture->display();
}


class Server {
  public:
    char *getPacket() {};
};


int main() {
  sf::Vector2f offset(100.0,100.0);

  // Init
  sf::RenderWindow window(sf::VideoMode(SCREEN_X, SCREEN_Y), "GPU Lighting Test",
    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

  //window.setFramerateLimit(60);
  
  std::vector<LightObject*> lightObjects;
  std::vector<LightPoint*> lightPoints;
  loadMap("data/format", &lightObjects, &lightPoints);

  sf::RenderTexture mainTexture;
  mainTexture.create(SCREEN_X,SCREEN_Y);

  sf::RenderTexture shadowTexture;
  shadowTexture.create(SCREEN_X,SCREEN_Y);

  sf::Sprite mainSprite(mainTexture.getTexture());
  sf::Sprite shadowSprite(shadowTexture.getTexture());

  sf::Texture lightMask;
  if (!lightMask.loadFromFile("resources/mask.png"))
  {
    std::cout << "resources/mask.png not found" << std::endl;
    return 1;
  }

  sf::Shader lightShader;
  lightShader.loadFromFile("resources/light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", mainTexture.getTexture());
  lightShader.setUniform("mask", shadowTexture.getTexture());
  lightShader.setUniform("light", lightMask);
  lightShader.setUniform("distanceScale", (float)2.2);

  sf::Clock clock;
  int frames = 0;
  //Init

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
       offset.x += 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
       offset.x -= 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
       offset.y += 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
       offset.y -= 10;
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    mainTexture.clear(sf::Color(0,0,0,255));

    for(unsigned int light = 0; light < lightPoints.size(); ++light) {
      drawLight(&mainTexture, &shadowTexture, &lightShader, &mainSprite, &shadowSprite, lightPoints[light], &lightObjects, offset);
    }

    window.draw(mainSprite);

    for (auto x: lightObjects) {
      x->draw(&window, offset);
    }

    window.display();

    frames++;

    sf::Time time = clock.getElapsedTime();
    if (time.asSeconds() >= 1) {
      std::cout << frames << std::endl;
      frames = 0;
      clock.restart();
    }
  }

  return 0;
}
