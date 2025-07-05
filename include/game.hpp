#ifndef GAME
#define GAME

#include <SFML/Graphics.hpp>
#include "particle.hpp" 

class Game{
    private:
        unsigned int WIDTH;
        unsigned int HEIGHT;

        sf::RenderWindow *window;
        sf::Event            evt;
        sf::VideoMode         vm;

        std::vector<sf::Uint8>  pxs; // pixels
        std::vector<Particle>  grid; // simulation
        sf::VertexArray   particles;

        void initVar();
        void initWin();
        
    public:
        Game();
        ~Game();

        bool isRunning() const;

        void update();
        void render();
};

#endif
