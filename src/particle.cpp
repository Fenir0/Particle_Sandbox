#include "../include/particle.hpp"

int idc = 0; // for debugging separate particles 
int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;

Particle::Particle(ParticleType type){
    this->type = type;
    this->id = idc++;
    vel_x = 0;
    vel_y = 0;

    state = getStateByType(type);
    color = getColorByType(type);
}

std::vector<short> Particle::getColorByType(ParticleType type){
    switch (type)
    {
    case ParticleType::Air:
        return {217, 217, 217};
    case ParticleType::Sand:
        return {230, 215, 160};
    case ParticleType::Water:
        return {102, 143, 220};
    case ParticleType::Stone:
        return {111, 118, 131};
    }
}
ParticleState      Particle::getStateByType(ParticleType type){
    switch (type)
    {
    case ParticleType::Air:
        return ParticleState::Gas;
    case ParticleType::Sand:
        return ParticleState::Solid;
    case ParticleType::Water:
        return ParticleState::Fluid;
    case ParticleType::Stone:
        return ParticleState::Solid;
    }
}

void Particle::setCoord(float pos_x, float pos_y){
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}
void Particle::setArgs(ParticleType type, ParticleState state, 
                        std::vector<short> color, float vel_x, float vel_y){
    this->type = type;
    this->state = state;
    this->color = color;
    this->vel_x = vel_x;
    this->vel_y = vel_y;
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
std::pair<int, int> Particle::getCoord(){
    return std::make_pair(std::floor(this->pos_x), std::floor(this->pos_y));

}

void Particle::Update(std::vector<Particle>& grid){
/*
old_pos | pos_x, pos_y = current position
new_pos | new_x, new_y = new position
If can be rounded to other coord => change visible position
*/

    if(state == ParticleState::Solid) {
        printf("%f %f: %d\n", pos_x, pos_y, id);
    }
    int old_pos = std::floor(pos_x) + std::floor(pos_y) * GRID_WIDTH;

    float new_x = pos_x+vel_x;
    float new_y = pos_y+vel_y;
    int new_pos = std::floor(new_x) + std::floor(new_y) * GRID_WIDTH;
    
    if(pos_y >= GRID_HEIGHT - 1){
        pos_y = GRID_HEIGHT - 1;
        vel_y = 0;
        return;
    }
    // NOT MOVED
    //    F=0 or not enough to move yet
    if(new_pos == old_pos){ 
        pos_x += vel_x;
        pos_y += vel_y;
        /* idk for now, probably nothing
        if(state == ParticleState::Solid){
            if(n_pos_y < _HEIGHT-2 && grid[new_pos + _WIDTH].state  == ParticleState::Gas){
                vel_y += 0.3;
            }
            else if(pos_y < _HEIGHT-2 && grid[new_pos + _WIDTH].state == ParticleState::Solid){
                new_pos = new_x + (new_y-1)*_WIDTH;
                grid[new_pos].vel_y += vel_y*0.5;
                vel_y *= 0.3;
            }
        }
        */
    }
    else{
        switch (grid[new_pos].state)
        {
                case ParticleState::Gas :
                    pos_x += vel_x;
                    pos_y += vel_y;
                    vel_y+=0.01;
                    std::swap(grid[old_pos], grid[new_pos]);
                    break;
                case ParticleState::Fluid :
                    pos_x += vel_x;
                    pos_y += vel_y;
                    vel_y *= 0.5;
                    std::swap(grid[old_pos], grid[new_pos]);
                    break;
                case ParticleState::Solid :
                    grid[new_pos].vel_y += vel_y*0.5;
                    vel_y *= 0.5;
                    break;
                    
        }

    }
    vel_x = std::min(vel_x, 1.0f);
    vel_y = std::min(vel_y, 0.8f);
}