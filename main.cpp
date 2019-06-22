#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>

sf::RenderTexture front;

int main() {
  sf::RenderWindow window(sf::VideoMode(1000, 800), "GPU Lighting Test",
    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);

  //window.setFramerateLimit(60);
  
  front.create(1000,800);

  sf::RenderTexture mainTexture;
  mainTexture.create(1000,800);

  sf::Sprite mainSprite(mainTexture.getTexture());

  sf::Shader lightShader;
  lightShader.loadFromFile("light.frag", sf::Shader::Fragment);
  lightShader.setUniform("texture", sf::Shader::CurrentTexture);

  sf::Sprite frontSprite(front.getTexture());

  sf::Clock clock;
  int frames = 0;

  while (window.isOpen()) {
    sf::Event event;
    while (window.pollEvent(event)) {
      if (event.type == sf::Event::Closed)
        window.close();
    }

    window.clear();
    mainTexture.clear(sf::Color(0,0,0,255));

    window.draw(mainSprite, &lightShader);
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
