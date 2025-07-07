#include "../include/particle.hpp"

int idc = 0;
int GRID_WIDTH = 0;
int GRID_HEIGHT = 0;

Particle::Particle(ParticleType type){
    this->type = type;
    this->id = idc++;
    vel_x = -1;
    vel_y = -1;

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
    return {0, 0, 0};
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
    return ParticleState::Gas;
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


void Particle::Update(std::vector<Particle>& grid, int i){
            /*
            | int   |     float   |
            old_pos | pos_x, pos_y = current position
            new_pos | new_x, new_y = new position after 1 tick

            if old_pos != new_pos => change visible position 
            swap(grid[old_pos], grid[new_pos])
            */
    
    /*
        OLD_POS + CURRENT VELOCITY + FRICTION + GRAVITY => NEW_POS 
    */

    /*
    if(state == ParticleState::Solid) {
        printf("%f %f: %d\n", pos_x, pos_y, id); // troubleshooting purposes
    }
    //*/

    if (pos_x == -1.f && pos_y == -1.f) {
        pos_x = i % GRID_WIDTH;
        pos_y = i / GRID_WIDTH;
    }

    // current position and surroundings
    int old_pos = std::floor(pos_x) + std::floor(pos_y) * GRID_WIDTH;

    int above = pos_y > 0               ? std::floor(pos_x) + (std::floor(pos_y)-1) * GRID_WIDTH: -1;
    int below = pos_y < GRID_HEIGHT - 1 ? std::floor(pos_x) + (std::floor(pos_y)+1) * GRID_WIDTH: -1;
    int left  = pos_x > 0               ? std::floor(pos_x - 1) + std::floor(pos_y) * GRID_WIDTH: -1;
    int right = pos_x < GRID_WIDTH  - 1 ? std::floor(pos_x + 1) + std::floor(pos_y) * GRID_WIDTH: -1;
    
    int diag_below_left  = below>0 && pos_x > 0            ? left  + GRID_WIDTH: -1;
    int diag_below_right = below>0 && pos_x < GRID_WIDTH-1 ? right + GRID_WIDTH: -1; 
    int diag_above_left  = above>0 && pos_x > 0            ? left  - GRID_WIDTH: -1;
    int diag_above_right = above>0 && pos_x < GRID_WIDTH-1 ? right + GRID_WIDTH: -1; 

    // on the floor 
    if(pos_y >= GRID_HEIGHT - 1){
        pos_y = GRID_HEIGHT - 1;
        vel_y = 0;
        vel_x *= 0.3;
    }
    // gravity 
    else if(state != ParticleState::Gas) {
        if(grid[below].state == ParticleState::Gas){
            vel_y += 0.5f;
        }
    }
    // friction (currently only from below)
    if(below < grid.size()) {
        if(grid[below].state == ParticleState::Solid){
            vel_x *= 0.3f;
        }
    }

    // movement of maximum of 1 tile per 1 tick
    vel_x = vel_x >= 0 ?std::min(vel_x, 0.99f):std::max(vel_x, -0.99f);
    vel_y = vel_y >= 0 ?std::min(vel_y, 0.99f):std::max(vel_y, -0.99f);
    
    // next position
    float new_x = pos_x+vel_x;
    float new_y = pos_y+vel_y;
    int new_pos = std::floor(new_x) + std::floor(new_y) * GRID_WIDTH;

    bool NEW_ABOVE = new_pos == above;
    bool NEW_BELOW = new_pos == below;
    bool NEW_LEFT  = new_pos == left;
    bool NEW_RIGHT = new_pos == right;

    bool NEW_DGBW_LEFT  = new_pos == diag_below_left;
    bool NEW_DGBW_RIGHT = new_pos == diag_below_right;
    bool NEW_DGAV_LEFT  = new_pos == diag_above_left;
    bool NEW_DGAV_RIGHT = new_pos == diag_above_right;
             
    // velocity not enough to change position (yet)
    if(new_pos == old_pos){ 

        // only significant movements to avoid weird behaviour
        if(std::abs(vel_x) > 0.05f) pos_x += vel_x;
        else vel_x *= 0.95f;
        if(std::abs(vel_y) > 0.05f) pos_y += vel_y;
        else vel_y *= 0.95f;

        return;
    }

    // FOR FALLING SAND CURRENTLY
    // later will be separated somehow
    switch (grid[new_pos].state)
    {
            // COLLIDING WITH GAS
    case ParticleState::Gas :
        grid[new_pos].setCoord(pos_x, pos_y);
        grid[old_pos].setCoord(new_x, new_y);
        std::swap(grid[old_pos], grid[new_pos]);
        if(NEW_ABOVE){
            vel_y -= 0.3f;
            return;
        }
        return;

            // COLLIDING WITH FLUID 
    case ParticleState::Fluid :
        grid[new_pos].setCoord(pos_x, pos_y);  
        grid[old_pos].setCoord(new_x, new_y);
        vel_x *= 0.7;
        vel_y *= 0.7;
        std::swap(grid[old_pos], grid[new_pos]);
        return;

            // COLLIDING WITH SOLID
    case ParticleState::Solid:
        if(NEW_ABOVE){
            grid[new_pos].vel_y += vel_y*0.5f;
            vel_y = -0.9f * vel_y;
            return;
        }
        else if(NEW_BELOW){
            int next_move = rand()%3;

            grid[new_pos].vel_y += vel_y*0.5f;
            vel_y *= 0.5f;

            if(next_move == 0 && new_x > 0 && new_y < GRID_HEIGHT && diag_below_left >= 0 && 
                        grid[diag_below_left].getState() != ParticleState::Solid){

                vel_x = -0.1f;
                vel_y = 0.5f;
                grid[diag_below_left].setCoord(std::floor(pos_x), std::floor(pos_y));
                grid[old_pos].setCoord(std::floor(pos_x)-1.f, std::floor(pos_y)+1);
                std::swap(grid[old_pos], grid[diag_below_left]);
                return;
            }
            else if(next_move == 1 && new_x < GRID_WIDTH - 1 && new_y < GRID_HEIGHT && diag_below_right >= 0 && 
                        grid[diag_below_right].getState() != ParticleState::Solid){

                vel_x = 0.1f;
                vel_y = 0.5f;
                grid[diag_below_right].setCoord(pos_x, pos_y);
                grid[old_pos].setCoord(std::floor(pos_x)+1.f, std::floor(pos_y)+1);
                std::swap(grid[old_pos], grid[diag_below_right]);
                return;
            }
            vel_y *= 0.2f;
            return;
        }
        else if(NEW_LEFT){
            grid[new_pos].vel_x += vel_x*0.5;
            vel_x = 0;
        }
        else if(NEW_RIGHT){
            grid[new_pos].vel_x += vel_x*0.5;
            vel_x = 0;
        }
        else if(NEW_DGBW_LEFT){
            if(grid[below].state != ParticleState::Solid){
                new_pos+=1;
            }
            else if(grid[left].state != ParticleState::Solid){
                new_pos -= GRID_WIDTH;
            }
            grid[new_pos].setCoord(pos_x, pos_y);
            grid[old_pos].setCoord(new_x, new_y);
            std::swap(grid[old_pos], grid[new_pos]);
        }
        else if(NEW_DGBW_RIGHT){
            if(grid[below].state != ParticleState::Solid){
                new_pos-=1;
                grid[new_pos].setCoord(pos_x, pos_y);
                grid[old_pos].setCoord(new_x, new_y);
                std::swap(grid[old_pos], grid[new_pos]);
            }
            else if(grid[right].state != ParticleState::Solid){
                new_pos -= GRID_WIDTH;
                grid[new_pos].setCoord(pos_x, pos_y);
                grid[old_pos].setCoord(new_x, new_y);
                std::swap(grid[old_pos], grid[new_pos]);
            }
        }
        else if(NEW_DGAV_LEFT){

        }
        else if(NEW_DGAV_RIGHT){
            
        }
    }
}