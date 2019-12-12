#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>

#include <SFML/Graphics.hpp>
#include "client.h"

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
    for (auto object:chunk->lightObjects) {
      bool shapeOnScreen = false;

      for (auto corner:*(object->corners)) {
        // Don't draw shape if offscreen
        if (corner.x < SCREEN_X + 1000 / scalingFactor
            && corner.x > 0 - 1000 / scalingFactor
            && corner.y < SCREEN_Y + 1000 / scalingFactor
            && corner.y > 0 - 1000 / scalingFactor)
          shapeOnScreen = true;
      }

      if (shapeOnScreen)
        object->getShadow(light, &shadows, offset, scalingFactor);
    }

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
        // Add 1000 so we still draw if we are within 1000 pixels from it
        if (objects[y].position.x < SCREEN_X + 1000
            && objects[y].position.x > 0 - 1000
            && objects[y].position.y < SCREEN_Y + 1000
            && objects[y].position.y > 0 - 1000)
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
          if (object->corners->at(y).x - offset.x < SCREEN_X + 1000
              && object->corners->at(y).x - offset.x > 0 - 1000
              && object->corners->at(y).y - offset.y < SCREEN_Y + 1000
              && object->corners->at(y).y - offset.y > 0 - 1000) {
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
