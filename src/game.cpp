#include "game.h"

int _WIDTH, _HEIGHT;           // PIXELS
int GRID_WIDTH, GRID_HEIGHT;   // PARTICLES

    // Scaleable
const int PIXEL_SIZE = 16;

int idc = 0;
                        // default arg for vector<Particle>.resize() 
Particle::Particle(ParticleType type = ParticleType::Air){
    this->type = type;
    this->id = idc++;
    colorPainter();
    vel_x = 0;
    vel_y = 0;

    switch (type)
    {
    case ParticleType::Sand:
        state = ParticleState::Solid;
        break;
    case ParticleType::Air:
        state = ParticleState::Gas;
        break;
    case ParticleType::Water:
        state = ParticleState::Fluid;
        break;
    }
}

void Particle::colorPainter(){
    switch (type)
    {
    case ParticleType::Air:
        color = {65, 175, 200};
        break;
    case ParticleType::Sand:
        color = {230, 215, 160};
        break;
    }
}
void Particle::setCoord(float pos_x, float pos_y){
    this->pos_x = pos_x;
    this->pos_y = pos_y;
}
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

/*
    INITIALIZATION
*/

Game::Game(){
    this->initWin();
    this->initVar();
}
Game::~Game(){
    delete this->window;
}

void Game::initVar(){
    grid.resize(GRID_HEIGHT*GRID_WIDTH);
    particles.setPrimitiveType(sf::Quads);
}
void Game::initWin(){
    vm = sf::VideoMode::getDesktopMode();
    vm.height = 512;
    vm.width  = 512;
    GRID_HEIGHT = std::floor(vm.height / PIXEL_SIZE) - 1; 
    GRID_WIDTH  = std::floor(vm.width  / PIXEL_SIZE);

    HEIGHT = GRID_HEIGHT * PIXEL_SIZE;
    WIDTH  = GRID_WIDTH  * PIXEL_SIZE;
    _HEIGHT = HEIGHT;
    _WIDTH = WIDTH;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "SANDBOX", sf::Style::Close, settings);
}

bool Game::isRunning() const{
    return window->isOpen();
}

/*
    UPDATE&RENDER WINDOW
*/


//
// @brief Updates the image in back buffer
void Game::update(){
    window->setFramerateLimit(60);
    while(window->pollEvent(evt)){
        switch (evt.type)
            {
            case sf::Event::KeyPressed:
                window->close(); 
                        break;
            case sf::Event::MouseButtonPressed:
                int mx = std::floor(evt.mouseButton.x / PIXEL_SIZE);
                int my = std::floor(evt.mouseButton.y / PIXEL_SIZE);
                if(mx >= 0 && mx < GRID_WIDTH && my >= 0 && my < GRID_HEIGHT){
                    grid[mx+my*GRID_WIDTH].type  = ParticleType ::Sand;
                    grid[mx+my*GRID_WIDTH].state = ParticleState::Solid;
                    grid[mx+my*GRID_WIDTH].color = {230, 215, 160};
                    grid[mx+my*GRID_WIDTH].vel_y = 0.5;
                    grid[mx+my*GRID_WIDTH].setCoord(mx, my);
                }
                        break;
        }
    }
    
    for(int i = 0; i < grid.size(); i++) {
        grid[i].Update(grid);
    }   
    particles.clear();
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            Particle& pt = grid[x + y * GRID_WIDTH];

            if(pt.type == ParticleType::Air)
                continue; 
            std::pair<int, int> pxy = pt.getCoord();
            float px = pxy.first * PIXEL_SIZE;
            float py = pxy.second * PIXEL_SIZE;
            sf::Color c(pt.color[0], pt.color[1], pt.color[2]);

            particles.append(sf::Vertex({px, py}, c));                             // top-left
            particles.append(sf::Vertex({px + PIXEL_SIZE, py}, c));                // top-right
            particles.append(sf::Vertex({px + PIXEL_SIZE, py + PIXEL_SIZE}, c));   // bottom-right
            particles.append(sf::Vertex({px, py + PIXEL_SIZE}, c));                // bottom-left
        }
    }   

}

//
// @brief Swaps the buffers back<->front
void Game::render(){
    window->clear(sf::Color::Blue);
    sf::RectangleShape floorLine(sf::Vector2f(WIDTH, 2));
    floorLine.setPosition(0, HEIGHT - 2);
    floorLine.setFillColor(sf::Color::Red);
    window->draw(floorLine);
    window->draw(particles);
    window->display();
}

/*
    LISTENERS
*/

inline void Game::ButtonCheck(){
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)){
		this->window->close();
	}
    /*
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)){
		player.move(-0.1f, 0.0f);
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)){
		player.move(0.1f, 0.0f);
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)){
		player.move(0.0f, -0.1f);
	}
	if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)){
		player.move(0.0f, 0.1f);
	}
    */
}

inline void Game::MouseCheck(){
	if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
		sf::Vector2i mousePos = sf::Mouse::getPosition(*this->window);
		//player.setPosition((float)mousePos.x, (float)mousePos.y);
	}
}
