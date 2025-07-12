#ifndef GAME
#define GAME

#include <SFML/Graphics.hpp>
#include "update.hpp" 

class Game{
    private:
        unsigned int WIDTH;
        unsigned int HEIGHT;

        sf::RenderWindow *window;
        sf::Event            evt;
        sf::VideoMode         vm;
        sf::Font            font;

        unsigned int particleCount;

        std::vector<sf::Uint8>  pxs; // pixels

        std::vector<Particle>  grid; // simulation
        sf::VertexArray   particles;

        void initVar();
        void initWin();

        int borderSize;

        void drawUI();
        void drawButton();
        void drawBorder();
        void calculateWindowSize();
        void updateUIScaling();

        const sf::Color screenColor {123, 123, 123, 155};

        ParticleType selectedType = ParticleType::Sand;
        int brushSize;

        void placeParticleOnScene(int mx, int my, ParticleType type);
        void mouseInput();
    public:
        Game();
        ~Game();

        bool isRunning() const;

        void update();
        void render();
};

#endif
