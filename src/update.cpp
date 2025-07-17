#include "../include/update.hpp"

// particle used grid is [1...GRID_DIMENSION-1]
bool Update::freezeSelect = false;

struct Surroundings{
    int   above,   below,     left,   right;
    int diag_bl, diag_br, diag_al, diag_ar;
    int to_move;

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

bool isGasOrNone(std::vector<Particle>& grid, int pos){
    return (pos >= -1) && ((grid[pos].state == ParticleState::Gas) || (grid[pos].state == ParticleState::None));
}
bool isSolidOrFluid(std::vector<Particle>& grid, int pos){
    return (pos >= -1) && ((grid[pos].state == ParticleState::Solid) || (grid[pos].state == ParticleState::Fluid));
}

void adjustForBorders(Particle& self){
    if (self.pos_x < 1) 
        self.pos_x = 1;
    if (self.pos_y < 1) 
        self.pos_y = 1;
    if (self.pos_x > GRID_WIDTH  - 1) 
        self.pos_x = GRID_WIDTH  - 1;
    if (self.pos_y > GRID_HEIGHT - 1) 
        self.pos_y = GRID_HEIGHT - 1;
}

void zeroOut(Particle& self){
    self.inertia_x = 0;
    self.inertia_y = 0;
    self.vel_x     = 0;
    self.vel_y     = 0;
}

// AMOUNT OF TILES MOVED PER TURN
// 0.1 ... 0.9
float simulationSpeed = 0.5f;

//
// @brief Clamp to 1 tile per 1 frame
float inertia_to_movement(float inertia) {
    if (inertia > simulationSpeed) return simulationSpeed;
    if (inertia < -simulationSpeed) return -simulationSpeed;
    if (std::abs(inertia) < 0.1f) return 0.0f; // dead zone
    return inertia;
}

enum Movement{STABLE, ABOVE, BELOW,  LEFT, RIGHT, 
                      DG_BL, DG_BR, DG_AL, DG_AR};
Movement getMovement(int new_pos, Surroundings& srd){
    if(new_pos == srd.above)   return Movement::ABOVE;
    if(new_pos == srd.below)   return Movement::BELOW;
    if(new_pos == srd.left )   return Movement::LEFT;
    if(new_pos == srd.right)   return Movement::RIGHT;

    if(new_pos == srd.diag_al) return Movement::DG_AL;
    if(new_pos == srd.diag_ar) return Movement::DG_AR;
    if(new_pos == srd.diag_bl) return Movement::DG_BL;
    if(new_pos == srd.diag_br) return Movement::DG_BR;
    return STABLE;
}

void Update::update_SAND(Particle &self, std::vector<Particle> &grid, int i){ 
    // In case the old position is out of borders
    adjustForBorders(self);

    // Current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);

    // Coordinates of surrounding tiles
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));

    // Applied forces
    getPressure(srd, self, grid, i);
    getGravity (srd, self, grid, i);
    getFriction(srd, self, grid, i);

    self.inertia_x += self.vel_x;
    self.inertia_y += self.vel_y; 


    // next position                                             // some noice for less idealistic moves
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x) + self.vel_x*(rand()%3)/5.f;
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y) + self.vel_y*(rand()%3)/5.f;
    int new_pos = getGridIndex(new_x, new_y);

    self.vel_x *= 0.7f;
    // Get the direction (stable if not moved/not possible to move)
    Movement mv = getMovement(new_pos, srd);

    // velocity not enough to change position (yet)
    if(new_pos > -1 && mv == Movement::STABLE){ 
        self.vel_x = 0;
        self.vel_y = 0;
        self.pos_x += inertia_to_movement(self.inertia_x);
        self.pos_y += inertia_to_movement(self.inertia_y);
        
        // sand might collapse when under pressure
        if((srd.diag_bl >= 0 || srd.diag_br >= 0) && srd.below >= 0 && grid[srd.below].state == ParticleState::Solid){
            int r = rand() % 2 + 2;
            if(r < self.pressure){
                //printf("%d\n", self.pressure);
                self.inertia_x = 0;
                self.inertia_y = 0;
                r = rand()%2;
                if(r%2){
                    if(grid[srd.diag_bl].state != ParticleState::Solid && 
                                    grid[srd.left].state != ParticleState::Solid){
                        swapper(self.pos_x, self.pos_y, 
                                self.pos_x-1, self.pos_y+1, grid);
                        return;
                    }
                }
                else{
                    if(grid[srd.diag_br].state != ParticleState::Solid && 
                                    grid[srd.right].state != ParticleState::Solid){
                        swapper(self.pos_x, self.pos_y, 
                                self.pos_x+1, self.pos_y+1, grid);
                        return;
                    }
                }
            }
        }  
        return;
    }
    if (new_pos < 0) {
        zeroOut(self);
        return;
    }
    switch (grid[new_pos].state)
        { // COLLIDING WITH GAS OR NONE
        case ParticleState::None :
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
                if(next_move == 0 && new_x > 1 && new_y < GRID_HEIGHT - 1 && 
                    srd.diag_bl >= 0 && grid[srd.diag_bl].state != ParticleState::Solid){
                    self.inertia_x = - 0.01f;
                    self.vel_y = 0;
                    self.inertia_y /= 3;
                    return;
                }
                else if(next_move == 1 && new_x < GRID_WIDTH - 2 && new_y < GRID_HEIGHT - 1 && 
                            srd.diag_br >= 0 && grid[srd.diag_br].state != ParticleState::Solid){
                    self.inertia_x = 0.01f;
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

void Update::update_SMOKE(Particle &self, std::vector<Particle> &grid, int i){ 
    // In case the old position is out of borders
    adjustForBorders(self);

    // Current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    // Coordinates of surrounding tiles
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));
    std::vector<int> possibleMoves;
    possibleMoves.reserve(10);

    float new_x = self.pos_x;
    float new_y = self.pos_y;

    /*
        will be refactored later
    */

    // Gas always moves up if possible (be it up, up-left, up-right)
    //                      looks awful, yeah           but works
    if(srd.above > -1 && grid[srd.above].state == ParticleState::None){
        possibleMoves.emplace_back(0);
    }
    if(srd.diag_al > -1 && grid[srd.diag_al].state == ParticleState::None 
                        && grid[srd.left].state == ParticleState::None){
        possibleMoves.emplace_back(1);
    }
    if(srd.diag_ar > -1 && grid[srd.diag_ar].state == ParticleState::None 
                        && grid[srd.right].state == ParticleState::None){
        possibleMoves.emplace_back(2);
    }
    if(!possibleMoves.empty()){
        int r = rand()%possibleMoves.size();
        new_y--;
        if(possibleMoves[r] == 1) new_x--;
        if(possibleMoves[r] == 2) new_x++;
        if(getGridIndex(new_x, new_y) != -1){
            swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
            return;
        }
    }
    new_x = self.pos_x;
    new_y = self.pos_y;
    // if up is impossible, move left or right
    possibleMoves.clear();
    int r = getClosestMove(srd, self, grid, i, self.state);
    if(r == 1){
        if(getGridIndex(self.pos_x - 1, self.pos_y) != -1
        && grid[getGridIndex(self.pos_x - 1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(getGridIndex(self.pos_x + 1, self.pos_y) != -1 
        && grid[getGridIndex(self.pos_x + 1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    possibleMoves.clear();
    //return;
    // if not moved, just swap with same particles to simulate chaos
    try{
    if(srd.above > -1 && grid[srd.above].state == self.state){
        possibleMoves.emplace_back(0);
    }
    if(isGasOrNone(grid, srd.left)){
        possibleMoves.emplace_back(1);
    }
    if(isGasOrNone(grid, srd.right)){
        possibleMoves.emplace_back(2);
    }
    if(isGasOrNone(grid, srd.below)){
        possibleMoves.emplace_back(3);
    }
    if(possibleMoves.size() > 3){
        int r = rand()%possibleMoves.size();
        switch (r)
        {
        case 0: new_y--; break;
        case 1: new_x--; break;
        case 2: new_x++; break;
        case 3: new_y++; break;      
        default:
            break;
        }
        swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
        return;
    }
    }
    catch(std::exception err){
        int r;

    }
}

void Update::update_STONE(Particle &self, std::vector<Particle> &grid, int i){
    return;
}

void Update::update_WATER(Particle &self, std::vector<Particle> &grid, int i){
    // In case the old position is out of borders
    adjustForBorders(self);
    // Current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    // Coordinates of surrounding tiles
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));

    getPressure(srd, self, grid, i);

    std::vector<int> possibleMoves;
    possibleMoves.reserve(10);

    float new_x = self.pos_x;
    float new_y = self.pos_y;

    if(isGasOrNone(grid, srd.below)){
        possibleMoves.emplace_back(0);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_bl)){
        possibleMoves.emplace_back(1);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_br)){
        possibleMoves.emplace_back(2);
    }
    if(!possibleMoves.empty()){
        int r = rand()%possibleMoves.size();
        new_y++;
        if(possibleMoves[r] == 1) new_x--;
        if(possibleMoves[r] == 2) new_x++;
        if(getGridIndex(new_x, new_y) != -1){
            swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
            return;
        }
    }

    // if down is impossible, move left or right
    int r = getClosestMove(srd, self, grid, i, self.state);
    if(r == 1){
        if(getGridIndex(self.pos_x - 1, self.pos_y) != -1 
        && grid[getGridIndex(self.pos_x - 1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(getGridIndex(self.pos_x + 1, self.pos_y) != -1 
        && grid[getGridIndex(self.pos_x + 1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    if(getGridIndex(self.pos_x - 1, self.pos_y) == -1 
        && grid[getGridIndex(self.pos_x + 1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    if(getGridIndex(self.pos_x + 1, self.pos_y) == -1 
        && grid[getGridIndex(self.pos_x +-1, self.pos_y)].state == ParticleState::None){
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    if(isSolidOrFluid(grid, srd.above)) grid[srd.above].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.left )) grid[srd.left ].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.right)) grid[srd.right].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.below)) grid[srd.below].pressure = self.pressure;
    
}
//*/
/*
    FORCE CHECKERS
*/

//
//@brief Get the pressure of particles above
void Update::getPressure(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    if(self.state != ParticleState::None && self.state != ParticleState::Gas){
        if(sr.above > -1 && grid[sr.above].state != ParticleState::None 
                         && grid[sr.above].state != ParticleState::Gas
                         && grid[sr.above].type != ParticleType::Stone) 
            self.pressure += grid[sr.above].pressure;
        else self.pressure = 1;
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
    if(self.state != ParticleState::None) {
        self.vel_y += Particle::GRAVITATIONAL_PULL;
    }
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

//
// @brief get the direction where possible to go forward; -1=NONE | 1=left | 2=right
int Update::getClosestMove(Surroundings &srd, Particle &self, std::vector<Particle> &grid, int i, ParticleState state) {
    if(state == ParticleState::Gas){
        if(srd.above == -1){
            return -1;
        }
        bool possibleLeft = true, possibleRight = true;
        for(int i = 1; i < GRID_WIDTH; i++){
            if(self.pos_x - i > 0 && possibleLeft){
                if(grid[getGridIndex(self.pos_x - i, self.pos_y)].state == ParticleState::Solid){
                    possibleLeft = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x - i, self.pos_y - 1)].state == ParticleState::None){
                        return 1;
                    }
                }
            }//else possibleLeft = false;
            if(self.pos_x + i < GRID_WIDTH - 1 && possibleRight){
                if(grid[getGridIndex(self.pos_x + i, self.pos_y)].state == ParticleState::Solid){
                    possibleRight = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x + i, self.pos_y - 1)].state == ParticleState::None){
                        return 2;
                    }
                }
            }//else possibleRight = false;
            if(!possibleLeft && !possibleRight) {
                return rand()%2+1; 
            }
        }
    }

    else if(state == ParticleState::Fluid){
        if(srd.below == -1){
            return -1; // nowhere to go lol
        }
        bool possibleLeft = true, possibleRight = true;
        for(int i = 1; i < GRID_WIDTH; i++){
            if(self.pos_x - i > 0 && possibleLeft){
                if(grid[getGridIndex(self.pos_x - i, self.pos_y)].state != ParticleState::None){
                    possibleLeft = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x - i, self.pos_y + 1)].state == ParticleState::None){
                        return 1;
                    }
                }
            }else possibleLeft = false;
            if(self.pos_x + i < GRID_WIDTH - 1 && possibleRight){
                if(grid[getGridIndex(self.pos_x + i, self.pos_y)].state != ParticleState::None){
                    possibleRight = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x + i, self.pos_y + 1)].state == ParticleState::None){
                        return 2;
                    }
                }
            }else possibleRight = false;
            if(!possibleLeft && !possibleRight) {
                return rand()%2+1; 
            }
        }
    }
    return -1;
}

void Update::transferEnergy(Surroundings &sr, Particle &self, std::vector<Particle> &grid, int i)
{  
    if(sr.above > -1   && grid[sr.above].type == self.type) grid[sr.above].energy += self.energy*0.7f;
    if(sr.below > -1   && grid[sr.below].type == self.type) grid[sr.below].energy += self.energy*0.7f;
    if(sr.left  > -1   && grid[sr.left ].type == self.type) grid[sr.left ].energy += self.energy*0.7f;
    if(sr.right > -1   && grid[sr.right].type == self.type) grid[sr.right].energy += self.energy*0.7f;
}

void Update::transmitEnergy (Surroundings& srd, Particle& self, std::vector<Particle>& grid, int i){
    if(self.energy <= 0.01f) return; // no point in calculating, no energy received :(
/*
    1) on movement: kin E+++
    2) on impact:   kin E => pot E
    3) pot E => speed for further movement => E = 0

Nothing is done on the first step when motion is present
    
*/

    if(self.inertia_x > 0.05f || self.inertia_y > 0.05f){ 
     //   self.vel_x = 0;
     //   self.vel_y = 0;
        return;
    }
    
    // 2nd and 3rd steps (after impact)
    self.inertia_x  = 0;
    self.inertia_y  = 0;
    self.energy *= 0.7f;

    // nowhere to move => just transfer it all

    if (srd.to_move == 0) {
        transferEnergy(srd, self, grid, i);
        self.energy = 0;
        return;
    }
    if(self.vel_y > 0) self.vel_y +=  sqrt(self.energy/4), self.energy /= 4;
    if(self.vel_y < 0) self.vel_y += -sqrt(self.energy/4), self.energy /= 4;

    if(self.vel_x > 0) self.vel_x +=  sqrt(self.energy/4);
    if(self.vel_x < 0) self.vel_x += -sqrt(self.energy/4);
    // if velocity is present => the direction should be predetermined 
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
