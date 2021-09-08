
// Author: Pierce Brooks

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <SFML3D/Graphics/RenderWindow.hpp>
#include <SFML3D/Window/Event.hpp>
#include <SFML3D/Window/Mouse.hpp>

int main()
{
    sf3d::RenderWindow* window = new sf3d::RenderWindow();
    window->create(sf3d::VideoMode(1280, 720), "Game");
    window->setFramerateLimit(60);
    while (window->isOpen())
    {
        sf3d::Event event;
        while (window->pollEvent(event))
        {
            switch (event.type)
            {
            case sf3d::Event::Closed:
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
