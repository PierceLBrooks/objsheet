
// Author: Pierce Brooks

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

int main()
{
    sf::RenderWindow* window = new sf::RenderWindow();
    window->create(sf::VideoMode(1280, 720), "Game");
    window->setFramerateLimit(60);
    while (window->isOpen())
    {
        sf::Event event;
        while (window->pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window->close();
                break;
            }
        }
        if (!window->isOpen())
        {
            break;
        }
        window->display();
    }
    delete window;
    return 0;
}
