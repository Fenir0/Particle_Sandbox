#include "../include/update.hpp"

// particle used grid is [1...GRID_DIMENSION-1]
int Update::offset = 1;

struct Surroundings{
    int   above,   below,     left,   right;
    int diag_bl, diag_br, diag_al, diag_ar;
    int to_move;

    Surroundings(int x, int y){
        above = y > Update::offset               ? Update::getGridIndex(x, y - 1): -1;
        below = y < GRID_HEIGHT - Update::offset ? Update::getGridIndex(x, y + 1): -1;
        left  = x > Update::offset               ? Update::getGridIndex(x - 1, y): -1;
        right = x < GRID_WIDTH  - Update::offset ? Update::getGridIndex(x + 1, y): -1;

        diag_bl = (below != -1 && left  != -1) ? left  + GRID_WIDTH : -1;
        diag_br = (below != -1 && right != -1) ? right + GRID_WIDTH : -1;
        diag_al = (above != -1 && left  != -1) ? left  - GRID_WIDTH : -1;
        diag_ar = (above != -1 && right != -1) ? right - GRID_WIDTH : -1;
        to_move = (above != -1) + (below != -1) + (left != -1) + (right != -1) +
                (diag_al != -1) + (diag_ar != -1) + (diag_bl != -1) + (diag_br != -1);

    }
};

bool isGasOrNone(std::vector<Particle>& grid, int pos){
    return (pos >= -1) && ((grid[pos].getState() == ParticleState::Gas) 
                         || (grid[pos].getState() == ParticleState::None));
}
bool isSolidOrFluid(std::vector<Particle>& grid, int pos){
    return (pos >= -1) && ((grid[pos].getState() == ParticleState::Solid) 
                         || (grid[pos].getState() == ParticleState::Fluid));
}

void adjustForBorders(Particle& self){
    if (self.pos_x < 1) self.pos_x = 1;
    if (self.pos_y < 1) self.pos_y = 1;
    if (self.pos_x > GRID_WIDTH  - 1) self.pos_x = GRID_WIDTH  - 1;
    if (self.pos_y > GRID_HEIGHT - 1) self.pos_y = GRID_HEIGHT - 1;
}

void zeroOut(Particle& self){
    self.inertia_x = 0;
    self.inertia_y = 0;
    self.vel_x     = 0;
    self.vel_y     = 0;
}

float simulationSpeed = 0.5f;

int waterCoolDown = 2;
int oilCoolDown   = 3;
int LavaCoolDown  = 4;
int sandCoolDown  = 1;

//
// @brief Clamp to 1 tile per 1 frame
float inertia_to_movement(float inertia, ParticleType type) {
    int kx;
    switch (type)
    {
    case ParticleType::Sand : kx = 1.f; break;
    case ParticleType::Oil  : kx = 0.3f; break;
    case ParticleType::Water: kx = 0.8f; break;
    case ParticleType::Smoke: kx = 1.f; break;

    }
    if (inertia > simulationSpeed)  return simulationSpeed*kx;
    if (inertia < -simulationSpeed) return -simulationSpeed*kx;
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
    self.coolDownTick(1);
    if(self.getCooldown() > 0) return;
    self.setCooldown(std::max(2, self.getCooldown())); 
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
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x, self.getType()) + self.vel_x*(rand()%3)/5.f;
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y, self.getType()) + self.vel_y*(rand()%3)/5.f;
    int new_pos = getGridIndex(new_x, new_y);

    self.vel_x *= 0.7f;
    // Get the direction (stable if not moved/not possible to move)
    Movement mv = getMovement(new_pos, srd);
    
    updateOnSurroundings(srd, self, grid, i);
    self.setTemperature(std::max(self.getTemperature(), 10));
    if(self.getTemperature() >= 500){
        int r = rand()%Particle::colorBurnedSand.size();
        self.setColor(Particle::colorBurnedSand[r]);
        return;
    }

    // velocity not enough to change position (yet)
    if(new_pos > -1 && mv == Movement::STABLE){ 
        self.vel_x = 0;
        self.vel_y = 0;
        self.pos_x += inertia_to_movement(self.inertia_x, self.getType());
        self.pos_y += inertia_to_movement(self.inertia_y, self.getType());
        
        // sand might collapse when under pressure
        if((srd.diag_bl >= 0 || srd.diag_br >= 0) && srd.below >= 0 && grid[srd.below].getState() == ParticleState::Solid){
            int r = rand() % 2 + 2;
            if(r < self.pressure){
                //printf("%d\n", self.pressure);
                self.inertia_x = 0;
                self.inertia_y = 0;
                r = rand()%2;
                if(r%2){
                    if(grid[srd.diag_bl].getState() != ParticleState::Solid && 
                                    grid[srd.left].getState() != ParticleState::Solid){
                        if(grid[getGridIndex(self.pos_x-1, self.pos_y+1)].getState() 
                                == ParticleState::Fluid) self.setCooldown(10);
                        swapper(self.pos_x, self.pos_y, 
                                self.pos_x-1, self.pos_y+1, grid);
                        return;
                    }
                }
                else{
                    if(grid[srd.diag_br].getState() != ParticleState::Solid && 
                                    grid[srd.right].getState() != ParticleState::Solid){
                        if(grid[getGridIndex(self.pos_x+1, self.pos_y+1)].getType() 
                                == ParticleType::Oil) self.setCooldown(10);
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
    switch (grid[new_pos].getState())
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
                if(next_move == 0 && new_x > 1 && new_y < GRID_HEIGHT - 1
                     && isGasOrNone(grid, srd.diag_bl) && isGasOrNone(grid, srd.left) ){
                    self.inertia_x = - 0.01f;
                    self.vel_y = 0;
                    self.inertia_y /= 3;
                    return;
                }
                else if(next_move == 1 && new_x < GRID_WIDTH - 2 && new_y < GRID_HEIGHT - 1  
                        && isGasOrNone(grid, srd.diag_br) && isGasOrNone(grid, srd.right)){
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
                if(isGasOrNone(grid, srd.below)){
                    new_pos+=1;
                }
                else if(isGasOrNone(grid, srd.left)){
                    new_pos -= GRID_WIDTH;
                }
                swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                zeroOut(self);
                return;
            }
            else if(mv == Movement::DG_BR){
                if(isGasOrNone(grid, srd.below)){
                    new_pos-=1;
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    zeroOut(self);
                    return;
                }
                else if(isGasOrNone(grid, srd.right)){
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

void Update::update_WETSAND(Particle &self, std::vector<Particle> &grid, int i){ 
    self.coolDownTick(1);
    if(self.getCooldown() > 0) return;
    self.setCooldown(std::max(3, self.getCooldown()));
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

    updateOnSurroundings(srd, self, grid, i);

    // next position                                             // some noice for less idealistic moves
    float new_x = self.pos_x + inertia_to_movement(self.inertia_x, self.getType()) + self.vel_x*(rand()%3)/5.f;
    float new_y = self.pos_y + inertia_to_movement(self.inertia_y, self.getType()) + self.vel_y*(rand()%3)/5.f;
    int new_pos = getGridIndex(new_x, new_y);

    //self.vel_x *= 0.7f;
    // Get the direction (stable if not moved/not possible to move)
    Movement mv = getMovement(new_pos, srd);

    // velocity not enough to change position (yet)
    if(new_pos > -1 && mv == Movement::STABLE){ 
        self.vel_x = 0;
        self.vel_y = 0;
        self.pos_x += inertia_to_movement(self.inertia_x, self.getType());
        self.pos_y += inertia_to_movement(self.inertia_y, self.getType());
        return;
    }
    if (new_pos < 0) {
        zeroOut(self);
        return;
    }
    switch (grid[new_pos].getState())
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
            self.setCooldown(5);
            swapper(self.pos_x, self.pos_y, 
                         new_x,      new_y,    grid);
            return;

                // COLLIDING WITH SOLID
        case ParticleState::Solid:
            self.pos_x = std::floor(self.pos_x);
            self.pos_y = std::floor(self.pos_y);
            if(mv == Movement::BELOW){
                int next_move = rand()%3;
                if(next_move == 0 && new_x > 1 && new_y < GRID_HEIGHT - 1
                     && isGasOrNone(grid, srd.diag_bl) && isGasOrNone(grid, srd.left) ){
                    self.inertia_x = - 0.01f;
                    self.vel_y = 0;
                    self.inertia_y /= 3;
                    return;
                }
                else if(next_move == 1 && new_x < GRID_WIDTH - 2 && new_y < GRID_HEIGHT - 1  
                        && isGasOrNone(grid, srd.diag_br) && isGasOrNone(grid, srd.right)){
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
                if(isGasOrNone(grid, srd.below)){
                    new_pos+=1;
                }
                else if(isGasOrNone(grid, srd.left)){
                    new_pos -= GRID_WIDTH;
                }
                swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                zeroOut(self);
                return;
            }
            else if(mv == Movement::DG_BR){
                if(isGasOrNone(grid, srd.below)){
                    new_pos-=1;
                    swapper(self.pos_x, self.pos_y, 
                        new_x,      new_y,    grid);
                    zeroOut(self);
                    return;
                }
                else if(isGasOrNone(grid, srd.right)){
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

void Update::update_LAVA(Particle &self, std::vector<Particle> &grid, int i){
    self.coolDownTick(1);
    if(self.getCooldown() > 0) 
        return;
    self.setCooldown(std::max(self.getCooldown(), LavaCoolDown));
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

   if(self.getTemperature() <= 0) self.setArgs(ParticleType::Smoke, self.vel_x, self.vel_y, 0, 0, 0);

    // Always try to move down first
    if(isGasOrNone(grid, srd.below) 
                || moreDense(self, srd.below, grid)){
        possibleMoves.emplace_back(0);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_bl) 
                || moreDense(self, srd.diag_bl, grid)){
        possibleMoves.emplace_back(1);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_br)
                || moreDense(self, srd.diag_br, grid)){
        possibleMoves.emplace_back(2);
    }
    if(!possibleMoves.empty()){
        int r = rand()%possibleMoves.size();
        new_y++;
        if(possibleMoves[r] == 1) new_x--;
        if(possibleMoves[r] == 2) new_x++;
        if(getGridIndex(new_x, new_y) != -1 || 
                 moreDense(self, getGridIndex(new_x, new_y), grid)){
            swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
            return;
        }
    }

    // if down is impossible, move left or right
    int r = getClosestMove(srd, self, grid, i, self.getState());
    if(r == 1){
        if(isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid)){
            self.setCooldown(LavaCoolDown * 2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid)){
            self.setCooldown(LavaCoolDown * 2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    if(getGridIndex(self.pos_x - 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid))){
            self.setCooldown(LavaCoolDown * 2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    if(getGridIndex(self.pos_x + 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid))){
            self.setCooldown(LavaCoolDown * 2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    if(isSolidOrFluid(grid, srd.above) && isSameState(grid, i, srd.above)) grid[srd.above].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.below) && isSameState(grid, i, srd.below)) grid[srd.below].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.left ) && isSameState(grid, i, srd.left )) grid[srd.left ].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.right) && isSameState(grid, i, srd.right)) grid[srd.right].pressure = self.pressure;

}

void Update::update_SMOKE(Particle &self, std::vector<Particle> &grid, int i){ 
    // In case the old position is out of borders
    self.coolDownTick(1);
    if(self.getCooldown() > 0) return;
    self.setCooldown(std::max(self.getCooldown(), 3));
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
    if(srd.above > -1 && grid[srd.above].getState() == ParticleState::None 
                        && moreDense(self, srd.above, grid)
    ){
        possibleMoves.emplace_back(0);
    }
    if(srd.diag_al > -1 && grid[srd.diag_al].getState() == ParticleState::None 
                        && grid[srd.left].getState() == ParticleState::None
                        && moreDense(self, srd.left, grid)
                        && moreDense(self, srd.diag_al, grid)){
        possibleMoves.emplace_back(1);
    }
    if(srd.diag_ar > -1 && grid[srd.diag_ar].getState() == ParticleState::None 
                        && grid[srd.right].getState() == ParticleState::None
                        && moreDense(self, srd.right, grid)
                        && moreDense(self, srd.diag_ar, grid)){
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
    int r = getClosestMove(srd, self, grid, i, self.getState());
    if(r == -1 && srd.above == -1) r = rand()%2+1;
    if(r == 1){
        if(getGridIndex(self.pos_x - 1, self.pos_y) != -1
        && grid[getGridIndex(self.pos_x - 1, self.pos_y)].getState() == ParticleState::None
        && moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid)){
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(getGridIndex(self.pos_x + 1, self.pos_y) != -1 
        && grid[getGridIndex(self.pos_x + 1, self.pos_y)].getState() == ParticleState::None
        && moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid)){
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    possibleMoves.clear();
    //return;
    // if not moved, just swap with same particles to simulate chaos
    try{
    if(srd.above > -1 && grid[srd.above].getState() == self.getState()){
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
        int r = rand()%(possibleMoves.size()+60);
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

void Update::update_STEAM(Particle &self, std::vector<Particle> &grid, int i){ 
    // In case the old position is out of borders
    self.coolDownTick(1);
    if(self.getCooldown() > 0) return;
    self.setCooldown(std::max(self.getCooldown(), 3));
    adjustForBorders(self);

    // Current position and surroundings
    int old_pos = getGridIndex(self.pos_x, self.pos_y);
    // Coordinates of surrounding tiles
    Surroundings srd (std::floor(self.pos_x),std::floor(self.pos_y));
    std::vector<int> possibleMoves;
    possibleMoves.reserve(10);

    float new_x = self.pos_x;
    float new_y = self.pos_y;

    // Gas always moves up if possible (be it up, up-left, up-right)
    //                      looks awful, yeah           but works
    if(srd.above > -1 && grid[srd.above].getState() == ParticleState::None 
                        && moreDense(self, srd.above, grid)
    ){
        possibleMoves.emplace_back(0);
    }
    if(srd.diag_al > -1 && grid[srd.diag_al].getState() == ParticleState::None 
                        && grid[srd.left].getState() == ParticleState::None
                        && moreDense(self, srd.left, grid)
                        && moreDense(self, srd.diag_al, grid)){
        possibleMoves.emplace_back(1);
    }
    if(srd.diag_ar > -1 && grid[srd.diag_ar].getState() == ParticleState::None 
                        && grid[srd.right].getState() == ParticleState::None
                        && moreDense(self, srd.right, grid)
                        && moreDense(self, srd.diag_ar, grid)){
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
    int r = getClosestMove(srd, self, grid, i, self.getState());
    if(r == -1 && srd.above == -1) r = rand()%2+1;
    if(r == 1){
        if(getGridIndex(self.pos_x - 1, self.pos_y) != -1
        && grid[getGridIndex(self.pos_x - 1, self.pos_y)].getState() == ParticleState::None
        && moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid)){
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(getGridIndex(self.pos_x + 1, self.pos_y) != -1 
        && grid[getGridIndex(self.pos_x + 1, self.pos_y)].getState() == ParticleState::None
        && moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid)){
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    possibleMoves.clear();
    //return;
    // if not moved, just swap with same particles to simulate chaos
    try{
    if(srd.above > -1 && grid[srd.above].getState() == self.getState()){
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
        int r = rand()%(possibleMoves.size()+60);
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
    self.coolDownTick(1);
    if(self.getCooldown() > 0) 
        return;
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

    updateOnSurroundings(srd, self, grid, i);

    // Always try to move down first
    if(isGasOrNone(grid, srd.below) 
                || moreDense(self, srd.below, grid)){
        possibleMoves.emplace_back(0);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_bl) 
                || moreDense(self, srd.diag_bl, grid)){
        possibleMoves.emplace_back(1);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_br)
                || moreDense(self, srd.diag_br, grid)){
        possibleMoves.emplace_back(2);
    }
    if(!possibleMoves.empty()){
        int r = rand()%possibleMoves.size();
        new_y++;
        if(possibleMoves[r] == 1) new_x--;
        if(possibleMoves[r] == 2) new_x++;
        if(getGridIndex(new_x, new_y) != -1 || 
                 moreDense(self, getGridIndex(new_x, new_y), grid)){
            self.setCooldown(waterCoolDown);
            swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
            return;
        }
    }

    // if down is impossible, move left or right
    int r = getClosestMove(srd, self, grid, i, self.getState());
    if(r == 1){
        if(isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid)){
            self.setCooldown(waterCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid)){
            self.setCooldown(waterCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    if(getGridIndex(self.pos_x - 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid))){
            self.setCooldown(waterCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    if(getGridIndex(self.pos_x + 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid))){
            self.setCooldown(waterCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    if(isSolidOrFluid(grid, srd.above) && isSameState(grid, i, srd.above)) grid[srd.above].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.below) && isSameState(grid, i, srd.below)) grid[srd.below].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.left ) && isSameState(grid, i, srd.left )) grid[srd.left ].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.right) && isSameState(grid, i, srd.right)) grid[srd.right].pressure = self.pressure;


    self.setCooldown(waterCoolDown);
    
}

void Update::update_OIL(Particle &self, std::vector<Particle> &grid, int i){
    self.coolDownTick(1);
    if(self.getCooldown() > 0) 
        return;
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

    updateOnSurroundings(srd, self, grid, i);
    if(self.getTemperature() > 100) {
        if(self.getTemperature()%10 == 5) {
        if(grid[srd.above].getType() == ParticleType::None)
            grid[srd.above].setArgs(ParticleType::Smoke, self.vel_x, self.vel_y, 0, 0, 100);
        else if(grid[srd.above].getType() == ParticleType::None)
            grid[srd.left ].setArgs(ParticleType::Smoke, self.vel_x, self.vel_y, 0, 0, 100);
        else if(grid[srd.above].getType() == ParticleType::None)
            grid[srd.right].setArgs(ParticleType::Smoke, self.vel_x, self.vel_y, 0, 0, 100);
        }
    }
    

    if(isGasOrNone(grid, srd.below) 
                || moreDense(self, srd.below, grid)){
        possibleMoves.emplace_back(0);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_bl) 
                || moreDense(self, srd.diag_bl, grid)){
        possibleMoves.emplace_back(1);
    }
    if(isGasOrNone(grid, srd.below) && isGasOrNone(grid, srd.diag_br)
                || moreDense(self, srd.diag_br, grid)){
        possibleMoves.emplace_back(2);
    }
    if(!possibleMoves.empty()){
        int r = rand()%possibleMoves.size();
        new_y++;
        if(possibleMoves[r] == 1) new_x--;
        if(possibleMoves[r] == 2) new_x++;
        if(getGridIndex(new_x, new_y) != -1 || 
                 moreDense(self, getGridIndex(new_x, new_y), grid)){
            self.setCooldown(oilCoolDown);
            swapper(self.pos_x, self.pos_y, new_x, new_y, grid);
            return;
        }
    }

    // if down is impossible, move left or right
    int r = getClosestMove(srd, self, grid, i, self.getState());
    if(r == 1){
        if(isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid)){
            self.setCooldown(oilCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    }
    if(r == 2) {
        if(isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid)){
            self.setCooldown(oilCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    }
    if(getGridIndex(self.pos_x - 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x + 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x + 1, self.pos_y), grid))){
            self.setCooldown(oilCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x + 1, self.pos_y, grid);
            return;
        }
    if(getGridIndex(self.pos_x + 1, self.pos_y) == -1 
        && (isGasOrNone(grid, getGridIndex(self.pos_x - 1, self.pos_y))
            || moreDense(self, getGridIndex(self.pos_x - 1, self.pos_y), grid))){
            self.setCooldown(oilCoolDown*2);
            swapper(self.pos_x, self.pos_y, self.pos_x - 1, self.pos_y, grid);
            return;
        }
    if(isSolidOrFluid(grid, srd.above)) grid[srd.above].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.left )) grid[srd.left ].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.right)) grid[srd.right].pressure = self.pressure;
    if(isSolidOrFluid(grid, srd.below)) grid[srd.below].pressure = self.pressure;

    self.setCooldown(oilCoolDown);
    
}

void Update::updateOnSurroundings(Surroundings& srd, Particle& self, std::vector<Particle> &grid, int i){
    if(self.getType() == ParticleType::Water){
        if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Lava ){
                self.setArgs(ParticleType::Steam, self.vel_x, self.vel_y, 0, 0, 100);
                if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Lava) 
                            grid[srd.above].setTemperature(self.getTemperature() - 1);
                if(isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Lava) 
                            grid[srd.below].setTemperature(self.getTemperature() - 1);
                if(isSolidOrFluid(grid, srd.left) && grid[srd.left].getType() == ParticleType::Lava) 
                            grid[srd.left ].setTemperature(self.getTemperature() - 1);

                if(isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Lava) 
                            grid[srd.right].setTemperature(self.getTemperature() - 1);
        }
        return;
    }
    if(self.getType() == ParticleType::Oil){
        if( isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Lava ){
                self.setTemperature(self.getTemperature() + 100);
                if(self.getTemperature() > 5'000){
                    self.setArgs(ParticleType::Smoke, self.vel_x, self.vel_y, 0, 0, 0);
                    return;
                }
                self.setTemperature(self.getTemperature() + 1);
                return;
        }
        else {
            self.setTemperature(std::max(self.getTemperature()/5, 10));
        }
        return;
    }
    if(self.getType() == ParticleType::Sand){
        // change to wet if water around
        if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Water ){
                self.setWetness(200);
                self.setArgs(ParticleType::WetSand, self.vel_x, self.vel_y, self.pressure, 0, 0);
        }
        if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Lava ){
                int r = rand()%Particle::colorBurnedSand.size();
                self.setColor(Particle::colorBurnedSand[r]);
                self.setTemperature(1000);
        }
        else if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Oil ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Oil ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Oil ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Oil){
                self.setWetness(100);
                self.setArgs(ParticleType::WetSand, self.vel_x, self.vel_y, self.pressure, 0, 0);
        }
        else if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Sand ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Sand ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Sand ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Sand){
                if(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Sand){
                    self.setTemperature(std::max(self.getTemperature(), (int) (grid[srd.above].getTemperature()*0.7)));
                }
                if(isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Sand){
                    self.setTemperature(std::max(self.getTemperature(), (int) (grid[srd.below].getTemperature()*0.7)));
                }
                if(isSolidOrFluid(grid, srd.left) && grid[srd.left].getType() == ParticleType::Sand){
                    self.setTemperature(std::max(self.getTemperature(), (int) (grid[srd.left].getTemperature()*0.7)));
                }
                if(isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Sand){
                    self.setTemperature(std::max(self.getTemperature(), (int) (grid[srd.right].getTemperature()*0.7)));
                }
        }
        return;
    }
    
    if(self.getType() == ParticleType::WetSand){
        if(!(isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Water ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Water) ){
                self.setWetness(self.getWetness() - 1);
                if(self.getWetness() == 0)
                    self.setArgs(ParticleType::Sand, self.vel_x, self.vel_y, self.pressure, 0, 0);
            }
        if((isSolidOrFluid(grid, srd.above) && grid[srd.above].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.below) && grid[srd.below].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.left ) && grid[srd.left ].getType() == ParticleType::Lava ||
            isSolidOrFluid(grid, srd.right) && grid[srd.right].getType() == ParticleType::Lava) ){
                self.setArgs(ParticleType::Sand, self.vel_x, self.vel_y, self.pressure, 0, 0);
            }
        return;
    }
}

/*
    FORCE CHECKERS
*/

bool Update::moreDense(Particle& self, int other, std::vector<Particle>& grid){
    if(other < 0) return false;
    Particle tmp = grid[other];
    return self.getDensity() > tmp.getDensity();
}
//
//@brief Get the pressure of particles above
void Update::getPressure(Surroundings& sr, Particle&self, std::vector<Particle> &grid, int i) {
    if(!isGasOrNone(grid, i)){
        if(sr.above > -1 && grid[sr.above].getState() != ParticleState::None 
                         && grid[sr.above].getState() != ParticleState::Gas
                         && grid[sr.above].getType()  != ParticleType::Stone) 
            self.pressure += grid[sr.above].pressure + 1;
        else self.pressure = 1;
    }
}

//
// @brief Apply the gravitational force
void Update::getGravity(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    if(self.pos_y >= GRID_HEIGHT - offset){
        self.pos_y = GRID_HEIGHT - offset;
        return;
    }
    if(self.getState() != ParticleState::None) {
        self.vel_y += Particle::GRAVITATIONAL_PULL;
    }
}

void Update::getFriction(Surroundings& sr,Particle&self, 
                            std::vector<Particle> &grid, int i) {
    switch (self.getType())
    {
    case ParticleType::Sand:
        if(sr.below == -1){
            self.vel_x = - self.inertia_x * 0.3f;
            return;
        }
        if(sr.below > -1  && sr.below < grid.size()) {
            if(grid[sr.below].getState() == ParticleState::Solid){
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
            if(grid[sr.below].getState() == ParticleState::Solid){
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
                if(grid[getGridIndex(self.pos_x - i, self.pos_y)].getState() == ParticleState::Solid){
                    possibleLeft = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x - i, self.pos_y - 1)].getState() == ParticleState::None){
                        return 1;
                    }
                }
            }//else possibleLeft = false;
            if(self.pos_x + i < GRID_WIDTH - 1 && possibleRight){
                if(grid[getGridIndex(self.pos_x + i, self.pos_y)].getState() == ParticleState::Solid){
                    possibleRight = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x + i, self.pos_y - 1)].getState() == ParticleState::None){
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
        for(int i = offset; i < GRID_WIDTH; i++){
            if(self.pos_x - i > 0 && possibleLeft){
                if(grid[getGridIndex(self.pos_x - i, self.pos_y)].getState() != ParticleState::None){
                    possibleLeft = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x - i, self.pos_y + 1)].getState() == ParticleState::None){
                        return 1;
                    }
                }
            }else possibleLeft = false;
            if(self.pos_x + i < GRID_WIDTH - offset && possibleRight){
                if(grid[getGridIndex(self.pos_x + i, self.pos_y)].getState() != ParticleState::None){
                    possibleRight = false;
                }
                else{
                    if(grid[getGridIndex(self.pos_x + i, self.pos_y + 1)].getState() == ParticleState::None){
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

// @brief Swap particles [pos_x, pos_y] and [new_x, new_y]
void Update::swapper(float pos_x, float pos_y, float new_x, float new_y, std::vector<Particle>& grid){
    int old_pos = getGridIndex(pos_x, pos_y);
    int new_pos = getGridIndex(new_x, new_y);
    grid[new_pos].setCoord(pos_x, pos_y);
    grid[old_pos].setCoord(new_x, new_y);
    std::swap(grid[old_pos], grid[new_pos]);
}

bool Update::isSameType(std::vector<Particle>&  grid, int i, int t){
    return i > -1 && t > -1 && (grid[i].getType() == grid[t].getType());
}
bool Update::isSameState(std::vector<Particle>&  grid, int i, int t){
    return i > -1 && t > -1 && (grid[i].getState() == grid[t].getState());
}

int Update::getGridIndex(float x, float y) {
    int ix = std::floor(x);
    int iy = std::floor(y);
    if (ix < offset|| iy < offset || ix >= GRID_WIDTH - offset || iy >= GRID_HEIGHT - offset){
      //  printf("WRONG COORD\n");
        return -1;
    }
    return ix + iy * GRID_WIDTH;
}
