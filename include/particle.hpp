#ifndef PARTICLE
#define PARTICLE

#include <cstdlib>
#include <cmath>
#include <vector>
#include <cstdio>

enum class ParticleType {Air, Sand, Water, Stone};
enum class ParticleState {Solid, Fluid, Gas};

extern int GRID_WIDTH;
extern int GRID_HEIGHT;

class Particle{
    int id;

    float pos_x;
    float pos_y;
    float vel_x;
    float vel_y;

    ParticleType type;
    ParticleState state;
    std::vector<short> color {0, 0, 0};

    public:
        
        Particle(ParticleType type = ParticleType::Air);

        void Update(std::vector<Particle>& grid, int i);

        void setCoord(float pos_x, float pos_y);
        void setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, float vel_x, float vel_y);

        ParticleType        getType();
        ParticleState       getState();
        std::vector<short>  getColor();
        std::pair<int, int> getCoord();

        static ParticleState      getStateByType(ParticleType type);
        static std::vector<short> getColorByType(ParticleType type);
};

#endif