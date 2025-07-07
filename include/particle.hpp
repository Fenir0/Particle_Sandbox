#ifndef PARTICLE
#define PARTICLE

#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>

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

    float mass;
    float inertia;

    public:
        
        Particle(ParticleType type = ParticleType::Air);

        void Update(std::vector<Particle>& grid, int i);

        void setCoord(float pos_x, float pos_y);
        void setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, float vel_x, float vel_y, float inertia);

        ParticleType        getType();
        ParticleState       getState();
        std::vector<short>  getColor();
        std::pair<int, int> getCoord();

        static const std::vector<ParticleType>  type_list;
        static const std::vector<std::string>  names_list;
        static const std::vector<std::vector<short>> colorSand;
        static const std::vector<std::vector<short>> colorWater;
        static const std::vector<std::vector<short>> colorStone;

        static ParticleState      getStateByType(ParticleType type);
        static std::vector<short> getColorByType(ParticleType type);
        static float getMassByType(ParticleType type);
        static float getInertiaByType(ParticleType type);
};

#endif