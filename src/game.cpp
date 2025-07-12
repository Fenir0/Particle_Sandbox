#include "../include/game.hpp"
/*
    SCREEN MANAGEMENT
    DRAWING
    UPDATE&RENDER
*/

int FPS = 30;

      int PIXEL_SIZE     = 12;
const int PIXEL_SIZE_MIN =  2;
const int PIXEL_SIZE_MAX = 20;

const int UI_HEIGHT_CELL = 6;

int UI_HEIGHT_PXS     = UI_HEIGHT_CELL * PIXEL_SIZE; 
int SIM_HEIGHT_PXS    = GRID_HEIGHT    * PIXEL_SIZE; 
int SIM_WIDTH_PXS     = GRID_WIDTH     * PIXEL_SIZE; 
int WINDOW_HEIGHT_PXS = SIM_HEIGHT_PXS * PIXEL_SIZE; 
int WINDOW_WIDTH_PXS  = SIM_WIDTH_PXS; 

float padding      = PIXEL_SIZE;
float buttonWidth  = PIXEL_SIZE * 5;
float buttonHeight = PIXEL_SIZE * 3;

void Game::calculateWindowSize(){
    updateUIScaling();
    UI_HEIGHT_PXS     = UI_HEIGHT_CELL * PIXEL_SIZE; 
    SIM_HEIGHT_PXS    = GRID_HEIGHT    * PIXEL_SIZE; 
    SIM_WIDTH_PXS     = GRID_WIDTH     * PIXEL_SIZE; 
    WINDOW_HEIGHT_PXS = SIM_HEIGHT_PXS + UI_HEIGHT_PXS; 
    WINDOW_WIDTH_PXS  = SIM_WIDTH_PXS; 
    HEIGHT = WINDOW_HEIGHT_PXS;
    WIDTH  = WINDOW_WIDTH_PXS;
    pxs.resize(GRID_WIDTH * GRID_HEIGHT * 4);

}

/*
    INITIALIZATION
*/

Game::Game(){
    this->initVar(); 
    this->initWin();
}
Game::~Game(){
    delete this->window;
}

void Game::initWin(){
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    calculateWindowSize();
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "SANDBOX", sf::Style::Default, settings);
    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
    window->setVerticalSyncEnabled(true);
}

void Game::initVar(){
    GRID_HEIGHT = 64;
    GRID_WIDTH  = 64;
    grid.resize(GRID_HEIGHT*GRID_WIDTH);
    particles.setPrimitiveType(sf::Quads);
    brushSize = 2;
    selectedType = ParticleType::Sand;
    font.loadFromFile("../assets/arial.ttf");
    particleCount = 0;
}

bool Game::isRunning() const{
    return window->isOpen();
}

/*
   Window managing
*/

// Left to draw, right to erase
void Game::mouseInput(){
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
        sf::Vector2i currPos = sf::Mouse::getPosition(*window);
        sf::Vector2f overPos = window->mapPixelToCoords(currPos); 
        int mx = overPos.x / PIXEL_SIZE;
        int my = (overPos.y - UI_HEIGHT_PXS) / PIXEL_SIZE;
        if(my > 0 && mx > 0 && my < GRID_HEIGHT && mx < GRID_WIDTH)
            placeParticleOnScene(mx, my, selectedType);
    }
    else if(sf::Mouse::isButtonPressed(sf::Mouse::Right)){
        sf::Vector2i mouse = sf::Mouse::getPosition(*window);
        int mx = mouse.x / PIXEL_SIZE;
        int my = (mouse.y - UI_HEIGHT_PXS) / PIXEL_SIZE;
        if(my > 0 && mx > 0 && my < GRID_HEIGHT && mx < GRID_WIDTH)
            placeParticleOnScene(mx, my, ParticleType::Air);
    }
}

void Game::placeParticleOnScene(int mx, int my, ParticleType type) {
    for(int dy = -brushSize; dy <= brushSize; dy++){
        for(int dx = -brushSize; dx <= brushSize; dx++){
            if(dx*dx + dy*dy <= brushSize*brushSize){
                int x = mx + dx;
                int y = my + dy;
                if(x > 0 && x < GRID_WIDTH - 1 && y > 0 && y < GRID_HEIGHT){
                    int index = Update::getGridIndex(x, y);
                    if(grid[index].type == ParticleType::Air || 
                                   type == ParticleType::Air){
                        if(type==ParticleType::Air&&grid[index].type != ParticleType::Air)
                            particleCount--;
                        else if(type != ParticleType::Air&&grid[index].type == ParticleType::Air)
                            particleCount++;
                        Particle prt;
                        prt.setArgs(type, Particle::getStateByType(type), 
                              Particle::getColorByType(type), 
                                                         0, 0, 0, 0);
                        prt.setCoord(x, y);
                        grid[index] = prt;
                    }
                }
            }
        }
    }
}

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
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))   brushSize = std::min(10, brushSize+1);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) brushSize = std::max(0 , brushSize-1);
                
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)){
                    PIXEL_SIZE = std::min(PIXEL_SIZE_MAX, PIXEL_SIZE + 1);
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)){
                    PIXEL_SIZE = std::max(PIXEL_SIZE_MIN, PIXEL_SIZE - 1);
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window->close();
                        break;
            case sf::Event::MouseButtonPressed:{
                int mx = std::floor(evt.mouseButton.x / PIXEL_SIZE);
                int my = std::floor(evt.mouseButton.y / PIXEL_SIZE);
                if(evt.mouseButton.button == sf::Mouse::Left){
                    for(int i = 0; i < Particle::type_list.size(); i++){
                        float x = (padding + i*(buttonWidth+padding))/PIXEL_SIZE;
                        float y = padding/PIXEL_SIZE;

                        if(mx >= x && mx <= x + buttonWidth /PIXEL_SIZE && 
                           my >= y && my <= y + buttonHeight/PIXEL_SIZE){
                            selectedType = Particle::type_list[i];
                        }
                    }
                }
                break;
            }
            // case sf::Event::Resized:
            //     sf::FloatRect visibleArea(0, 0, evt.size.width, evt.size.height);
            //     window->setView(sf::View(visibleArea));
            //     break;
        }
    }
    mouseInput();
    
    for(int i = 0; i < grid.size(); i++) {
        grid[i].Update(grid, i);
    }   
    particles.clear();
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            Particle& pt = grid[x + y * GRID_WIDTH];

            ParticleType  type           = pt.getType();
            ParticleState state          = pt.getState();
            std::pair<float,float> pxy   = pt.getCoord();
            std::vector<short>     color = pt.getColor();
            sf::Color  c(color[0], color[1], color[2]);
            float px =                 pxy.first  * PIXEL_SIZE;
            float py = UI_HEIGHT_PXS + pxy.second * PIXEL_SIZE;
            if(type == ParticleType::Air)
                continue;

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
   // printf("%d\n", particleCount);
    window->clear(screenColor); 
    drawUI();
    window->draw(particles);
    window->display();
}

void Game::updateUIScaling() {
    padding      = PIXEL_SIZE * 2;
    buttonWidth  = PIXEL_SIZE * 5;
    buttonHeight = PIXEL_SIZE * 3;
    UI_HEIGHT_PXS = UI_HEIGHT_CELL * PIXEL_SIZE;
}

void Game::drawUI(){
    sf::Text brushText;
    brushText.setFont(font);
    brushText.setString("Brush: " + std::to_string(brushSize*2+1));
    brushText.setCharacterSize(14);
    brushText.setFillColor(sf::Color::White);
    brushText.setPosition(WINDOW_WIDTH_PXS-80, UI_HEIGHT_PXS - 20);
    window->draw(brushText);
    drawButton();
    drawBorder();
}

void Game::drawBorder(){
    sf::RectangleShape border;
    border.setFillColor(sf::Color::Black);
    
    // Top border
    border.setSize(sf::Vector2f(WIDTH, PIXEL_SIZE));
    border.setPosition(0, UI_HEIGHT_PXS);
    window->draw(border);

    // Bottom border
    border.setPosition(0, HEIGHT - PIXEL_SIZE);
    window->draw(border);

    // Left border
    border.setSize(sf::Vector2f(PIXEL_SIZE, HEIGHT));
    border.setPosition(0, 0);
    window->draw(border);

    // Right border
    border.setPosition(WIDTH - PIXEL_SIZE, 0);
    window->draw(border);

}

void Game::drawButton(){
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