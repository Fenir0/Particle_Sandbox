#ifndef PARTICLE
#define PARTICLE

#include <cstdlib>
#include <algorithm>
#include <cmath>
#include <vector>
#include <string>
#include <random>

enum class ParticleType {Smoke, Sand, Water, Oil, Lava, Stone, Obsidian, None, WetSand, Steam};
enum class ParticleState {Solid, Fluid, Gas, None};

extern int GRID_WIDTH;
extern int GRID_HEIGHT;

class Particle{
    public:
        int id;

        int cooldown;
        int density; 
        int wetness;
        int temperature;

        float pos_x;
        float pos_y;
        float vel_x;  // speed as viewed by user
        float vel_y;

        ParticleType type;
        ParticleState state;
        std::vector<short> color {0, 0, 0};

        float pressure;    // pressure from above
        float inertia_x;  // kinda abstract term here but fits 
        float inertia_y; 

        // inertia == overall speed
        // colliding and rendering purposes: 
        //      a maximum movement of 1 tile no matter the speed
        //     => saving the speed as inertia to make impacts more significant
        //      
        
        float energy;   //  impact: mV^2/2 where V == inertia 

        static const float STONE_FRICTION;
        static const float SAND_FRICTION;
        static const float GRAVITATIONAL_PULL;
        Particle(ParticleType type = ParticleType::None);

        void Update(std::vector<Particle>& grid, int i);

        void setCoord(float pos_x, float pos_y);
        void setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, 
                        float vel_x, float vel_y, 
                        float pressure, float inertia, int temperature);

        ParticleType        getType();
        ParticleState       getState();
        std::vector<short>  getColor();
        std::pair<float, float> getCoord();

        static const std::vector<std::vector<ParticleType>> types;
        static const std::vector<std::vector<std::string>>  type_list;
        static const std::vector<std::string>               states_list;
        static const std::vector<std::vector<short>> colorSand;
        static const std::vector<std::vector<short>> colorWetSand;
        static const std::vector<std::vector<short>> colorWater;
        static const std::vector<std::vector<short>> colorStone;
        static const std::vector<std::vector<short>> colorSmoke;
        static const std::vector<std::vector<short>> colorOil;
        static const std::vector<std::vector<short>> colorLava;
        static const std::vector<std::vector<short>> colorObsidian;
        static const std::vector<std::vector<short>> colorSteam;

        static ParticleState      getStateByType   (ParticleType type);
        static std::vector<short> getColorByType   (ParticleType type);
        static float              getPressureByType(ParticleType type);
        static int                getDensityByType (ParticleType type);
        static int                getTempByType    (ParticleType type);
        static std::string        getTypeAsString  (ParticleType type);

        static bool               moreDense(Particle& self, int other, std::vector<Particle>& grid);
        static bool isTypeAndState(ParticleType type, ParticleState state);
};

#endif