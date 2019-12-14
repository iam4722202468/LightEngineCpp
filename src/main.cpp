#include <vector>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstring>
#include <omp.h>

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

      if (object->renderMethod == 1) {
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
    }

  // Convert shadows to drawable triangles
  sf::VertexArray triangle(sf::Triangles, shadows.size());
  for (unsigned int x = 0; x < shadows.size(); ++x) {
    triangle[x].position = shadows[x] - offset;
    triangle[x].color = sf::Color(100,100,100,255);
  }

  // Draw shadow triangles to shadow texture
  shadowTexture->draw(triangle);
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

  sf::RenderTexture objectTexture;
  objectTexture.create(SCREEN_X,SCREEN_Y);

  sf::RenderTexture objectNormalTexture;
  objectNormalTexture.create(SCREEN_X,SCREEN_Y);

  sf::RenderTexture shadowTexture;
  shadowTexture.create(SCREEN_X,SCREEN_Y);

  sf::Sprite mainSprite(mainTexture.getTexture());
  sf::Sprite objectSprite(objectTexture.getTexture());
  sf::Sprite objectNormalSprite(objectNormalTexture.getTexture());
  sf::Sprite shadowSprite(shadowTexture.getTexture());

  sf::Texture lightMask;
  if (!lightMask.loadFromFile("resources/mask.png"))
  {
    std::cout << "resources/mask.png not found" << std::endl;
    return 1;
  }

  float scalingFactor = 1.8;

  sf::Shader lightShader;
  lightShader.loadFromFile("resources/light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", mainTexture.getTexture());
  lightShader.setUniform("mask", shadowTexture.getTexture());
  lightShader.setUniform("light", lightMask);
  lightShader.setUniform("distanceScale", scalingFactor);

  lightShader.setUniform("normals", objectNormalTexture.getTexture());
  lightShader.setUniform("sprites", objectTexture.getTexture());

  sf::Shader normalShader;
  normalShader.loadFromFile("resources/normal.frag", sf::Shader::Fragment);

  sf::Texture treeTexture;
  if (!treeTexture.loadFromFile("data/normalMaps/tree.png"))
  {
    std::cout << "data/normalMaps/tree.png not found" << std::endl;
    return 1;
  }

  sf::Texture treeTextureNormal;
  if (!treeTextureNormal.loadFromFile("data/normalMaps/tree_normal.png"))
  {
    std::cout << "data/normalMaps/tree_normal.png not found" << std::endl;
    return 1;
  }

  sf::Clock clock;
  int frames = 0;
  float counter = 0;
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
    objectTexture.clear(sf::Color(0,0,0,0));
    objectNormalTexture.clear(sf::Color(0,0,0,0));

    counter += 0.01;

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
          object->draw(&objectTexture, &objectNormalTexture, offset, &treeTexture, &treeTextureNormal, &normalShader);
      }

      //if (chunk->lightPoints.size() > 0) {
      //  chunk->lightPoints.at(1)->pos.x = sin(counter)*200+300;
      //  chunk->lightPoints.at(1)->pos.y = cos(counter)*200+300;
      //}

      for (auto light:chunk->lightPoints) {
        drawLight(&mainTexture, &shadowTexture, &lightShader, &mainSprite, &shadowSprite, light, map.loadedChunks, offset, scalingFactor);
      }
    }

    window.draw(mainSprite);
    objectTexture.display();
    objectNormalTexture.display();
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
