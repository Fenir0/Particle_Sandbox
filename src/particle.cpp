#include "../include/update.hpp"

int idc = 0;
int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;

const float Particle::STONE_FRICTION     = 0.6f;
const float Particle::SAND_FRICTION      = 0.7f;
const float Particle::GRAVITATIONAL_PULL = 0.2f;

const std::vector<std::string> Particle::names_list {"Sand", "Water", "Smoke", "Stone"};
const std::vector<ParticleType> Particle::type_list {ParticleType::Sand, ParticleType::Water,
                ParticleType::Smoke, ParticleType::Stone};

const std::vector<std::vector<short>> Particle::colorSand{
    {230, 215, 160},{204, 191, 142},{179, 169, 130},{230, 210, 138},{173, 158, 104},
    {242, 206, 134},{236, 211, 135},{244, 223, 146},{209, 184, 110},{187, 176, 136}};
const std::vector<std::vector<short>> Particle::colorWater{{102, 143, 220}};
const std::vector<std::vector<short>> Particle::colorStone{{84 ,  88,  95}};
const std::vector<std::vector<short>> Particle::colorSmoke{
    { 15,  15,  15},{ 0,  0,  0},{ 85,  85,  85},{ 84,  88,  95}, { 40,  40, 40},
    {60, 50, 50},{0, 0, 0}, {105,  88, 88},
    { 60,  60,  60},{33, 37, 41},{ 89,  89,  89},{127, 127, 127},
    { 52,  58,  64},{73, 80, 87},{108, 117, 125}
};

Particle::Particle(ParticleType type){
    this->type = type;
    this->id = idc++;
    vel_x = 0;
    vel_y = 0;

    state = getStateByType(type);
    color = getColorByType(type);
}

std::vector<short> Particle::getColorByType(ParticleType type){
    int r;
    switch (type)
    {
    case ParticleType::Smoke:
        r = rand() % colorSmoke.size();
        return colorSmoke[r];
    case ParticleType::None:
        return {123, 123, 123};
    case ParticleType::Sand:
        r = rand() % colorSand.size();
        return colorSand[r];
    case ParticleType::Water:
        return colorWater[0];
    case ParticleType::Stone:
        return colorStone[0];
    }
    return {0, 0, 0};
}
ParticleState      Particle::getStateByType(ParticleType type){
    switch (type)
    {
    case ParticleType::None:
        return ParticleState::None;
    case ParticleType::Smoke:
        return ParticleState::Gas;
    case ParticleType::Sand:
        return ParticleState::Solid;
    case ParticleType::Water:
        return ParticleState::Fluid;
    case ParticleType::Stone:
        return ParticleState::Solid;
    }
    return ParticleState::None;
}

void Particle::setCoord(float pos_x, float pos_y){
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}
void Particle::setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, float vel_x, float vel_y, 
                        float mass, float inertia){
    this->type = type;
    this->state = state;
    this->color = color;
    this->vel_x = vel_x;
    this->vel_y = vel_y;
    this->mass = mass;
    this->inertia_x = inertia;
    this->inertia_y = inertia;
    this->energy = mass*(inertia_x*inertia_x+inertia_y*inertia_y)/2;
}
ParticleType        Particle::getType(){
    return this->type;
};
ParticleState       Particle::getState(){
    return this->state;
};
std::vector<short>  Particle::getColor(){
    return this->color;
};
std::pair<float, float> Particle::getCoord(){
    return std::make_pair(std::floor(this->pos_x), std::floor(this->pos_y));

}


void Particle::Update(std::vector<Particle>& grid, int i){
    switch (type)
    {
    case ParticleType::Sand:  Update::update_SAND (*this, grid, i); break;
    case ParticleType::Water: Update::update_WATER(*this, grid, i); break;
    case ParticleType::Stone: Update::update_STONE(*this, grid, i); break;
    case ParticleType::Smoke: Update::update_SMOKE(*this, grid, i); break;
    }

}