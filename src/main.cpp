#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>

#include <SFML/Graphics.hpp>
#include "lighting.h"
#include "client.h"
#include "map.h"

#define CHUNK_SIZE_X 1000
#define CHUNK_SIZE_Y 1000
#define RENDER_DISTANCE 0

class MapChunk {
  public:
    int x, y;
    bool loaded = false;

    std::vector<LightObject*> lightObjects;
    std::vector<LightPoint*> lightPoints;

    MapChunk(int x, int y): x(x), y(y) {}
};

class Map {
  public:
    std::vector<MapChunk*> loadedChunks;

    MapChunk *getChunk(int x, int y) {
      for (auto chunk:loadedChunks) {
        if (chunk->x == x && chunk->y == y)
          return chunk;
      }

      MapChunk *chunk = new MapChunk(x,y);
      loadedChunks.push_back(chunk);

      return chunk;
    }

};

void loadMap(
    std::string mapName,
    Map *map
  ) {
    std::ifstream file(mapName, std::ios::in | std::ios::binary);

    file.seekg(0, std::ios::end);
    int size = file.tellg();
    file.seekg(0, std::ios::beg);

    short length;
    int id;
    int leng;
    unsigned char type;
    unsigned char r,g,b,a;

    while (file.tellg() < size && file.tellg() >= 0) {
      file.read((char*)&leng, sizeof(leng));
      file.read((char*)&type, sizeof(type));

      if (type == 0x00) {
        int start = (int)file.tellg() - 5;

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

        if (tempVec->size() > 0) {
          int loadx = tempVec->at(0).x/CHUNK_SIZE_X;
          int loady = tempVec->at(0).y/CHUNK_SIZE_Y;

          MapChunk *addTo = map->getChunk(loadx, loady);
          LightObject *newLight = new LightObject(id, tempVec, bytes);
          addTo->lightObjects.push_back(newLight);
        }
      }

      if (type == 0x01) {
        int start = (int)file.tellg() - 5;

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

        int loadx = x/CHUNK_SIZE_X;
        int loady = y/CHUNK_SIZE_Y;

        MapChunk *addTo = map->getChunk(loadx, loady);
        LightPoint *newLight = new LightPoint(id, sf::Vector2f(x,y), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0), bytes);
        addTo->lightPoints.push_back(newLight);
      }
    }
  }

class FileGetter {
  public:
    char *data;
    int size;
    int place = 0;
  
    FileGetter(char *data, int size): data(data), size(size) {}

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
};

void loadIntoChunk(
    char *data,
    int size,
    MapChunk *chunk
  ) {
    FileGetter f(data, size);

    short length;
    int id;
    int leng;
    unsigned char type;
    unsigned char r,g,b,a;

    while (f.place < size && f.place >= 0) {
      leng = f.getInt();
      type = f.getByte();

      if (type == 0x00) {
        int x, y;
        std::vector<sf::Vector2f> *tempVec = new std::vector<sf::Vector2f>;

        id = f.getInt();
        length = f.getShort();

        std::cout << "object\n";
        for (int i = 0; i < length/2; ++i) {
          x = f.getInt();
          y = f.getInt();

          tempVec->push_back(sf::Vector2f(x,y));
        }

        LightObject *newLight = new LightObject(id, tempVec, NULL);
        chunk->lightObjects.push_back(newLight);
      }

      if (type == 0x01) {
        int x,y;

        id = f.getInt();
        x = f.getInt();
        y = f.getInt();

        r = f.getByte();
        g = f.getByte();
        b = f.getByte();
        a = f.getByte();

        LightPoint *newLight = new LightPoint(id, sf::Vector2f(x,y), sf::Glsl::Vec4((float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0), NULL);
        chunk->lightPoints.push_back(newLight);
      }
    }
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
            *length += light->bytes[0];
            bytes->push_back(light->bytes);
          }
          for (auto object:chunk->lightObjects) {
            *length += object->bytes[0];
            bytes->push_back(object->bytes);
          }
        }
      }

      char *returnBytes = new char[*length];

      int place = 0;
      for (auto i:*bytes) {
        int length = i[0];
        std::memcpy(&returnBytes[place], i, length);
        place += length;
      }

      delete bytes;
      return returnBytes;
    }
};

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

void drawLight(
    sf::RenderTexture *mainTexture,
    sf::RenderTexture *shadowTexture,
    sf::Shader *lightShader,
    sf::Sprite *mainSprite,
    sf::Sprite *shadowSprite,
    LightPoint *light,
    std::vector<MapChunk*> chunks,
    sf::Vector2f offset,
    float scalingFactor
  ) {
  // Don't draw lights that are not visible
  if (light->pos.x - offset.x < 0 - 1000 / scalingFactor
      || light->pos.x - offset.x > SCREEN_X + 1000 / scalingFactor
      || light->pos.y - offset.y < 0 - 1000 / scalingFactor
      || light->pos.y - offset.y > SCREEN_Y + 1000 / scalingFactor)
    return;

  lightShader->setUniform("lightpos", sf::Vector2f(light->pos.x - offset.x, SCREEN_Y-light->pos.y + offset.y));
  lightShader->setUniform("lightcol", light->col);

  std::vector<sf::Vector2f> shadows;

  shadowTexture->clear(sf::Color(255,255,255,255));

  // Get shadows
  for (auto chunk:chunks)
    for (auto object:chunk->lightObjects)
      object->getShadow(light, &shadows, offset, scalingFactor);

  // Convert shadows to drawable triangles
  sf::VertexArray triangle(sf::Triangles, shadows.size());
  for (unsigned int x = 0; x < shadows.size(); ++x) {
    triangle[x].position = shadows[x] - offset;
    triangle[x].color = sf::Color(100,100,100,255);
  }

  // Draw shadow triangles to shadow texture
  shadowTexture->draw(triangle);

  // Draw back shapes on shadow texture
  for (auto chunk:chunks) {
    for (auto object:chunk->lightObjects) {
      sf::VertexArray objects(sf::TrianglesFan, object->corners->size());
      object->getShadow(light, &shadows, offset, scalingFactor);

      bool shapeOnScreen = false;

      for (unsigned int y = 0; y < object->corners->size(); ++y) {
        objects[y].position = object->corners->at(y) - offset;
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
  }

  shadowTexture->display();

  mainTexture->draw(*mainSprite, lightShader);
  mainTexture->display();
}


int main() {
  sf::Vector2f offset(100.0,100.0);
  ServerConn *server = new ServerConn;
  Map map;

  // Init
  sf::RenderWindow window(sf::VideoMode(SCREEN_X, SCREEN_Y), "GPU Lighting Test",
    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

  //window.setFramerateLimit(60);
  
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
       offset.x -= 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
       offset.x += 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
       offset.y -= 10;
      if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
       offset.y += 10;
      if (event.type == sf::Event::Closed)
        window.close();
    }

    int inChunkx = offset.x < 0 ? (int)(offset.x)/CHUNK_SIZE_X - 1 : (int)(offset.x)/CHUNK_SIZE_X;
    int inChunky = offset.y < 0 ? (int)(offset.y)/CHUNK_SIZE_Y - 1 : (int)(offset.y)/CHUNK_SIZE_Y;

    server->loadChunks(&map, inChunkx, inChunky);

    window.clear();
    mainTexture.clear(sf::Color(0,0,0,255));

    for (auto chunk:map.loadedChunks)
      for (auto light:chunk->lightPoints)
        drawLight(&mainTexture, &shadowTexture, &lightShader, &mainSprite, &shadowSprite, light, map.loadedChunks, offset, scalingFactor);

    window.draw(mainSprite);


    for (auto chunk:map.loadedChunks) {
      for (auto object: chunk->lightObjects) {
        bool shapeOnScreen = false;

        // Don't draw shape if offscreen
        for (unsigned int y = 0; y < object->corners->size(); ++y) {
          if (object->corners->at(y).x - offset.x < SCREEN_X
              && object->corners->at(y).x - offset.x > 0
              && object->corners->at(y).y - offset.y < SCREEN_Y
              && object->corners->at(y).y - offset.y > 0) {
            shapeOnScreen = true;
            break;
          }
        }

        if (shapeOnScreen)
          object->draw(&window, offset);
      }
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
