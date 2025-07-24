#include "../include/update.hpp"

int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;

const float Particle::GRAVITATIONAL_PULL = 0.2f;

/* PLACEABLE TYPES (INTERFACE)
    Only those available to user
*/
const std::vector<std::string>                Particle::states_list 
                        {"Solid", "Fluid", "Gas"};
const std::vector<std::vector<std::string>>   Particle::type_list 
                        {{"Stone", "Sand"}, {"Water", "Oil", "Lava"}, {"Smoke"}};
const std::vector<std::vector<ParticleType>>  Particle::types 
                        {{ParticleType::Stone, ParticleType::Sand}, 
                        {ParticleType::Water, ParticleType::Oil, ParticleType::Lava}, 
                        {ParticleType::Smoke}};

/* COLORS*/

const std::vector<std::vector<short>> Particle::colorSand{
    {230, 215, 160},{230, 210, 138},
    {242, 206, 134},{236, 211, 135},{244, 223, 146}};
const std::vector<std::vector<short>> Particle::colorBurnedSand{
    {74, 76, 78},{70, 75, 80},{80, 75, 70},{65, 75, 70},{70, 80, 65}};
const std::vector<std::vector<short>> Particle::colorWetSand{
    {204, 191, 142},{179, 169, 130},{173, 158, 104},
    {236, 211, 135},{209, 184, 110},{187, 176, 136}};
const std::vector<std::vector<short>> Particle::colorWater{
    {83, 117, 179},{66, 98, 158},{75, 109, 172},{56, 85, 140},
    {101, 125, 169},{73, 102, 155},{86, 118, 186},{101, 136, 200}
};
const std::vector<std::vector<short>> Particle::colorStone{
    {217, 217, 217},{217, 217, 217},{217, 217, 217},{217, 217, 217},{217, 217, 217},
    {208, 199, 199},
    {193, 177, 177},{185, 185, 185},{208, 199, 199}
};
const std::vector<std::vector<short>> Particle::colorSmoke{
    { 15,  15,  15},{ 0,  0,  0},{ 85,  85,  85},{ 84,  88,  95}, { 40,  40, 40},
    { 0,  0,  0},{73, 80, 87},{0, 0, 0}
};
const std::vector<std::vector<short>> Particle::colorLava{
    {238,  53,  53},{238,  53,  53},{238, 53, 53},
    {234, 252,  99},{222,  94,  94},{222, 94, 94},
    {116,  45,  45},{212,  62,  62},{194, 73, 73}
};
const std::vector<std::vector<short>> Particle::colorOil{
    {46, 38, 29},{80,  68,  55},{58, 46, 34},
    {78, 62, 45}
};
const std::vector<std::vector<short>> Particle::colorSteam{
    {216, 207, 207},{240,  231,  231},{225, 209, 209},
    {208, 197, 197}
};

/* TYPE-RELATED LOGIC*/
std::vector<short> Particle::getColorByType  (ParticleType type){
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
    case ParticleType::Lava:
        r = rand() % colorLava.size();
        return colorLava[r];
    case ParticleType::Stone:
        r = rand() % colorStone.size();
        return colorStone[r];
    case ParticleType::Oil:
        r = rand() % colorOil.size();
        return colorOil[r];
    case ParticleType::Steam:
        r = rand() % colorSteam.size();
        return colorSteam[r];
    }
    return {0, 0, 0};
}
ParticleState      Particle::getStateByType  (ParticleType type){
    switch (type)
    {
    case ParticleType::None:
        return ParticleState::None;

    case ParticleType::Sand:
    case ParticleType::WetSand:
    case ParticleType::Stone:
        return ParticleState::Solid;

    case ParticleType::Water:
    case ParticleType::Oil:
    case ParticleType::Lava:
        return ParticleState::Fluid;

    case ParticleType::Smoke:
    case ParticleType::Steam:
        return ParticleState::Gas;
    }
    return ParticleState::None;
}
int                Particle::getDensityByType(ParticleType type){
    switch (type)
    {
    case ParticleType::None:
        return 0;
    case ParticleType::Smoke:
        return 70;
    case ParticleType::Steam:
        return 90;
    case ParticleType::Oil:
        return 150;
    case ParticleType::Water:
        return 180;
    case ParticleType::Lava:
        return 180;
    case ParticleType::Sand:
        return 200;
    case ParticleType::WetSand:
        return 200;
    case ParticleType::Stone:
        return 250;
    }
    return 0;
}
int                Particle::getTempByType   (ParticleType type){
    switch (type)
    {
    case ParticleType::Steam:
        return 100;
    case ParticleType::Lava:
        return 4000;
    case ParticleType::Sand:
    case ParticleType::WetSand:
    case ParticleType::Water:
    case ParticleType::Oil:
        return 0;
    }
    return 0;
}
std::string        Particle::getTypeAsString (ParticleType type){
    switch (type)
    {
    case ParticleType::Sand    : return "Sand";
    case ParticleType::WetSand : return "Sand";
    case ParticleType::Stone   : return "Stone";
    case ParticleType::Smoke   : return "Smoke";
    case ParticleType::Water   : return "Water";
    case ParticleType::Lava    : return "Lava";
    case ParticleType::Oil     : return "Oil";
    case ParticleType::Steam   : return "Steam";
    }
    return "LOLnotpossible";
}


bool Particle::isTypeAndState(ParticleType type, ParticleState state){
    switch (state)
    {
    case ParticleState::Solid:
        if(type == ParticleType::Stone)     return true;
        if(type == ParticleType::Sand)      return true;
        return false;
    case ParticleState::Fluid:
    if(type == ParticleType::Lava)   return true;
        if(type == ParticleType::Water) return true;
        if(type == ParticleType::Oil)   return true;
        return false;
    case ParticleState::Gas:
        if(type == ParticleType::Smoke) return true;
        if(type == ParticleType::Steam) return true;
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
    case ParticleType::Lava   : Update::update_LAVA   (*this, grid, i); break;
    case ParticleType::Stone  : Update::update_STONE  (*this, grid, i); break;
    case ParticleType::Smoke  : Update::update_SMOKE  (*this, grid, i); break;
    case ParticleType::Oil    : Update::update_OIL    (*this, grid, i); break;
    case ParticleType::Steam  : Update::update_STEAM  (*this, grid, i); break;
    }
}

/* CONSTRUCTOR*/
Particle::Particle(ParticleType type){
    this->type = type;
    cooldown = 0;
    wetness  = 0;
    state    = getStateByType(type);
}
// Particle::Particle(const Particle& other){
//     this->type = other.type;
//     this->pos_x = other.pos_x;
//     this->pos_y = other.pos_y;
//     this->vel_x = other.vel_x;
//     this->vel_y = other.vel_y;
//     this->pressure = other.pressure;
//     this->inertia_x = other.inertia_x;
//     this->inertia_y = other.inertia_y;
//     this->temperature = other.temperature;
//     color   = getColorByType(type);
//     density = getDensityByType(type);
//     state   = getStateByType(type);
// }


/*SETTERS AND GETTERS*/

// to be called when particle changes its type
void Particle::setArgs(ParticleType type, float vel_x, float vel_y, 
                        float pressure, float inertia, int temperature){
    this->type = type;
    this->vel_x = vel_x;
    this->vel_y = vel_y;
    this->pressure = pressure;
    this->inertia_x = inertia;
    this->inertia_y = inertia;
    this->temperature = temperature;
    color   = getColorByType(type);
    density = getDensityByType(type);
    state   = getStateByType(type);
}

void Particle::setCoord(float pos_x, float pos_y){
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}
ParticleType        Particle::getType(){
    return this->type;
};
ParticleState       Particle::getState(){
    return this->state;
};

int             Particle::getCooldown   (){
    return this->cooldown;
}
void            Particle::coolDownTick  (int frames){
    this->cooldown -= frames;
}

int             Particle::getDensity    (){
    return this->density;
}
int             Particle::getWetness    (){
    return this->wetness;
}
int             Particle::getTemperature(){
    return this->temperature;
}
void            Particle::setCooldown   (int newCooldown){
    this->cooldown = newCooldown;
}
void            Particle::setDensity    (int newDensity){
    this->density = newDensity;
}
void            Particle::setWetness    (int newWetness){
    this->wetness = newWetness;
}
void            Particle::setTemperature(int newTemperature){
    this->temperature = newTemperature;
}

void                Particle::setColor(std::vector<short> newColor){
    this->color = newColor;
};
std::vector<short>  Particle::getColor(){
    return this->color;
};


std::pair<float, float> Particle::getCoord(){
    return std::make_pair(std::floor(this->pos_x), std::floor(this->pos_y));

}