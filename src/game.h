#ifndef GAME
#define GAME

#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>


enum class ParticleType {Air, Sand, Water};
enum class ParticleState {Solid, Fluid, Gas};
class Particle{
    int id;
    void colorPainter();
    float pos_x;
    float pos_y;
    public:
        ParticleType type;
        ParticleState state;
        std::vector<short> color {0, 0, 0};
        float vel_x;
        float vel_y;
        
        Particle(ParticleType type);

        void Update(std::vector<Particle>& grid);
        void setCoord(float pos_x, float pos_y);
        std::pair<int, int> getCoord();
};

class Game{
    private:
        unsigned int WIDTH;
        unsigned int HEIGHT;

        sf::RenderWindow *window;
        sf::Event evt;
        sf::VideoMode vm;

        sf::Texture texture;
        sf::Sprite sprite;

        std::vector<sf::Uint8> pxs; // pixels
        std::vector<Particle> grid; // simulation
        sf::VertexArray particles;

        void initVar();
        void initWin();

        inline void ButtonCheck();
        inline void MouseCheck();

        
    public:
        Game();
        ~Game();

        bool isRunning() const;

        void update();
        void render();
};

#endif
