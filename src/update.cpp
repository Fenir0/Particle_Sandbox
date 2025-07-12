#include "../include/update.hpp"

// particle used grid is [1...GRID_DIMENSION-1]

struct Surroundings{
    int   above,   below,    left,   right;
    int diag_bl, diag_br, diag_al, diag_ar;
    bool to_move;

    Surroundings(int x, int y){
        above = y > 1               ? Update::getGridIndex(x, y - 1): -1;
        below = y < GRID_HEIGHT - 2 ? Update::getGridIndex(x, y + 1): -1;
        left  = x > 1               ? Update::getGridIndex(x - 1, y): -1;
        right = x < GRID_WIDTH  - 2 ? Update::getGridIndex(x + 1, y): -1;

        diag_bl = (below != -1 && left  != -1) ? left  + GRID_WIDTH : -1;
        diag_br = (below != -1 && right != -1) ? right + GRID_WIDTH : -1;
        diag_al = (above != -1 && left  != -1) ? left  - GRID_WIDTH : -1;
        diag_ar = (above != -1 && right != -1) ? right - GRID_WIDTH : -1;
        to_move = (above != -1) + (below != -1) + (left != -1) + (right != -1) +
                (diag_al != -1) + (diag_ar != -1) + (diag_bl != -1) + (diag_br != -1);

    }
};

void adjustForBorders(Particle& self){
    if(self.pos_x < 1) 
        self.pos_x = 1;
    if(self.pos_y < 1) 
        self.pos_y = 1;
    if(self.pos_x >= GRID_WIDTH  - 1) 
        self.pos_x = GRID_WIDTH - 1;
    if(self.pos_y >= GRID_HEIGHT  - 1) 
        self.pos_y = GRID_HEIGHT - 1;
}

void zeroOut(Particle& self){
    self.inertia_x = 0;
    self.inertia_y = 0;
    self.vel_x = 0;
    self.vel_y = 0;
}


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

void Update::update_SAND(Particle &self, std::vector<Particle> &grid, int i){       
    if (self.pos_x == -1.f && self.pos_y == -1.f) {
        self.pos_x = i % GRID_WIDTH;
        self.pos_y = i / GRID_WIDTH;
    }
    adjustForBorders(self);

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
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x);
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y);
    int new_pos = getGridIndex(new_x, new_y);

    // get the direction (stable if not moved/not possible to move)
    Movement mv = getMovement(new_pos, srd);

    // velocity not enough to change position (yet)
    if(mv == Movement::STABLE){ 
        self.vel_x = 0;
        self.vel_y = 0;
        self.pos_x += inertia_to_movement(self.inertia_x);
        self.pos_y += inertia_to_movement(self.inertia_y);
        
        // sand might collapse when under pressure
        if((srd.diag_bl >= 0 || srd.diag_br >= 0)){
            int r = rand() % 50 + 2;
            if(r < self.mass){
                //printf("%d\n", self.mass);
                self.inertia_x = 0;
                self.inertia_y = 0;
                r = rand()%2;
                if(r%2){
                    if(grid[srd.diag_bl].state != ParticleState::Solid && 
                                    grid[srd.left].type == ParticleType::Air){
                        swapper(self.pos_x, self.pos_y, 
                                self.pos_x-1, self.pos_y+1, grid);
                        return;
                    }
                }
                else{
                    if(grid[srd.diag_br].state != ParticleState::Solid && 
                                    grid[srd.right].type == ParticleType::Air){
                        swapper(self.pos_x, self.pos_y, 
                            self.pos_x+1, self.pos_y+1, grid);
                        return;
                    }
                }
            }
        }  
        return;
    }
    if (new_pos < 0 || new_pos >= grid.size()) {
        zeroOut(self);
        return;
    }
    switch (grid[new_pos].state)
        { // COLLIDING WITH GAS
        case ParticleState::Gas :
            self.vel_x = 0; 
            self.vel_y = 0; 
            grid[new_pos].setCoord(self.pos_x, self.pos_y);
            grid[old_pos].setCoord(new_x, new_y);
            std::swap(grid[old_pos], grid[new_pos]);
            return;

                // COLLIDING WITH FLUID 
        case ParticleState::Fluid :
            self.vel_x = 0;
            self.vel_y = 0;
            self.inertia_y *= 0.7f;
            self.inertia_x *= 0.7f;
            swapper(self.pos_x, self.pos_y, 
                         new_x,      new_y,    grid);
            return;

                // COLLIDING WITH SOLID
        case ParticleState::Solid:
            self.pos_x = std::floor(self.pos_x);
            self.pos_y = std::floor(self.pos_y);
            if(mv == Movement::BELOW){
                int next_move = rand()%3;
                if(next_move == 0 && new_x > 1 && new_y < GRID_HEIGHT - 1 && srd.diag_bl >= 0 && 
                            grid[srd.diag_bl].getState() != ParticleState::Solid){
                    self.inertia_x = - self.inertia_y / 3;
                    self.vel_y = 0;
                    self.inertia_y /= 3;
                    return;
                }
                else if(next_move == 1 && new_x < GRID_WIDTH - 2 && new_y < GRID_HEIGHT - 1 && srd.diag_br >= 0 && 
                            grid[srd.diag_br].getState() != ParticleState::Solid){
                    self.inertia_x = self.inertia_y / 2;
                    self.vel_y = 0;
                    self.inertia_y /= 2;
                    return;
                }
                zeroOut(self);
                return;
            }
            else if(mv == Movement::LEFT){
                grid[new_pos].vel_x += self.vel_x*0.5;
                self.inertia_x = -self.inertia_x * 0.5f;
                zeroOut(self);
                return;
            }
            else if(mv == Movement::RIGHT){
                grid[new_pos].vel_x += self.vel_x*0.5;
                self.inertia_x = -self.inertia_x * 0.5f;
                zeroOut(self);
                return;
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
                zeroOut(self);
                return;
            }
            else if(mv == Movement::DG_BR){
                if(grid[srd.below].state != ParticleState::Solid){
                    new_pos-=1;
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    zeroOut(self);
                    return;
                }
                else if(grid[srd.right].state != ParticleState::Solid){
                    new_pos -= GRID_WIDTH;
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    zeroOut(self);
                    return;
                }
                zeroOut(self);
                return;
            }
            zeroOut(self);
            return;
        }
        zeroOut(self);
        return;
}

void Update::update_STONE(Particle &self, std::vector<Particle> &grid, int i){
    return;
}
void Update::update_WATER(Particle &self, std::vector<Particle> &grid, int i){
    // current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    if (old_pos < 0 || old_pos >= grid.size()) {
        zeroOut(self);
        return;
    }
    
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));

    // Update velocity based on carried energy
    // (After impacts)
    transmitEnergy(srd, self, grid, i);

    // Applied forces
    getPressure(srd, self, grid, i);
    if(srd.below == -1){
        int r = rand() % 2;
        self.inertia_x += (self.mass-1) * pow(-1, r);
    }
    getGravity (srd, self, grid, i);
    getFriction(srd, self, grid, i);

    self.inertia_x += self.vel_x;
    self.inertia_y += self.vel_y;

    self.energy = (std::pow(self.inertia_x, 2) + std::pow(self.inertia_y, 2)) / 2.f;
    
    // next position
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x);
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y);

    int new_pos = getGridIndex(std::floor(new_x), std::floor(new_y));

    if(new_pos < 0 || new_pos >= grid.size()) 
        return;

    Movement mv = getMovement(new_pos, srd);

    if(srd.to_move == 0) {
        transferEnergy(srd, self, grid, i);
        self.energy = 0;
        zeroOut(self);
        return;
    }
        
// velocity not enough to change position (yet)
    if(mv == Movement::STABLE){ 
        self.pos_x += inertia_to_movement(self.inertia_x);
        self.pos_y += inertia_to_movement(self.inertia_y); 
        if(srd.below > -1 && grid[srd.below].state != ParticleState::Gas 
                    && (srd.diag_bl >= 0 || srd.diag_br >= 0)){
            int r = rand()%2;
            if(r%2){
                if(grid[srd.diag_bl].state != ParticleState::Fluid && 
                   grid[srd.left].type == ParticleType::Air){
                    swapper(self.pos_x, self.pos_y, 
                            self.pos_x-1, self.pos_y+1, grid);
                    return;
                }
            }
            else{
                if(grid[srd.diag_br].state != ParticleState::Fluid && 
                   grid[srd.right].type == ParticleType::Air){
                    swapper(self.pos_x, self.pos_y, 
                            self.pos_x+1, self.pos_y+1, grid);
                    return;
                }
            }
        }  
        if(srd.below > -1 && grid[srd.below].state != ParticleState::Fluid 
                    && (srd.left >= 0 || srd.right >= 0)){
            int r = rand()%2;
            if(r%2){
                if(grid[srd.diag_bl].state == ParticleState::Fluid && 
                   grid[srd.left   ].type  == ParticleType::Air){
                    swapper(self.pos_x, self.pos_y, 
                            self.pos_x-1, self.pos_y+1, grid);
                    return;
                }
            }
            else{
                if(grid[srd.diag_br].state == ParticleState::Fluid && 
                   grid[srd.right  ].type  == ParticleType::Air){
                    swapper(self.pos_x, self.pos_y, 
                            self.pos_x+1, self.pos_y+1, grid);
                    return;
                }
            }
        }  
        self.vel_x = 0;
        self.vel_y = 0; 
        return;
    }

    Particle& target = grid[new_pos];

    switch (target.state){ 
        // COLLIDING WITH GAS
    case ParticleState::Gas :
        self.vel_x = 0;
        self.vel_y = 0;
        swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
        return;

        // COLLIDING WITH FLUID 
    case ParticleState::Fluid: {
            target.vel_x += self.vel_x*0.6f;
            target.vel_y += self.vel_y*0.6f;
            target.energy += self.energy * 0.5f;
            self.energy *= 0.5f;
            self.vel_x = 0;
            self.vel_y = 0;
            self.inertia_x = 0;
            self.inertia_y = 0;
            return;
        }

            // COLLIDING WITH SOLID
    case ParticleState::Solid:
        self.vel_x = 0;
        self.vel_y = 0;
        self.inertia_x = 0;
        self.inertia_y = 0;
        target.energy += self.energy * 0.3f;
        self.energy *= 0.3f;
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

//
// @brief Apply the gravitational force
void Update::getGravity(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    if(self.pos_y >= GRID_HEIGHT - 1){
        self.pos_y = GRID_HEIGHT - 1;
        return;
    }
    if(self.state != ParticleState::Gas) {
        self.vel_y += Particle::GRAVITATIONAL_PULL;
    }
    if(sr.below != -1 && grid[sr.below].state == ParticleState::Solid)
        self.inertia_y = 0;
}

void Update::getFriction(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    switch (self.type)
    {
    case ParticleType::Sand:
        if(sr.below == -1){
            self.vel_x = - self.inertia_x * 0.3f;
            return;
        }
        if(sr.below > -1  && sr.below < grid.size()) {
            if(grid[sr.below].state == ParticleState::Solid){
                self.vel_x = - self.inertia_x * 0.3f;
            }
        }
        break;
    case ParticleType::Water:
        if(sr.below == -1){
            self.vel_x = - self.inertia_x * 0.3f;
            return;
        }
        if(sr.below > -1  && sr.below < grid.size()) {
            if(grid[sr.below].state == ParticleState::Solid){
                self.vel_x = - self.inertia_x * 0.3f;
            }
        }
        break;
    }
}

void Update::transferEnergy(Surroundings &sr, Particle &self, std::vector<Particle> &grid, int i)
{  
    if(sr.above > -1   && grid[sr.above].type == self.type) grid[sr.above].energy += self.energy*0.7f;
    if(sr.below > -1   && grid[sr.below].type == self.type) grid[sr.below].energy += self.energy*0.7f;
    if(sr.left  > -1   && grid[sr.left] .type == self.type) grid[sr.left] .energy += self.energy*0.7f;
    if(sr.right > -1   && grid[sr.right].type == self.type) grid[sr.right].energy += self.energy*0.7f;
}

void Update::transmitEnergy (Surroundings& srd, Particle& self, std::vector<Particle>& grid, int i){
    if(self.energy <= 0.01f) return; // no energy received :(

    // 1) on movement: kin E+++
    // 2) on impact: kin E => pot E
    // 3) pot E => speed for further movement
    // 4) pot E ---
    // 1) on movement: ...    

    if(self.inertia_x <= 0.05f && self.inertia_y <= 0.05f){ // 2nd step (or from standpoint) 
        self.inertia_x = 0;
        self.inertia_y = 0;
        self.energy *= 0.7f;

        // nowhere to move => just transfer it all

        if (srd.to_move == 0) {
            transferEnergy(srd, self, grid, i);
            self.energy = 0;
            return;
        }


        std::vector<int> next_move;
        next_move.reserve(8);
        if(srd.above > -1 && grid[srd.above].state == ParticleState::Gas) next_move.emplace_back(srd.above);
        if(srd.below > -1 && grid[srd.below].state == ParticleState::Gas) next_move.emplace_back(srd.below);
        if(srd.left  > -1 && grid[srd.left ].state == ParticleState::Gas) next_move.emplace_back(srd.left);
        if(srd.right > -1 && grid[srd.right].state == ParticleState::Gas) next_move.emplace_back(srd.right);
        if(srd.diag_al > -1 && grid[srd.diag_al].state == ParticleState::Gas) next_move.emplace_back(srd.diag_al);
        if(srd.diag_ar > -1 && grid[srd.diag_ar].state == ParticleState::Gas) next_move.emplace_back(srd.diag_ar);
        if(srd.diag_bl > -1 && grid[srd.diag_bl].state == ParticleState::Gas) next_move.emplace_back(srd.diag_bl);
        if(srd.diag_br > -1 && grid[srd.diag_br].state == ParticleState::Gas) next_move.emplace_back(srd.diag_br);
        if(next_move.empty()) return; // idk if possible, but helps

        std::shuffle(next_move.begin(), next_move.end(),
                         std::mt19937{std::random_device{}()});
        int chosen = next_move.front();
        if(chosen == srd.above)  self.inertia_x = 0, self.inertia_y = -sqrt(self.energy/4);
        if(chosen == srd.below)  self.inertia_x = 0, self.inertia_y = sqrt(self.energy);
        if(chosen == srd.left )  self.inertia_x = -sqrt(self.energy), self.inertia_y = 0;
        if(chosen == srd.right)  self.inertia_x =  sqrt(self.energy), self.inertia_y = 0;

        if(chosen == srd.diag_al) self.inertia_x = -sqrt(self.energy/8), self.inertia_y = -sqrt(self.energy/8);
        if(chosen == srd.diag_ar) self.inertia_x =  sqrt(self.energy/8), self.inertia_y = -sqrt(self.energy/8);
        if(chosen == srd.diag_bl) self.inertia_x = -sqrt(self.energy/8), self.inertia_y =  sqrt(self.energy/8);
        if(chosen == srd.diag_br) self.inertia_x =  sqrt(self.energy/8), self.inertia_y =  sqrt(self.energy/8);
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
    int ix = std::floor(x);
    int iy = std::floor(y);
    if (ix < 1 || iy < 1 || ix >= GRID_WIDTH - 1 || iy >= GRID_HEIGHT - 1){
      //  printf("WRONG COORD\n");
        return -1;
    }
    return ix + iy * GRID_WIDTH;
}