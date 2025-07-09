#include "../include/update.hpp"
struct Surroundings{
    int above, below, left, right;
    int diag_bl, diag_br, diag_al, diag_ar;
    bool to_move;
    Surroundings(int x, int y){
        above = y > 0               ? Update::getGridIndex(x, y - 1): -1;
        below = y < GRID_HEIGHT - 1 ? Update::getGridIndex(x, y + 1): -1;
        left  = x > 0               ? Update::getGridIndex(x - 1, y): -1;
        right = x < GRID_WIDTH  - 1 ? Update::getGridIndex(x + 1, y): -1;

        diag_bl = below>0 && x > 0            ? left  + GRID_WIDTH: -1;
        diag_br = below>0 && x < GRID_WIDTH-1 ? right + GRID_WIDTH: -1; 
        diag_al = above>0 && x > 0            ? left  - GRID_WIDTH: -1;
        diag_ar = above>0 && x < GRID_WIDTH-1 ? right + GRID_WIDTH: -1;
        to_move = above != -1 || below != -1 || left != -1 || right != -1 ||
                diag_al != -1 || diag_ar != -1 || diag_bl != -1 || diag_br != -1;

    }
};

float inertia_to_movement(float inertia) {
    if (inertia > 0.9f) return 0.9f;
    if (inertia < -0.9f) return -0.9f;
    if (std::abs(inertia) < 0.1f) return 0.0f; // dead zone
    return inertia;
}

enum Movement{STABLE, ABOVE, BELOW,  LEFT, RIGHT, 
                    DG_BL, DG_BR, DG_AL, DG_AR};
Movement getMovement(int new_pos, Surroundings& srd){
    if(new_pos == srd.above) return Movement::ABOVE;
    if(new_pos == srd.below) return Movement::BELOW;
    if(new_pos == srd.left) return Movement::LEFT;
    if(new_pos == srd.right) return Movement::RIGHT;

    if(new_pos == srd.diag_al) return Movement::DG_AL;
    if(new_pos == srd.diag_ar) return Movement::DG_AR;
    if(new_pos == srd.diag_bl) return Movement::DG_BL;
    if(new_pos == srd.diag_br) return Movement::DG_BR;
    return STABLE;
}

void Update::update_SAND(Particle &self, std::vector<Particle> &grid, int i)
{       

    if (self.pos_x == -1.f && self.pos_y == -1.f) {
        self.pos_x = i % GRID_WIDTH;
        self.pos_y = i / GRID_WIDTH;
    }

    // current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));
    // Applied forces
    getPressure(srd, self, grid, i);
    getGravity (srd, self, grid, i);
    getFriction(srd, self, grid, i);

    self.inertia_y += self.vel_y; 
    self.inertia_x += self.vel_x;

    // next position
    float new_x = self.pos_x+inertia_to_movement(self.inertia_x);
    float new_y = self.pos_y+inertia_to_movement(self.inertia_y);
    int new_pos = getGridIndex(new_x, new_y);
    Movement mv = getMovement(new_pos, srd);

    // velocity not enough to change position (yet)
    if(mv == Movement::STABLE){ 
        self.vel_x = 0;
        self.vel_y = 0;
        self.pos_x+=inertia_to_movement(self.inertia_x);
        self.pos_y+=inertia_to_movement(self.inertia_y);
        
        // sand might collapse when under pressure
        if((srd.diag_bl >= 0 || srd.diag_br >= 0)){
            int r = rand() % 6 + 1;
            if(r < self.mass){
                r = rand()%2;
                if(r%2){
                    if(grid[srd.diag_bl].state != ParticleState::Solid && 
                                    grid[srd.left].type == ParticleType::Air){
                        grid[srd.diag_bl].setCoord(self.pos_x, self.pos_y);
                        grid[old_pos].setCoord(self.pos_x-1, self.pos_y+1);
                        std::swap(grid[old_pos], grid[srd.diag_bl]);
                    }
                }
                else{
                    if(grid[srd.diag_br].state != ParticleState::Solid && 
                                    grid[srd.right].type == ParticleType::Air){
                        grid[srd.diag_br].setCoord(self.pos_x, self.pos_y);
                        grid[old_pos].setCoord(self.pos_x+1, self.pos_y+1);
                        std::swap(grid[old_pos], grid[srd.diag_br]);
                    }
                }
            }
        }  
        return;
    }
    
    switch (grid[new_pos].state)
        { // COLLIDING WITH GAS
        case ParticleState::Gas :
            grid[new_pos].setCoord(self.pos_x, self.pos_y);
            grid[old_pos].setCoord(new_x, new_y);
            std::swap(grid[old_pos], grid[new_pos]);
            return;

                // COLLIDING WITH FLUID 
        case ParticleState::Fluid :
            self.vel_x *= 0.7;
            self.vel_y *= 0.7;
            swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
            return;

                // COLLIDING WITH SOLID
        case ParticleState::Solid:
            if(mv == Movement::ABOVE){
                grid[new_pos].vel_y += self.vel_y*0.5f;
                self.vel_y = -0.9f * self.vel_y;
                return;
            }
            else if(mv == Movement::BELOW){
                self.inertia_x = 0;
                self.inertia_y = 0;
                int next_move = rand()%5;

                grid[new_pos].vel_y += self.vel_y*0.5f;
                self.vel_y *= 0.5f;

                if(next_move == 0 && new_x > 0 && new_y < GRID_HEIGHT && srd.diag_bl >= 0 && 
                            grid[srd.diag_bl].getState() != ParticleState::Solid){

                    self.vel_x = -0.1f;
                    self.vel_y = Particle::GRAVITATIONAL_PULL;
                    swapper(self.pos_x,     self.pos_y, 
                            self.pos_x-1.f, self.pos_y+1.f, grid);
                    return;
                }
                else if(next_move == 1 && new_x < GRID_WIDTH - 1 && new_y < GRID_HEIGHT && srd.diag_br >= 0 && 
                            grid[srd.diag_br].getState() != ParticleState::Solid){

                    self.vel_x = 0.1f;
                    self.vel_y = Particle::GRAVITATIONAL_PULL;
                    swapper(self.pos_x,     self.pos_y, 
                            self.pos_x+1.f, self.pos_y+1.f, grid);
                    return;
                }
                self.vel_y *= 0.2f;
                return;
            }
            else if(mv == Movement::LEFT){
                grid[new_pos].vel_x += self.vel_x*0.5;
                self.inertia_x = -self.inertia_x * 0.5f;
                self.vel_x = 0;
            }
            else if(mv == Movement::RIGHT){
                grid[new_pos].vel_x += self.vel_x*0.5;
                self.inertia_x = -self.inertia_x * 0.5f;
                self.vel_x = 0;
            }
            else if(mv == Movement::DG_BL){
                if(grid[srd.below].state != ParticleState::Solid){
                    new_pos+=1;
                }
                else if(grid[srd.left].state != ParticleState::Solid){
                    new_pos -= GRID_WIDTH;
                }
                swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
            }
            else if(mv == Movement::DG_BR){
                if(grid[srd.below].state != ParticleState::Solid){
                    new_pos-=1;
                    
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    std::swap(grid[old_pos], grid[new_pos]);
                }
                else if(grid[srd.right].state != ParticleState::Solid){
                    new_pos -= GRID_WIDTH;
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    std::swap(grid[old_pos], grid[new_pos]);
                }
            }
        }
        return;
}

void Update::update_WATER(Particle &self, std::vector<Particle> &grid, int i){
    // current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));

    transmitEnergy(srd, self, grid, i);

    // Applied forces
    getPressure(srd, self, grid, i);
    getGravity (srd, self, grid, i);
    getFriction(srd, self, grid, i);

    // carry the energy for potential impact and transer
    self.inertia_x += self.vel_x;
    self.inertia_y += self.vel_y;
    self.energy = (std::pow(self.inertia_x, 2) + std::pow(self.inertia_y, 2)) / 2.f;
    
    // next position
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x);
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y);
    int new_pos = getGridIndex(new_x, new_y);
    Movement mv = getMovement(new_pos, srd);

    if(grid[srd.below].state == ParticleState::Fluid && 
        grid[srd.left].state == ParticleState::Fluid && 
        grid[srd.right].state == ParticleState::Fluid)
            transferEnergy(srd, self, grid, i);

    // velocity not enough to change position (yet)
    if(mv == Movement::STABLE){ 
        self.pos_x += inertia_to_movement(self.inertia_x);
        self.pos_y += inertia_to_movement(self.inertia_y);   
        self.vel_x = 0;
        self.vel_y = 0; 
        return;
    }

    if(new_pos < 0 || new_pos >= grid.size()) return;

    Particle& target = grid[new_pos];

    switch (target.state){ 
        // COLLIDING WITH GAS
    case ParticleState::Gas :
        swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
        self.vel_x = 0;
        self.vel_y = 0;
        return;

        // COLLIDING WITH FLUID 
    case ParticleState::Fluid: {
            if (self.energy > target.energy + 0.01f) {
                swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
                self.energy *= 0.5f;
                target.energy += self.energy;
                self.vel_x = 0;
                self.vel_y = 0;
                return;
            }
            self.vel_x = 0;
            self.vel_y = 0;
            self.energy *= 0.7f;
            return;
        }

            // COLLIDING WITH SOLID
    case ParticleState::Solid:
        self.vel_x = 0;
        self.vel_y = 0;
        self.inertia_x = 0;
        self.inertia_y = 0;
        self.energy *= 0.7f;
    }
}
//*/
/*
    FORCE CHECKERS
*/

void Update::getPressure(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    if(self.state != ParticleState::Gas){
        if(sr.above > -1 && grid[sr.above].state != ParticleState::Gas) self.mass += grid[sr.above].mass;
        else self.mass = 1;
    }
}
void Update::getGravity(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    if(self.pos_y >= GRID_HEIGHT - 1){
        self.pos_y = GRID_HEIGHT - 1;
        self.vel_y = 0;
        return;
    }
    if(self.state != ParticleState::Gas) {
        if(grid[sr.below].state != ParticleState::Solid){
            self.vel_y += self.GRAVITATIONAL_PULL;
        }
        else{
            self.vel_y = 0;
        }
    }
}
void Update::getFriction(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    switch (self.type)
    {
    case ParticleType::Sand:
        if(sr.below == -1){
            self.vel_x *= 0.3f;
            return;
        }
        if(sr.below > -1  && sr.below < grid.size()) {
            if(grid[sr.below].state == ParticleState::Solid){
                self.vel_x *= 0.3f;
            }
        }
        break;
    case ParticleType::Water:
        if(sr.below == -1){
            self.vel_x *= 0.5f;
            return;
        }
        if(sr.below > -1  && sr.below < grid.size()) {
            if(grid[sr.below].state == ParticleState::Solid){
                self.vel_x *= 0.9f;
            }
        }
        break;
    }
}

void Update::transferEnergy(Surroundings &sr, Particle &self, std::vector<Particle> &grid, int i)
{  
    if(sr.above > -1   && grid[sr.above].type == self.type) grid[sr.above].energy += self.energy*0.95f;
    if(sr.below > -1   && grid[sr.below].type == self.type) grid[sr.below].energy += self.energy*0.95f;
    if(sr.left > -1    && grid[sr.left] .type == self.type) grid[sr.left] .energy += self.energy*0.95f;
    if(sr.right > -1   && grid[sr.right].type == self.type) grid[sr.right].energy += self.energy*0.95f;

    //if(sr.diag_al > -1 && grid[sr.diag_al].type == self.type) grid[sr.diag_al].vel_y += self.vel_y*0.5f;
  //  if(sr.diag_ar > -1 && grid[sr.diag_ar].type == self.type) grid[sr.diag_ar].vel_y += self.vel_y*0.5f;
   // if(sr.diag_bl > -1 && grid[sr.diag_bl].type == self.type) grid[sr.diag_bl].vel_y += self.vel_y*0.5f;
   // if(sr.diag_br > -1 && grid[sr.diag_br].type == self.type) grid[sr.diag_br].vel_y += self.vel_y*0.5f;
}

void Update::transmitEnergy (Surroundings& srd, Particle& self, std::vector<Particle>& grid, int i){
    if(self.energy == 0) return; // no energy received :(

    // considerable points: kin E = 0 and pot E = 0
    // 1) on movement: kin E+++
    // 2) on impact: kin E => pot E
    // 3) pot E => speed for further movement
    // 4) pot E ---
    // 1) on movement: ...    

    if(self.vel_x == 0 && self.vel_y == 0){ // 2nd step (or from standpoint) 
        self.inertia_x = 0;
        self.inertia_y = 0;
        self.energy *= 0.7f;
        // external energy (not via movement of itself)
        if(!srd.to_move) return; // nowhere to move => just transfer it all
        std::vector<int> next_move;
        next_move.reserve(7);
        if(srd.below > -1 && grid[srd.below].state == ParticleState::Gas) {
            self.vel_x = 0, self.vel_y = std::min( 0.9,  sqrt(self.energy));
            return;
        }
        if(srd.above > -1 && grid[srd.above].state == ParticleState::Gas) next_move.emplace_back(srd.above);
        if(srd.left  > -1 && grid[srd.left ].state == ParticleState::Gas) next_move.emplace_back(srd.left);
        if(srd.right > -1 && grid[srd.right].state == ParticleState::Gas) next_move.emplace_back(srd.right);
        if(srd.diag_al > -1 && grid[srd.diag_al].state == ParticleState::Gas) next_move.emplace_back(srd.diag_al);
        if(srd.diag_ar > -1 && grid[srd.diag_ar].state == ParticleState::Gas) next_move.emplace_back(srd.diag_ar);
        if(srd.diag_bl > -1 && grid[srd.diag_bl].state == ParticleState::Gas) next_move.emplace_back(srd.diag_bl);
        if(srd.diag_br > -1 && grid[srd.diag_br].state == ParticleState::Gas) next_move.emplace_back(srd.diag_br);
        if(next_move.empty()) return; // idk if possible, but helps
        std::random_shuffle(next_move.begin(), next_move.end());
        int chosen = next_move.front();
        if(chosen == srd.above)  self.vel_x = 0, self.vel_y = std::max(-0.9, -sqrt(self.energy));
        if(chosen == srd.left )  self.vel_x = std::max(-0.9, -sqrt(self.energy)), self.vel_y = 0;
        if(chosen == srd.right)  self.vel_x = std::min( 0.9,  sqrt(self.energy)), self.vel_y = 0;

        if(chosen == srd.diag_al) self.vel_x = std::max(-0.9, -sqrt(self.energy/2)), self.vel_y = std::max(-0.9, -sqrt(self.energy/2));
        if(chosen == srd.diag_ar) self.vel_x = std::min( 0.9,  sqrt(self.energy/2)), self.vel_y = std::max(-0.9, -sqrt(self.energy/2));
        if(chosen == srd.diag_bl) self.vel_x = std::max(-0.9, -sqrt(self.energy/2)), self.vel_y = std::min( 0.9,  sqrt(self.energy/2));
        if(chosen == srd.diag_br) self.vel_x = std::min( 0.9,  sqrt(self.energy/2)), self.vel_y = std::min( 0.9,  sqrt(self.energy/2));
        self.energy = 0;
    }
}
// @brief Swap particles [pos_x, pos_y] and [new_x, new_y]
void Update::swapper(float pos_x, float pos_y, float new_x, float new_y, std::vector<Particle>& grid){
    int old_pos = getGridIndex(pos_x, pos_y);
    int new_pos = getGridIndex(new_x, new_y);
    grid[new_pos].setCoord(pos_x, pos_y);
    grid[old_pos].setCoord(new_x, new_y);
    std::swap(grid[old_pos], grid[new_pos]);
}

int Update::getGridIndex(float x, float y) {
    return std::floor(x) + std::floor(y)*GRID_WIDTH;
    return 0;
}