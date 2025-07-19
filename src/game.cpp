#include "../include/game.hpp"
/*
    SCREEN MANAGEMENT
    DRAWING
    UPDATE&RENDER
*/
int FPS = 60;

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

unsigned int defaultCharSize = 14;

bool clearSelect = false;

bool SolidSelect = false;
bool FluidSelect = false;
bool GasSelect   = false;
bool SolidHoverSelect = false;
bool FluidHoverSelect = false;
bool GasHoverSelect   = false;

bool suppressClick = false;

void Game::calculateWindowSize(){
    updateUIScaling();
    UI_HEIGHT_PXS     = UI_HEIGHT_CELL * PIXEL_SIZE; 
    SIM_HEIGHT_PXS    = GRID_HEIGHT    * PIXEL_SIZE; 
    SIM_WIDTH_PXS     = GRID_WIDTH     * PIXEL_SIZE; 
    WINDOW_HEIGHT_PXS = SIM_HEIGHT_PXS + UI_HEIGHT_PXS*2+PIXEL_SIZE; 
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
    // Defaults
    GRID_HEIGHT = 64;
    GRID_WIDTH  = 64;

    brushSize = 2;
    particleCount = 0;
    Update::freezeSelect = false;
    selectedType = ParticleType::Sand;

    // Resize and load
    grid.resize(GRID_HEIGHT*GRID_WIDTH);
    particles.setPrimitiveType(sf::Quads);
    font.loadFromFile("../assets/arial.ttf");
}

bool Game::isRunning() const{
    return window->isOpen();
}

/*
   Particle placement
*/

void Game::mouseInput(){
    // Left click - place
    if(sf::Mouse::isButtonPressed(sf::Mouse::Left)){
        sf::Vector2i currPos = sf::Mouse::getPosition(*window);
        sf::Vector2f overPos = window->mapPixelToCoords(currPos); 
        int mx = overPos.x / PIXEL_SIZE;
        int my = (overPos.y - UI_HEIGHT_PXS) / PIXEL_SIZE;
        if(my > 0 && mx > 0 && my < GRID_HEIGHT - 1 && mx < GRID_WIDTH - 1)
            placeParticleOnScene(mx, my, selectedType);
    }
    // Right click - erase
    else if(sf::Mouse::isButtonPressed(sf::Mouse::Right)){
        sf::Vector2i mouse = sf::Mouse::getPosition(*window);
        int mx = mouse.x / PIXEL_SIZE;
        int my = (mouse.y - UI_HEIGHT_PXS) / PIXEL_SIZE;
        if(my > 0 && mx > 0 && my < GRID_HEIGHT - 1 && mx < GRID_WIDTH - 1)
            placeParticleOnScene(mx, my, ParticleType::None);
    }
}

void Game::placeParticleOnScene(int mx, int my, ParticleType type) {
    for(int dy = -brushSize; dy <= brushSize; dy++){
        for(int dx = -brushSize; dx <= brushSize; dx++){
            if(dx*dx + dy*dy <= brushSize*brushSize){
                int x = mx + dx;
                int y = my + dy;
                if(x > 0 && x < GRID_WIDTH - 1 && y > 0 && y < GRID_HEIGHT - 1){
                    int index = Update::getGridIndex(x, y);
                    if(grid[index].type == ParticleType::None || 
                                   type == ParticleType::None){
                        if(type==ParticleType::None && 
                            grid[index].type != ParticleType::None)
                            particleCount--;
                        else if(type != ParticleType::None &&
                            grid[index].type == ParticleType::None)
                            particleCount++;

                        Particle prt;
                        prt.setArgs(type, Particle::getStateByType(type), 
                                          Particle::getColorByType(type), 
                                                         0, 0, 0, 0, Particle::getTempByType(selectedType));
                        prt.setCoord(x, y);
                        grid[index] = prt;
                    }
                }
            }
        }
    }
}

/*
    Main event loop
*/
void Game::update(){
    clearSelect = false;
    window->setFramerateLimit(FPS);
    while(window->pollEvent(evt)){
        switch (evt.type)
            {
            case sf::Event::KeyPressed:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    brushSize = std::min(10, brushSize+1);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  brushSize = std::max(0 , brushSize-1);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) Update::freezeSelect = !Update::freezeSelect;
                
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)){
                    PIXEL_SIZE = std::min(PIXEL_SIZE_MAX, PIXEL_SIZE + 1);
                    defaultCharSize++;
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)){
                    PIXEL_SIZE = std::max(PIXEL_SIZE_MIN, PIXEL_SIZE - 1);
                    defaultCharSize--;
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window->close();

                break;
            case sf::Event::Closed:
                window->close();
                break;
            
            case sf::Event::MouseButtonPressed:{
                int mx = std::floor(evt.mouseButton.x / PIXEL_SIZE);
                int my = std::floor(evt.mouseButton.y / PIXEL_SIZE);
                if(evt.mouseButton.button == sf::Mouse::Left){
                    for(int i1 = 0; i1 < Particle::states_list.size(); i1++){
                        float xs = (padding + i1*(buttonWidth+padding))/PIXEL_SIZE;
                        float ys = padding/PIXEL_SIZE;
                        if(checkIfHoverSelect(i1)){
                            for(int i2 = 0; i2 < Particle::type_list[i1].size(); i2++){
                                float x = (padding + i1*(buttonWidth+padding))/PIXEL_SIZE;
                                float y = (padding + i2*buttonHeight)/PIXEL_SIZE;

                                if(mx >= x && mx <= x + buttonWidth /PIXEL_SIZE && 
                                my >= y && my <= y + buttonHeight/PIXEL_SIZE){
                                    selectedType = Particle::types[i1][i2];
                                    stateDisable();
                                    hoverDisable();
                                    if(i1 == 0) SolidSelect = true;
                                    if(i1 == 1) FluidSelect = true;
                                    if(i1 == 2) GasSelect   = true;
                                    suppressClick = true;
                                } else{
                                    hoverDisable();
                                }
                            }
                        }
                        // type selected, menu not yet opened
                        else if(mx >= xs && mx <= xs + buttonWidth /PIXEL_SIZE && 
                                my >= ys && my <= ys + buttonHeight/PIXEL_SIZE){
                            hoverDisable();
                            stateDisable();
                            selectHover (i1);
                            suppressClick = true;
                        }
                    }
                }
                
                // CLEAR ALL
                float x = (WINDOW_WIDTH_PXS-(buttonWidth+padding)*3)/PIXEL_SIZE;
                float y = padding/PIXEL_SIZE;
                if(mx >= x && mx < x + buttonWidth*1.6 /PIXEL_SIZE && 
                    my >= y && my < y + buttonHeight/PIXEL_SIZE){
                    grid.clear();
                    grid.resize(GRID_HEIGHT*GRID_WIDTH);
                    clearSelect = true;
                }
                
                // FREEZE ALL
                x = (WINDOW_WIDTH_PXS-(buttonWidth+padding)*4.5)/PIXEL_SIZE;
                y = padding/PIXEL_SIZE;
                if(mx >= x && mx < x + buttonWidth*1.6 /PIXEL_SIZE && 
                    my >= y && my < y + buttonHeight/PIXEL_SIZE){
                    Update::freezeSelect = !Update::freezeSelect;
                }
                break;
            }
            case sf::Event::MouseButtonReleased:
                suppressClick = false;
        }
    }
    if(!suppressClick)
        mouseInput();

    if(!Update::freezeSelect) {
        for(int i = 0; i < grid.size(); i++) {
            grid[i].Update(grid, i);
        }   
    }
    particles.clear();
    for(int y = 0; y < GRID_HEIGHT; y++) {
        for(int x = 0; x < GRID_WIDTH; x++) {
            Particle& pt = grid[x + y * GRID_WIDTH];

            ParticleType  type           = pt.type;
            ParticleState state          = pt.state;
            std::pair<float,float> pxy   = pt.getCoord();
            std::vector<short>     color = pt.color;
            sf::Color  c(color[0], color[1], color[2]);
            float px =                 pxy.first  * PIXEL_SIZE;
            float py = UI_HEIGHT_PXS + pxy.second * PIXEL_SIZE;
            if(type == ParticleType::None)
                continue;

            particles.append(sf::Vertex({px, py}, c));                             // top-left
            particles.append(sf::Vertex({px + PIXEL_SIZE, py}, c));                // top-right
            particles.append(sf::Vertex({px + PIXEL_SIZE, py + PIXEL_SIZE}, c));   // bottom-right
            particles.append(sf::Vertex({px, py + PIXEL_SIZE}, c));                // bottom-left
        }
    }   
}

/*
    Selection checkers and disablers
*/
bool Game::checkIfHoverSelect(int i){
    if(i == 0 && SolidHoverSelect) return true;
    if(i == 1 && FluidHoverSelect) return true;
    if(i == 2 && GasHoverSelect  ) return true;
    return false;
}
bool Game::checkIfStateSelect(int i){
    if(i == 0 && SolidSelect) return true;
    if(i == 1 && FluidSelect) return true;
    if(i == 2 && GasSelect  ) return true;
    return false;
}

void Game::hoverDisable(){
    SolidHoverSelect = false;
    FluidHoverSelect = false;
    GasHoverSelect   = false;
}
void Game::stateDisable(){
    SolidSelect = false;
    FluidSelect = false;
    GasSelect   = false;
}
void Game::selectHover(int i){
    switch (i)
    {
    case 0:
        SolidHoverSelect = true;
        SolidSelect      = true;
        return;
    
    case 1:
        FluidHoverSelect = true;
        FluidSelect      = true;
        return;
    
    case 2:
        GasHoverSelect = true;
        GasSelect      = true;
        return;
    }
}

/*
    Draw
*/

void Game::render(){
   // printf("%d\n", particleCount);
    window->clear(screenColor); 
    window->draw(particles);
    drawUI();
    window->display();
}
void Game::updateUIScaling() {
    padding      = PIXEL_SIZE * 2;
    buttonWidth  = PIXEL_SIZE * 5;
    buttonHeight = PIXEL_SIZE * 3;
    UI_HEIGHT_PXS = UI_HEIGHT_CELL * (PIXEL_SIZE + 1);
}

void Game::drawUI(){
    sf::Text brushText;
    brushText.setFont(font);
    brushText.setString("Brush: " + std::to_string(brushSize*2+1));
    brushText.setCharacterSize(defaultCharSize);
    brushText.setFillColor(sf::Color::White);
    brushText.setPosition(WINDOW_WIDTH_PXS-80, UI_HEIGHT_PXS - 20);

    window->draw(brushText);
    drawBorder();
    drawButton();
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

    border.setPosition(0, HEIGHT - UI_HEIGHT_PXS-PIXEL_SIZE*2);
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
    for(int i1 = 0; i1 < Particle::states_list.size(); i1++){
        // hover menu closed
        if(!checkIfHoverSelect(i1)){
            sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
            button.setPosition(padding+i1*(buttonWidth+padding), padding);

            if(checkIfStateSelect(i1)) button.setFillColor(sf::Color(200, 200, 100));
            else                       button.setFillColor(sf::Color(150, 150, 150));
            button.setOutlineColor(sf::Color::Black);
            button.setOutlineThickness(2);

            sf::Text text;
            text.setFont(font);
            if(checkIfStateSelect(i1))
                text.setString(Particle::getTypeAsString(selectedType));

            else  
                text.setString(Particle::states_list[i1]);

            text.setCharacterSize(defaultCharSize);
            text.setFillColor(sf::Color::Black);
            text.setPosition(button.getPosition().x + PIXEL_SIZE/2, button.getPosition().y + PIXEL_SIZE/2);

            window->draw(button);
            window->draw(text);
            continue;
        }
        // Hover menu opened
        for(int i2 = 0; i2 < Particle::type_list[i1].size(); i2++){
            sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
            button.setPosition(padding+i1*(buttonWidth+padding), padding+i2*buttonHeight);

            if(Particle::types[i1][i2] == selectedType){
                button.setFillColor(sf::Color(200, 200, 100));
            }
            else{
                button.setFillColor(sf::Color(150, 150, 150));
            }
            button.setOutlineColor(sf::Color::Black);
            button.setOutlineThickness(2);

            sf::Text text;
            text.setFont(font);
            text.setString(Particle::type_list[i1][i2]);
            text.setCharacterSize(defaultCharSize);
            text.setFillColor(sf::Color::Black);
            text.setPosition(button.getPosition().x + PIXEL_SIZE/2, button.getPosition().y + PIXEL_SIZE/2);
            window->draw(button);
            window->draw(text);
        }
    }
    /*
        * * * CLEAR ALL  * * *
    */
    {
    sf::RectangleShape buttonClearAll(sf::Vector2f(buttonWidth*1.6, buttonHeight));
    buttonClearAll.setPosition(WINDOW_WIDTH_PXS-(buttonWidth+padding)*3, padding);
    if(clearSelect)buttonClearAll.setFillColor(sf::Color(200, 200, 100));
    else buttonClearAll.setFillColor(sf::Color(150, 150, 150));
    buttonClearAll.setOutlineColor(sf::Color::Black);
    buttonClearAll.setOutlineThickness(2);

    sf::Text textClearAll;
    textClearAll.setFont(font);
    textClearAll.setString("CLEAR ALL");
    textClearAll.setCharacterSize(defaultCharSize);
    textClearAll.setFillColor(sf::Color::Black);
    textClearAll.setPosition(buttonClearAll.getPosition().x + 10, buttonClearAll.getPosition().y + 10);
    window->draw(buttonClearAll);
    window->draw(textClearAll);
    }

    /*
        * * *FREEZE ALL * * *
    */
    {
    sf::RectangleShape buttonFreezeAll(sf::Vector2f(buttonWidth*1.6, buttonHeight));
    buttonFreezeAll.setPosition(WINDOW_WIDTH_PXS-(buttonWidth+padding)*4.5, padding);
    if(Update::freezeSelect) buttonFreezeAll.setFillColor(sf::Color(200, 200, 100));
    else buttonFreezeAll.setFillColor(sf::Color(150, 150, 150));
    buttonFreezeAll.setOutlineColor(sf::Color::Black);
    buttonFreezeAll.setOutlineThickness(2);

    sf::Text textFreezeAll;
    textFreezeAll.setFont(font);
    textFreezeAll.setString("FREEZE ALL");
    textFreezeAll.setCharacterSize(defaultCharSize);
    textFreezeAll.setFillColor(sf::Color::Black);
    textFreezeAll.setPosition(buttonFreezeAll.getPosition().x + 10, buttonFreezeAll.getPosition().y + 10);
    window->draw(buttonFreezeAll);
    window->draw(textFreezeAll);}   
    
    /*
        BOTTOM BUTTONS
    */

    float bottomY = HEIGHT - PIXEL_SIZE - buttonHeight - padding;

    for(int i = 0; i < 3; i++){
        sf::RectangleShape buttonBottom(sf::Vector2f(buttonWidth*1.6f, buttonHeight));
        buttonBottom.setPosition(padding + i*(buttonWidth+2*padding), bottomY);
        if(Update::freezeSelect) buttonBottom.setFillColor(sf::Color(200, 200, 100));
        else buttonBottom.setFillColor(sf::Color(150, 150, 150));
        buttonBottom.setOutlineColor(sf::Color::Black);
        buttonBottom.setOutlineThickness(2);

        sf::Text buttonBottomText;
        buttonBottomText.setFont(font);
        buttonBottomText.setString("DUMMY");
        buttonBottomText.setCharacterSize(defaultCharSize);
        buttonBottomText.setFillColor(sf::Color::Black);
        buttonBottomText.setPosition(buttonBottom.getPosition().x + 10, buttonBottom.getPosition().y + 10);
        window->draw(buttonBottom);
        window->draw(buttonBottomText);
    }
}