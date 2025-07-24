#ifndef UPDATER
#define UPDATER
#include "particle.hpp"

struct Surroundings;

class Update{
    public:
        static void update_WETSAND (Particle& self, std::vector<Particle> &grid, int i);
        static void update_SAND (Particle& self, std::vector<Particle> &grid, int i);
        static void update_WATER(Particle& self, std::vector<Particle> &grid, int i);
        static void update_OIL  (Particle& self, std::vector<Particle> &grid, int i);
        static void update_LAVA (Particle& self, std::vector<Particle> &grid, int i);
        static void update_STONE(Particle& self, std::vector<Particle> &grid, int i);
        static void update_SMOKE(Particle& self, std::vector<Particle> &grid, int i);
        static void update_STEAM(Particle& self, std::vector<Particle> &grid, int i);

        static void updateOnSurroundings(Surroundings& srd, Particle& self, std::vector<Particle> &grid, int i);

        static void getGravity (Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);
        static void getPressure(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);
        static void getFriction(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);

        static int  getClosestMove(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i, ParticleState state);
        
        static bool moreDense(Particle& self, int other, std::vector<Particle>& grid);

        static bool isSameType (std::vector<Particle>& grid, int i, int t);
        static bool isSameState(std::vector<Particle>& grid, int i, int t);

        static void swapper(float pos_x, float pos_y, 
                            float new_x, float new_y, 
                            std::vector<Particle>& grid);

        static int getGridIndex(float x, float y);
};

#endif