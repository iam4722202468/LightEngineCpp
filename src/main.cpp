#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"

#define CHUNK_SIZE_X 100
#define CHUNK_SIZE_Y 100

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
    int id;
    char type;
    unsigned char r,g,b,a;

    while (file.tellg() < size && file.tellg() >= 0) {
      file.read((char*)&type, sizeof(type));

      if (type == 0x00) {
        int start = (int)file.tellg() - 1;

        int x, y;
        std::vector<sf::Vector2f> *tempVec = new std::vector<sf::Vector2f>;

        file.read((char*)&id, sizeof(id));
        file.read((char*)&length, sizeof(length));

        for (int i = 0; i < length/2; ++i) {
          file.read((char*)&x, sizeof(x));
          file.read((char*)&y, sizeof(y));

          tempVec->push_back(sf::Vector2f(x,y));
        }

        int current = file.tellg();
        char *bytes = new char[current-start];

        file.seekg(start, std::ios::beg);
        file.read(bytes, current-start);

        lightObjects->push_back(new LightObject(id, tempVec, bytes));
      }

      if (type == 0x01) {
        int start = (int)file.tellg() - 1;

        int x,y;
        file.read((char*)&id, sizeof(id));
        file.read((char*)&x, sizeof(x));
        file.read((char*)&y, sizeof(y));

        file.read((char*)&r, sizeof(r));
        file.read((char*)&g, sizeof(g));
        file.read((char*)&b, sizeof(b));
        file.read((char*)&a, sizeof(a));

        int current = file.tellg();
        char *bytes = new char[current-start];
        file.seekg(start, std::ios::beg);
        file.read(bytes, current-start);

        lightPoints->push_back(new LightPoint(id, sf::Vector2f(x,y), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0), bytes));
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
    sf::Vector2f offset,
    float scalingFactor
  ) {
  // Don't draw lights that are not visible
  if (light->pos.x + offset.x < 0 - 1000 / scalingFactor
      || light->pos.x + offset.x > SCREEN_X + 1000 / scalingFactor
      || light->pos.y + offset.y < 0 - 1000 / scalingFactor
      || light->pos.y + offset.y > SCREEN_Y + 1000 / scalingFactor)
    return;

  lightShader->setUniform("lightpos", sf::Vector2f(light->pos.x + offset.x, SCREEN_Y-light->pos.y - offset.y));
  lightShader->setUniform("lightcol", light->col);

  std::vector<sf::Vector2f> shadows;

  shadowTexture->clear(sf::Color(255,255,255,255));

  // Get shadows
  for (auto x:*lightObjects)
    x->getShadow(light, &shadows, offset, scalingFactor);

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

    bool shapeOnScreen = false;

    for (unsigned int y = 0; y < x->corners->size(); ++y) {
      objects[y].position = x->corners->at(y) + offset;
      objects[y].color = sf::Color(255,255,255,255);

      // Don't draw shape if offscreen
      if (objects[y].position.x < SCREEN_X
          && objects[y].position.x > 0
          && objects[y].position.y < SCREEN_Y
          && objects[y].position.y > 0)
        shapeOnScreen = true;
    }

    if (shapeOnScreen)
      shadowTexture->draw(objects);
  }

  shadowTexture->display();

  mainTexture->draw(*mainSprite, lightShader);
  mainTexture->display();
}

class MapChunk {
  public:
    int chunkx, chunky;
    int chunkId;
    bool loaded = false;

    std::vector<LightObject*> lightObjects;
    std::vector<LightPoint*> lightPoints;
};

class Map {
  public:
    std::vector<MapChunk> loadedChunks;
    std::vector<MapChunk> requestedChunks;
};

class Server {
  public:
    Server() {
      std::vector<LightObject*> allLightObjects;
      std::vector<LightPoint*> allLightPoints;
      loadMap("data/format", &allLightObjects, &allLightPoints);
    }

    char *getPacket() {}
    void loadChunk(sf::Vector2f start, sf::Vector2f end) {
    }
};

int main() {
  sf::Vector2f offset(100.0,100.0);
  Server server;
  Map gameMap;

  server.loadChunk(sf::Vector2f(-1000, -1000), sf::Vector2f(1000, 1000));

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

  float scalingFactor = 2.2;

  sf::Shader lightShader;
  lightShader.loadFromFile("resources/light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", mainTexture.getTexture());
  lightShader.setUniform("mask", shadowTexture.getTexture());
  lightShader.setUniform("light", lightMask);
  lightShader.setUniform("distanceScale", scalingFactor);

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
      drawLight(&mainTexture, &shadowTexture, &lightShader, &mainSprite, &shadowSprite, lightPoints[light], &lightObjects, offset, scalingFactor);
    }

    window.draw(mainSprite);

    for (auto x: lightObjects) {
      bool shapeOnScreen = false;

      // Don't draw shape if offscreen
      for (unsigned int y = 0; y < x->corners->size(); ++y) {
        if (x->corners->at(y).x + offset.x < SCREEN_X
            && x->corners->at(y).x + offset.x > 0
            && x->corners->at(y).y + offset.y < SCREEN_Y
            && x->corners->at(y).y + offset.y > 0) {
          shapeOnScreen = true;
          break;
        }
      }

      if (shapeOnScreen)
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
