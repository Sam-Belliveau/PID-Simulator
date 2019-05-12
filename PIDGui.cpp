#include <SFML/Graphics.hpp>
#include <iostream>
#include <thread>
#include <deque>
#include <cstdint>
#include "Systems.hpp"

constexpr std::size_t Width = 1280;
constexpr std::size_t Height = 720;
constexpr std::size_t BufferSize = 1*Width/2;

int main() 
{
    sf::ContextSettings set;
    set.antialiasingLevel = 16;

    sf::RenderWindow window(sf::VideoMode(Width, Height), "PID Test", 7, set);

    PIDLoop pid(1, 0.2, 0.15);

    auto PIDEditor = [&pid, &window]() {
        while(window.isOpen())
        {
            std::cout << "\nP: " << pid.P << "\nI: " << pid.I << "\nD: " << pid.D << "\n\n";
            std::cout << "Enter Var: ";
            std::string in;
            std::getline(std::cin, in);

            switch (in[0])
            {
            case 'p': case 'P':
                std::cout << "Enter Value: ";
                std::cin >> pid.P;
                break;

            case 'i': case 'I':
                std::cout << "Enter Value: ";
                std::cin >> pid.I;
                break;

            case 'd': case 'D':
                std::cout << "Enter Value: ";
                std::cin >> pid.D;
                break;

            default:
                std::cout << "Please Enter P, I, or D!\n";
                break;
            }

            std::cin.clear();
            std::cin.ignore(10000,'\n');
        }
    };

    std::thread editor = std::thread(PIDEditor);

    pid.setPos(-1);

    std::deque<double> posbuf(BufferSize, 0);
    std::deque<double> intbuf(BufferSize, 0);
    std::deque<double> derbuf(BufferSize, 0);
    std::deque<double> errbuf(BufferSize, 0);


    sf::VertexArray graph[4];
    for(auto& i : graph)
        i = sf::VertexArray(sf::LineStrip, BufferSize);

    sf::VertexArray middle(sf::Lines, 2);

    middle[0].color = sf::Color(200, 200, 200);
    middle[1].color = sf::Color(200, 200, 200);
    middle[0].position.x = 0;
    middle[0].position.y = Height/2;
    middle[1].position.x = Width;
    middle[1].position.y = Height/2;

    int frame = 0;
    window.setFramerateLimit(FPS);
    while(window.isOpen()) 
    {
        if(++frame == FPS) frame = 0;

        sf::Event event;
        while (window.pollEvent(event))
        {
            // Close window: exit
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Clear screen
        window.clear();

        posbuf.pop_front();
        intbuf.pop_front();
        derbuf.pop_front();
        errbuf.pop_front();
        posbuf.push_back(pid.getPos());
        intbuf.push_back(pid.integral);
        derbuf.push_back(pid.derivative);
        errbuf.push_back(pid.previous_error);

        if(sf::Mouse::isButtonPressed(sf::Mouse::Middle))
        {
            pid.reset();
            double pos = 1.0 - sf::Mouse::getPosition(window).y/double(Height/2);
            for(double& i : posbuf) i = pos;
            pid.setPos(pos);
        }

        pid.update();

        for(std::size_t i = 0; i < BufferSize; ++i) 
        {
            graph[0][i].color = sf::Color::White;
            graph[0][i].position.x = i;
            graph[0][i].position.y = Height - ((posbuf[i] + 1) * double(Height) / 2.0);

            graph[1][i].color = sf::Color::Red;
            graph[1][i].position.x = i;
            graph[1][i].position.y = Height - ((errbuf[i] + 1) * double(Height) / 2.0);

            graph[2][i].color = sf::Color::Green;
            graph[2][i].position.x = i;
            graph[2][i].position.y = Height - ((derbuf[i] + 1) * double(Height) / 2.0);

            graph[3][i].color = sf::Color::Blue;
            graph[3][i].position.x = i;
            graph[3][i].position.y = Height - ((intbuf[i] + 1) * double(Height) / 2.0);
        }

        for(int i = 0; i < Width + FPS; i += FPS) {
            sf::VertexArray t(sf::Lines, 2);

            t[0].color = sf::Color(200, 200, 200, 128);
            t[1].color = sf::Color(200, 200, 200, 128);
            t[0].position.x = Width - i - frame;
            t[1].position.x = Width - i - frame;
            t[0].position.y = 0;
            t[1].position.y = Height;

            window.draw(t);
        }

        window.draw(middle);
        for(int i = 3; i >= 0; --i)
            window.draw(graph[i]);

        // Update the window
        window.display();
    }

    editor.join();
}