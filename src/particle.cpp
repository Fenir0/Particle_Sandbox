#include "../include/update.hpp"

int idc = 0;
int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;

const float Particle::STONE_FRICTION     = 0.6f;
const float Particle::SAND_FRICTION      = 0.7f;
const float Particle::GRAVITATIONAL_PULL = 0.2f;

const std::vector<std::string>              Particle::states_list 
                        {"Solid", "Fluid", "Gas"};
const std::vector<std::vector<std::string>> Particle::type_list 
                        {{"Stone", "Sand"}, {"Water", "Oil"}, {"Smoke"}};
const std::vector<std::vector<ParticleType>>  Particle::types 
                        {{ParticleType::Stone, ParticleType::Sand}, 
                        {ParticleType::Water, ParticleType::Oil}, 
                        {ParticleType::Smoke}};

const std::vector<std::vector<short>> Particle::colorSand{
    {230, 215, 160},{230, 210, 138},
    {242, 206, 134},{236, 211, 135},{244, 223, 146}};
const std::vector<std::vector<short>> Particle::colorWetSand{
    {204, 191, 142},{179, 169, 130},{173, 158, 104},
    {236, 211, 135},{209, 184, 110},{187, 176, 136}};
const std::vector<std::vector<short>> Particle::colorWater{
    {83, 117, 179},{66, 98, 158},{75, 109, 172},{56, 85, 140},
    {101, 125, 169},{73, 102, 155},{86, 118, 186},{101, 136, 200}
};
const std::vector<std::vector<short>> Particle::colorStone{{200 ,  200,  200}};
const std::vector<std::vector<short>> Particle::colorSmoke{
    { 15,  15,  15},{ 0,  0,  0},{ 85,  85,  85},{ 84,  88,  95}, { 40,  40, 40},
   // {60, 50, 50},{0, 0, 0}, {105,  88, 88},
    //{ 60,  60,  60},{33, 37, 41},{ 89,  89,  89},{127, 127, 127},
    { 52,  58,  64},{73, 80, 87},{108, 117, 125}
};
const std::vector<std::vector<short>> Particle::colorOil{
    { 0,  0,  0},{ 10,  10,  10},{ 0,  0,  10}
};

Particle::Particle(ParticleType type){
    this->type = type;
    this->id = idc++;
    vel_x = 0;
    vel_y = 0;
    cooldown = 0;
    density = getDensityByType(type);
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
    case ParticleType::WetSand:
        r = rand() % colorWetSand.size();
        return colorWetSand[r];
    case ParticleType::Water:
        r = rand() % colorWater.size();
        return colorWater[r];
    case ParticleType::Stone:
        return colorStone[0];
    case ParticleType::Oil:
        r = rand() % colorOil.size();
        return colorOil[r];
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
    case ParticleType::WetSand:
        return ParticleState::Solid;
    case ParticleType::Water:
        return ParticleState::Fluid;
    case ParticleType::Oil:
        return ParticleState::Fluid;
    case ParticleType::Stone:
        return ParticleState::Solid;
    }
    return ParticleState::None;
}
int                Particle::getDensityByType(ParticleType type){
    switch (type)
    {
    case ParticleType::None:
        return 0;
    case ParticleType::Smoke:
        return 0;
    case ParticleType::Sand:
        return 100;
    case ParticleType::WetSand:
        return 100;
    case ParticleType::Water:
        return 50;
    case ParticleType::Oil:
        return 80;
    case ParticleType::Stone:
        return 150;
    }
    return 0;
}
std::string Particle::getTypeAsString(ParticleType type){
    switch (type)
    {
    case ParticleType::Sand : return "Sand";
    case ParticleType::Stone: return "Stone";
    case ParticleType::Smoke: return "Smoke";
    case ParticleType::Water: return "Water";
    case ParticleType::Oil:   return "Oil";
    }
    return "LOLnotpossible";
}

void Particle::setCoord(float pos_x, float pos_y){
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}
void Particle::setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, float vel_x, float vel_y, 
                        float pressure, float inertia){
    this->type = type;
    this->state = state;
    this->color = color;
    this->vel_x = vel_x;
    this->vel_y = vel_y;
    this->pressure = pressure;
    this->inertia_x = inertia;
    this->inertia_y = inertia;
    this->energy = pressure*(inertia_x*inertia_x+inertia_y*inertia_y)/2;
    this->density = getDensityByType(type);
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

bool Particle::isTypeAndState(ParticleType type, ParticleState state){
    switch (state)
    {
    case ParticleState::Solid:
        if(type == ParticleType::Stone) return true;
        if(type == ParticleType::Sand)  return true;
        return false;
    case ParticleState::Fluid:
        if(type == ParticleType::Water) return true;
        if(type == ParticleType::Oil)   return true;
        return false;
    case ParticleState::Gas:
        if(type == ParticleType::Smoke) return true;
        return false;
    }
    return false;
}

void Particle::Update(std::vector<Particle>& grid, int i){
    switch (type)
    {
    case ParticleType::Sand   : Update::update_SAND   (*this, grid, i); break;
    case ParticleType::WetSand: Update::update_WETSAND(*this, grid, i); break;
    case ParticleType::Water  : Update::update_WATER  (*this, grid, i); break;
    case ParticleType::Stone  : Update::update_STONE  (*this, grid, i); break;
    case ParticleType::Smoke  : Update::update_SMOKE  (*this, grid, i); break;
    case ParticleType::Oil    : Update::update_OIL    (*this, grid, i); break;
    }

}