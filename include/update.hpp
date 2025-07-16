#ifndef UPDATER
#define UPDATER
#include "particle.hpp"

struct Surroundings;

class Update{
    public:
        static bool freezeSelect;

        static void update_SAND (Particle& self, std::vector<Particle> &grid, int i);
        static void update_WATER(Particle& self, std::vector<Particle> &grid, int i);
        static void update_STONE(Particle& self, std::vector<Particle> &grid, int i);
        static void update_SMOKE(Particle& self, std::vector<Particle> &grid, int i);

        // Checkers
        static void getGravity (Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);
        static void getPressure(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);
        static void getFriction(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);

        static int getClosestMove(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i, ParticleState state);

        static void transferEnergy(Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i); 
        static void transmitEnergy (Surroundings& sr, Particle& self, std::vector<Particle>& grid, int i);

        static void clamp(Particle& self);

        static void swapper(float pos_x, float pos_y, 
                            float new_x, float new_y, 
                            std::vector<Particle>& grid);
        static bool isValid(int x, int y);
        static int getGridIndex(float x, float y);
};

#endif