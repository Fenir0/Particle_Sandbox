#include "../include/game.hpp"
/*
    SCREEN MANAGEMENT
    DRAWING
    UPDATE&RENDER
*/

const int FPS = 30;
    // Scaleable (not currently)
const int PIXEL_SIZE = 12;

float padding = PIXEL_SIZE*2;
float buttonWidth = PIXEL_SIZE*9;
float buttonHeight = PIXEL_SIZE*3;

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

void Game::initWin(){
    vm = sf::VideoMode::getDesktopMode();
    vm.height = 512;
    vm.width  = 512;
    GRID_HEIGHT = std::floor(vm.height / PIXEL_SIZE) - 1; 
    GRID_WIDTH  = std::floor(vm.width  / PIXEL_SIZE);

    HEIGHT = GRID_HEIGHT * PIXEL_SIZE;
    WIDTH  = GRID_WIDTH  * PIXEL_SIZE;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;

    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "SANDBOX", sf::Style::Close, settings);
}

void Game::initVar(){
    grid.resize(GRID_HEIGHT*GRID_WIDTH);
    particles.setPrimitiveType(sf::Quads);
}

bool Game::isRunning() const{
    return window->isOpen();
}

/*
   Window managing
*/

//
// @brief Updates the image in back buffer
void Game::update(){
    window->setFramerateLimit(FPS);
    while(window->pollEvent(evt)){
        switch (evt.type)
            {
            case sf::Event::KeyPressed:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) selectedType = ParticleType::Sand;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) selectedType = ParticleType::Water;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) selectedType = ParticleType::Air;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window->close();
                        break;
            case sf::Event::MouseButtonPressed:
                int mx = std::floor(evt.mouseButton.x / PIXEL_SIZE);
                int my = std::floor(evt.mouseButton.y / PIXEL_SIZE);
                if(evt.mouseButton.button == sf::Mouse::Left){
                    for(int i = 0; i < Particle::type_list.size(); i++){
                        float x = (padding + i*(buttonWidth+padding))/PIXEL_SIZE;
                        float y = padding/PIXEL_SIZE;

                        if(mx >= x && mx <= x + buttonWidth/PIXEL_SIZE && 
                        my >= y && my <= y + buttonHeight/PIXEL_SIZE){
                            selectedType = Particle::type_list[i];
                        }
                    }
                    if(my > (buttonHeight+padding)/PIXEL_SIZE){
                        if(mx >= 0 && mx < GRID_WIDTH && my >= 0 && my < GRID_HEIGHT){
                            grid[mx+my*GRID_WIDTH].setArgs(selectedType, Particle::getStateByType(selectedType), 
                                                                Particle::getColorByType(selectedType), 0, 0, 0);
                            grid[mx+my*GRID_WIDTH].setCoord(mx, my);
                        }
                    }
                }
                else if(evt.mouseButton.button == sf::Mouse::Right){
                    if(my > (buttonHeight+padding)/PIXEL_SIZE){
                        if(mx >= 0 && mx < GRID_WIDTH && my >= 0 && my < GRID_HEIGHT){
                            grid[mx+my*GRID_WIDTH].setArgs(ParticleType::Air, Particle::getStateByType(ParticleType::Air), 
                                                                Particle::getColorByType(ParticleType::Air), 0, 0, 0);
                            grid[mx+my*GRID_WIDTH].setCoord(mx, my);
                        }
                    }
                }
                break;
        }
    }
    
    for(int i = 0; i < grid.size(); i++) {
        grid[i].Update(grid, i);
    }   
    particles.clear();
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            Particle& pt = grid[x + y * GRID_WIDTH];

            ParticleType  type       = pt.getType();
            ParticleState state      = pt.getState();
            std::pair<int,int> pxy   = pt.getCoord();
            std::vector<short> color = pt.getColor();
            float px = pxy.first  * PIXEL_SIZE;
            float py = pxy.second * PIXEL_SIZE;
            if(type == ParticleType::Air){
                continue;
            }
            sf::Color c(color[0], color[1], color[2]);

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
    window->clear(screenColor); 
    buttonDraw();
    sf::RectangleShape floorLine(sf::Vector2f(WIDTH, 2));
    floorLine.setPosition(0, HEIGHT - 2);
    floorLine.setFillColor(sf::Color::Red);
    window->draw(floorLine);
    window->draw(particles);


    window->display();
}

void Game::buttonDraw(){
    sf::Font font;
    font.loadFromFile("../assets/arial.ttf");

    for(int i = 0; i < Particle::type_list.size(); i++){
        sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(padding+i*(buttonWidth+padding), padding);

        if(selectedType == Particle::type_list[i]){
            button.setFillColor(sf::Color(200, 200, 100));
        }
        else{
            button.setFillColor(sf::Color(150, 150, 150));
        }
        button.setOutlineColor(sf::Color::Black);
        button.setOutlineThickness(2);

        sf::Text text;
        text.setFont(font);
        text.setString(Particle::names_list[i]);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::Black);
        text.setPosition(button.getPosition().x + 10, button.getPosition().y + 10);
        window->draw(button);
        window->draw(text);

    }
}