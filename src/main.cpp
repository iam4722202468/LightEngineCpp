#include <vector>
#include <iostream>

#include <math.h>

#include <SFML/Graphics.hpp>
#include "lighting.h"

int main() {
  sf::RenderWindow window(sf::VideoMode(SCREEN_X, SCREEN_Y), "GPU Lighting Test",
    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

  window.setFramerateLimit(60);

  std::vector<std::vector<sf::Vector2f>> lightObjectsRaw({
    {sf::Vector2f(100, 100), sf::Vector2f(200, 50), sf::Vector2f(50, 100)},
    {sf::Vector2f(900, 700), sf::Vector2f(850, 750), sf::Vector2f(950, 700)},
    {sf::Vector2f(500, 100), sf::Vector2f(700, 50), sf::Vector2f(450, 100)},
    {sf::Vector2f(500, 500), sf::Vector2f(500, 600), sf::Vector2f(600, 600), sf::Vector2f(600, 500)}
  });

  sf::RenderTexture mainTexture;
  mainTexture.create(SCREEN_X,SCREEN_Y);
  sf::RenderTexture mainTextureSwap;
  mainTextureSwap.create(SCREEN_X,SCREEN_Y);

  sf::RenderTexture shadowTexture;
  shadowTexture.create(SCREEN_X,SCREEN_Y);

  sf::Sprite mainSprite(mainTexture.getTexture());
  sf::Sprite mainSpriteSwap(mainTextureSwap.getTexture());
  sf::Sprite shadowSprite(shadowTexture.getTexture());

  sf::Shader lightShader;
  lightShader.loadFromFile("resources/light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", mainTexture.getTexture());
  lightShader.setUniform("mask", shadowTexture.getTexture());

  sf::Shader blurShader;
  blurShader.loadFromFile("resources/blur.frag", sf::Shader::Fragment);
  blurShader.setUniform("texture", mainTexture.getTexture());
  blurShader.setUniform("screenX", (float)SCREEN_X);
  blurShader.setUniform("screenY", (float)SCREEN_Y);

  sf::Clock clock;
  int frames = 0;
  int counter = 0;

  std::vector<LightObject*> lightObjects;

  for (unsigned int x = 0; x < lightObjectsRaw.size(); ++x)
    lightObjects.push_back(new LightObject(&lightObjectsRaw[x]));

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

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    std::vector<sf::Vector2f> shadows;

    window.clear();
    mainTexture.clear(sf::Color(0,0,0,255));

    for(unsigned int light = 0; light < lightPoints.size(); ++light) {
      lightShader.setUniform("lightpos", sf::Vector2f(lightPoints[light].x, SCREEN_Y-lightPoints[light].y));
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

    blurShader.setUniform("direction", sf::Vector2f(1.2, 0.0));
    mainTextureSwap.draw(mainSprite, &blurShader);
    blurShader.setUniform("direction", sf::Vector2f(0.0, 1.2));

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
