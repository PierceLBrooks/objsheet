
// Author: Pierce Brooks

#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <SFML3D/Graphics.hpp>
#include <SFML3D/Window/Event.hpp>
#include <SFML3D/Window/Mouse.hpp>
#include <SFML3D/Window/Keyboard.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

class ObjModel : public sf3d::Model
{
private:
    struct FaceData
    {
    public:
        unsigned int position0, position1, position2;
        unsigned int textureCoordinate0, textureCoordinate1, textureCoordinate2;
        unsigned int normal0, normal1, normal2;
    };

    sf3d::Vertex makeVertex(const std::string& indices)
    {
        sf3d::Vertex vertex;
        std::istringstream indiceStream(indices);

        unsigned int positionIndex = 0;
        unsigned int textureCoordinateIndex = 0;
        unsigned int normalIndex = 0;
        char separator = 0;

        if (!(indiceStream >> positionIndex >> separator).good())
        {
            return vertex;
        }

        if (!(indiceStream >> textureCoordinateIndex >> separator).good())
        {
            return vertex;
        }

        if (!(indiceStream >> normalIndex).eof())
        {
            return vertex;
        }

        // .obj indices start at 1
        positionIndex -= 1;
        textureCoordinateIndex -= 1;
        normalIndex -= 1;

        if ((positionIndex >= m_vertexPositions.size()) ||
            (textureCoordinateIndex >= m_vertexTextureCoordinates.size()) ||
            (normalIndex >= m_vertexNormals.size()))
        {
            return vertex;
        }

        vertex.position = m_vertexPositions[positionIndex];
        vertex.texCoords = m_vertexTextureCoordinates[textureCoordinateIndex];
        vertex.normal = m_vertexNormals[normalIndex];

        // Not needed, but just for demonstration
        vertex.color = getColor();

        return vertex;
    }

public:
    bool loadFromFile(const std::string& filename)
    {
        // Open our model file
        std::ifstream modelFile(filename.c_str());
        if (!modelFile.is_open())
        {
            return false;
        }

        // Parse the model file line by line
        std::string line;
        std::istringstream lineStream;
        std::string token;

        while (true)
        {
            std::getline(modelFile, line);

            // Break on error or failure to read (end of file)
            if ((modelFile.bad()) || (modelFile.fail()))
            {
                break;
            }

            lineStream.clear();
            lineStream.str(line);
            lineStream >> token;

            if (token == "v")
            {
                // Handle vertex positions
                sf3d::Vector3f position;
                lineStream >> position.x >> position.y >> position.z;
                m_vertexPositions.push_back(position);
            }
            else if (token == "vt")
            {
                // Handle vertex texture coordinates
                sf3d::Vector2f coordinate;
                lineStream >> coordinate.x >> coordinate.y;
                m_vertexTextureCoordinates.push_back(coordinate);
            }
            else if (token == "vn")
            {
                // Handle vertex normals
                sf3d::Vector3f normal;
                lineStream >> normal.x >> normal.y >> normal.z;
                m_vertexNormals.push_back(normal);
            }
            else if (token == "f")
            {
                // Handle faces
                std::string vertexString0, vertexString1, vertexString2;

                lineStream >> vertexString0 >> vertexString1 >> vertexString2;

                sf3d::Vertex vertex0(makeVertex(vertexString0));
                sf3d::Vertex vertex1(makeVertex(vertexString1));
                sf3d::Vertex vertex2(makeVertex(vertexString2));

                addVertex(vertex0);
                addVertex(vertex1);
                addVertex(vertex2);

                unsigned int index = getVertexCount();

                addFace(index - 3, index - 2, index - 1);
            }
        }

        // Update the underlying polyhedron geometry
        update();

        return true;
    }

private:
    std::vector<sf3d::Vector3f> m_vertexPositions;
    std::vector<sf3d::Vector2f> m_vertexTextureCoordinates;
    std::vector<sf3d::Vector3f> m_vertexNormals;
    std::vector<FaceData> m_faceData;
};

int run(sf3d::RenderWindow& window, sf3d::RenderTexture& frameTexture, const std::string& argument)
{
    bool focus = true;
    float length = 1.0f;
    float magnitude = 0.0f;
    float pi = 3.141592654f;
    float yaw = pi / 2.0f;
    float pitch = 0.0f;
    float zoom = -100.0f;
    float delta = 0.0f;
    sf3d::Light light;
    sf3d::Clock clock;
    sf3d::Sprite frame(frameTexture.getTexture());
    sf3d::Cuboid axisX(sf3d::Vector3f(500.0f, 2.0f, 2.0f));
    sf3d::Cuboid axisY(sf3d::Vector3f(2.0f, 500.0f, 2.0f));
    sf3d::Cuboid axisZ(sf3d::Vector3f(2.0f, 2.0f, 500.0f));
    sf3d::Camera camera(90.0f, 0.001f, 1000.0f);
    sf3d::Vector2f coordinate;
    sf3d::Vector3f previous;
    sf3d::Vector3f direction;
    sf3d::Vector3f rightVector;
    sf3d::Vector3f offset;
    sf3d::Vector3f origin = camera.getPosition();
    float aspectRatio = static_cast<float>(window.getSize().x) / static_cast<float>(window.getSize().y);
    sf3d::Vector3f downscaleFactor(1.0f / static_cast<float>(window.getSize().x) * aspectRatio, -1.0f / static_cast<float>(window.getSize().y), 1.0f);
    camera.scale(1.0f / aspectRatio, 1.0f, 1.0f);
    camera.setPosition(sf3d::Vector3f());
    window.setView(camera);
    light.setColor(sf3d::Color::White);
    light.setAmbientIntensity(0.5f);
    light.setDiffuseIntensity(1.0f);
    light.setLinearAttenuation(0.002f);
    light.setQuadraticAttenuation(0.0005f);
    light.enable();
    sf3d::Light::enableLighting();
    frame.setScale(0.75f, 0.75f);
    axisX.setColor(sf3d::Color::Red);
    axisX.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisX.setOrigin(sf3d::Vector3f(-250.0f, 1.0f, 1.0f));
    axisY.setColor(sf3d::Color::Green);
    axisY.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisY.setOrigin(sf3d::Vector3f(1.0f, -250.0f, 1.0f));
    axisZ.setColor(sf3d::Color::Blue);
    axisZ.setPosition(sf3d::Vector3f(0.0f, 0.0f, 0.0f));
    axisZ.setOrigin(sf3d::Vector3f(1.0f, 1.0f, -250.0f));
    
    // Enable depth testing so we can draw 3D objects in any order
    frameTexture.enableDepthTest(true);

    // Keep the mouse cursor within the window
    sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);

    clock.restart();
    while (window.isOpen())
    {
        int deltaX = 0;
        int deltaY = 0;
        glm::vec3 upVector;
        sf3d::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
                case sf3d::Event::Closed:
                    window.close();
                    break;
                case sf3d::Event::KeyPressed:
                    switch (event.key.code)
                    {
                        case sf3d::Keyboard::Key::Escape:
                            window.close();
                            break;
                        case sf3d::Keyboard::Key::Return:
                            offset = sf3d::Vector3f();
                            break;
                    }
                    break;
                case sf3d::Event::MouseMoved:
                    deltaX = event.mouseMove.x - (window.getSize().x / 2);
                    deltaY = event.mouseMove.y - (window.getSize().y / 2);
                    break;
                case sf3d::Event::MouseWheelMoved:
                    zoom += event.mouseWheel.delta;
                    break;
                case sf3d::Event::GainedFocus:
                    focus = true;
                    break;
                case sf3d::Event::LostFocus:
                    focus = false;
                    break;
                case sf3d::Event::MouseButtonReleased:
                    if (event.mouseButton.button == sf3d::Mouse::Button::Left)
                    {
                        offset = camera.getPosition() - offset;
                        previous = direction;
                    }
                    break;
            }
        }
        if (!window.isOpen())
        {
            break;
        }
        delta = clock.restart().asSeconds();
        if (!focus)
        {
            window.clear(sf3d::Color::Black);
            window.display();
            continue;
        }

        // Keep the mouse cursor within the window
        sf3d::Mouse::setPosition(sf3d::Vector2i(window.getSize()) / 2, window);

        if (sf3d::Mouse::isButtonPressed(sf3d::Mouse::Button::Left))
        {
            coordinate.x += deltaX;
            coordinate.y += deltaY;

            while (coordinate.x > 360.0f)
            {
                coordinate.x -= 360.0f;
            }
            while (coordinate.x < 0.0f)
            {
                coordinate.x += 360.0f;
            }
            if (coordinate.y > 90.0f)
            {
                coordinate.y = 90.0f;
            }
            if (coordinate.y < -90.0f)
            {
                coordinate.y = -90.0f;
            }

            direction.x = cosf(coordinate.x*(pi/180.0f))*sinf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            direction.y = cosf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            direction.z = sinf(coordinate.x*(pi/180.0f))*sinf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            rightVector.x = cosf((coordinate.x+(pi*0.25f))*(pi/180.0f))*sinf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            rightVector.y = cosf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            rightVector.z = sinf((coordinate.x+(pi*0.25f))*(pi/180.0f))*sinf((coordinate.y*(pi/180.0f))+(pi*0.5f));
            camera.setPosition(origin + offset + (direction * zoom));
        }
        else
        {
            yaw -= (deltaX / 5.0f) * delta;
            pitch -= (deltaY / 5.0f) * delta;
            direction.x = cosf(yaw) * cosf(pitch);
            direction.y = sinf(pitch);
            direction.z = -sinf(yaw) * cosf(pitch);
            //direction += previous;
            //magnitude = sqrtf((direction.x * direction.x) + (direction.y * direction.y) + (direction.z * direction.z));
            //direction /= magnitude;
            rightVector.x = -direction.z;
            rightVector.y = 0.0f;
            rightVector.z = direction.x;
            magnitude = sqrtf((rightVector.x * rightVector.x) + (rightVector.y * rightVector.y) + (rightVector.z * rightVector.z));
            rightVector /= magnitude;

            // W key pressed : move forward
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::W))
            {
                offset += sf3d::Vector3f(direction * 50.f * delta);
            }

            // A key pressed : strafe left
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::A))
            {
                offset += sf3d::Vector3f(rightVector * -50.f * delta);
            }

            // S key pressed : move backward
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::S))
            {
                offset += sf3d::Vector3f(direction * -50.f * delta);
            }

            // D key pressed : strafe right
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::D))
            {
                offset += sf3d::Vector3f(rightVector * 50.f * delta);
            }

            // Space bar pressed : move upwards
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::Space))
            {
                offset += sf3d::Vector3f(0, 50.0f * delta, 0.0f);
            }

            // Left shift key pressed : move downwards
            if (sf3d::Keyboard::isKeyPressed(sf3d::Keyboard::Key::LShift))
            {
                offset += sf3d::Vector3f(0.0f, -50.0f * delta, 0.0f);
            }

            camera.setPosition(origin + offset);
        }

        upVector = glm::cross(glm::vec3(direction.x, direction.y, direction.z), glm::vec3(rightVector.x, rightVector.y, rightVector.z));
        camera.setDirection(direction);
        camera.setUpVector(-sf3d::Vector3f(upVector.x, upVector.y, upVector.z));

        // Inform the window to update its view with the new camera data
        frameTexture.setView(camera);

        // Clear the window
        frameTexture.clear(sf3d::Color::Black);

        light.setPosition(camera.getPosition());

        // Draw the background
        //frameTexture.draw(sf3d::Color::Black);

        // Disable lighting for the text and the light sphere
        sf3d::Light::disableLighting();

        // Disable depth testing for sf3d::Text because it requires blending
        frameTexture.enableDepthTest(false);
        //frameTexture.draw(text);
        frameTexture.enableDepthTest(true);

        // Enable lighting again
        sf3d::Light::enableLighting();

        // Draw everything
        frameTexture.draw(axisX);
        frameTexture.draw(axisY);
        frameTexture.draw(axisZ);

        // Disable lighting and reset to 2D view to draw information
        sf3d::Light::disableLighting();
        frameTexture.setView(window.getDefaultView());

        // Draw informational text
        //frameTexture.draw(info);

        // Reset view to our camera and enable lighting again
        frameTexture.setView(camera);
        sf3d::Light::enableLighting();

        // Finally, display the rendered frame on screen
        frameTexture.display();

        window.clear(sf3d::Color(128, 128, 128));
        window.draw(frame);
        window.display();
    }
    return 0;
}

int main(int argc, char** argv)
{
    int result;
    std::vector<std::string> arguments;
    sf3d::RenderWindow* window = new sf3d::RenderWindow();
    sf3d::RenderTexture* frameTexture = new sf3d::RenderTexture();
    window->create(sf3d::VideoMode(1280, 720), "objsheet");
    window->setFramerateLimit(60);
    window->setVerticalSyncEnabled(true);
    window->setMouseCursorVisible(false);
    frameTexture->create(window->getSize().x, window->getSize().y, true);
    for (int i = 0; i != argc; ++i)
    {
        arguments.push_back(std::string(argv[i]));
        std::cout << i << '\t' << arguments.back() << std::endl;
    }
    if (arguments.size() < 2)
    {
        window->close();
        delete window;
        return -1;
    }
    result = run(*window, *frameTexture, arguments.back());
    delete frameTexture;
    delete window;
    return result;
}

