#include "../include/game.hpp"
/*
    SCREEN MANAGEMENT
    DRAWING
    UPDATE&RENDER
*/
int FPS = 60;

      int PIXEL_SIZE         =  8;
      int DEFAULT_PIXEL_SIZE =  8;
const int PIXEL_SIZE_MIN     =  4;
const int PIXEL_SIZE_MAX     = 32;

const int UI_HEIGHT_CELL = 6;

int UI_HEIGHT_PXS     = UI_HEIGHT_CELL * DEFAULT_PIXEL_SIZE; 
int SIM_HEIGHT_PXS    = GRID_HEIGHT    * DEFAULT_PIXEL_SIZE; 
int SIM_WIDTH_PXS     = GRID_WIDTH     * DEFAULT_PIXEL_SIZE; 
int WINDOW_HEIGHT_PXS = SIM_HEIGHT_PXS * DEFAULT_PIXEL_SIZE; 
int WINDOW_WIDTH_PXS  = SIM_WIDTH_PXS; 

float padding      = DEFAULT_PIXEL_SIZE;
float buttonWidth  = DEFAULT_PIXEL_SIZE * 5;
float buttonHeight = DEFAULT_PIXEL_SIZE * 3;

float bottomY;

unsigned int defaultCharSize = DEFAULT_PIXEL_SIZE;

bool clearSelect = false;

bool SolidSelect = false;
bool FluidSelect = false;
bool GasSelect   = false;
bool SolidHoverSelect = false;
bool FluidHoverSelect = false;
bool GasHoverSelect   = false;

bool suppressClick = false;

float k;

void Game::calculateWindowSize(){
    updateUIScaling();
    k = DEFAULT_PIXEL_SIZE/(float)PIXEL_SIZE;
    UI_HEIGHT_PXS     = UI_HEIGHT_CELL * DEFAULT_PIXEL_SIZE; 
    SIM_HEIGHT_PXS    = GRID_HEIGHT    * PIXEL_SIZE; 
    SIM_WIDTH_PXS     = GRID_WIDTH     * PIXEL_SIZE; 
    WINDOW_HEIGHT_PXS = SIM_HEIGHT_PXS + UI_HEIGHT_PXS*2+DEFAULT_PIXEL_SIZE; 
    WINDOW_WIDTH_PXS  = SIM_WIDTH_PXS; 
    HEIGHT = WINDOW_HEIGHT_PXS;
    WIDTH  = WINDOW_WIDTH_PXS;
    pxs.resize(GRID_WIDTH * GRID_HEIGHT * 4);
    bottomY = HEIGHT - DEFAULT_PIXEL_SIZE - buttonHeight - padding;
    defaultCharSize = DEFAULT_PIXEL_SIZE;
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
    sf::Image logo;
    logo.loadFromFile("../logo.png");
    sf::ContextSettings settings;
    settings.antialiasingLevel = 0;
    calculateWindowSize();
    window = new sf::RenderWindow(sf::VideoMode(WIDTH, HEIGHT), "SANDBOX", sf::Style::Default, settings);
    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
    window->setVerticalSyncEnabled(true);
    window->setIcon(logo.getSize().x,logo.getSize().y, logo.getPixelsPtr());
}

void Game::initVar(){
    // Defaults
    GRID_HEIGHT = 64; // visible grid shorter by 2
    GRID_WIDTH  = 64;

    brushSize = 2;
    particleCount = 0;
    freezeSelect = false;
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
        sf::Vector2i currPos = sf::Mouse::getPosition(*window);
        sf::Vector2f overPos = window->mapPixelToCoords(currPos); 
        int mx = overPos.x / PIXEL_SIZE;
        int my = (overPos.y - UI_HEIGHT_PXS) / PIXEL_SIZE;
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
                    if(grid[index].getType() == ParticleType::None || 
                                   type == ParticleType::None){
                        if(type==ParticleType::None && 
                            grid[index].getType() != ParticleType::None)
                            particleCount--;
                        else if(type != ParticleType::None &&
                            grid[index].getType() == ParticleType::None)
                            particleCount++;

                        Particle prt (selectedType);
                        prt.setArgs(type, 0, 0, 0, 0, Particle::getTempByType(selectedType));
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
    //printf("%d\n", particleCount);
    clearSelect = false;
    window->setFramerateLimit(FPS);
    while(window->pollEvent(evt)){
        switch (evt.type)
            {
            case sf::Event::KeyPressed:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    brushSize = std::min(10, brushSize+1);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  brushSize = std::max(0 , brushSize-1);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) freezeSelect = !freezeSelect;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::X)){
                    DEFAULT_PIXEL_SIZE = std::min(PIXEL_SIZE_MAX, DEFAULT_PIXEL_SIZE *2);
                    PIXEL_SIZE = std::min(PIXEL_SIZE_MAX, PIXEL_SIZE *2);
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)){
                    DEFAULT_PIXEL_SIZE = std::max(PIXEL_SIZE_MIN, DEFAULT_PIXEL_SIZE / 2);
                    PIXEL_SIZE = std::max(PIXEL_SIZE_MIN, PIXEL_SIZE / 2);
                    calculateWindowSize();
                    window->setSize(sf::Vector2u(WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS));
                    window->setView(sf::View(sf::FloatRect(0, 0, WINDOW_WIDTH_PXS, WINDOW_HEIGHT_PXS)));
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window->close();
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::K))resizeGrid(0);
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::L))resizeGrid(1);
                break;
            case sf::Event::Closed:
                window->close();
                break;
            case sf::Event::MouseButtonPressed:{
                int mx = std::floor(evt.mouseButton.x / DEFAULT_PIXEL_SIZE);
                int my = std::floor(evt.mouseButton.y / DEFAULT_PIXEL_SIZE);
                if(evt.mouseButton.button == sf::Mouse::Left){
                    for(int i1 = 0; i1 < Particle::states_list.size(); i1++){
                        float xs = (padding + i1*(buttonWidth+padding))/DEFAULT_PIXEL_SIZE;
                        float ys =  padding                            /DEFAULT_PIXEL_SIZE;

                        if(checkIfHoverSelect(i1)){
                            for(int i2 = 0; i2 < Particle::type_list[i1].size(); i2++){
                                float x = (padding + i1*(buttonWidth+padding))/DEFAULT_PIXEL_SIZE;
                                float y = (padding + i2*buttonHeight         )/DEFAULT_PIXEL_SIZE;

                                if(mx >= x && mx <= x + buttonWidth /DEFAULT_PIXEL_SIZE && 
                                   my >= y && my <= y + buttonHeight/DEFAULT_PIXEL_SIZE){
                                    selectedType = Particle::types[i1][i2];

                                    stateSelect(i1);
                                    hoverDisable();
                                    suppressClick = true;
                                } else{
                                    hoverDisable();
                                }
                            }
                        }
                        // type selected, menu not yet opened
                        else if(mx >= xs && mx <= xs + buttonWidth /DEFAULT_PIXEL_SIZE && 
                                my >= ys && my <= ys + buttonHeight/DEFAULT_PIXEL_SIZE && 
                                my >= ys && my <= ys + buttonHeight/DEFAULT_PIXEL_SIZE){
                            hoverDisable();
                            hoverSelect (i1);
                            suppressClick = true;
                        }
                    }
                }
                
                // CLEAR ALL
                float x = (WINDOW_WIDTH_PXS-(buttonWidth+padding)*3)/DEFAULT_PIXEL_SIZE;
                float y = padding/DEFAULT_PIXEL_SIZE;
                if(mx >= x && mx < x + buttonWidth*1.6 /DEFAULT_PIXEL_SIZE && 
                   my >= y && my < y + buttonHeight    /DEFAULT_PIXEL_SIZE){
                    grid.clear();
                    grid.resize(GRID_HEIGHT*GRID_WIDTH);
                    clearSelect = true;
                }
                
                // FREEZE ALL
                x = (WINDOW_WIDTH_PXS-(buttonWidth+padding)*4.5)/DEFAULT_PIXEL_SIZE;
                y = padding/DEFAULT_PIXEL_SIZE;
                if(mx >= x && mx < x + buttonWidth*1.6 /DEFAULT_PIXEL_SIZE && 
                   my >= y && my < y + buttonHeight    /DEFAULT_PIXEL_SIZE){
                    freezeSelect = !freezeSelect;
                }
                
                /* Bottom buttons */
                for(int i = 0; i < 3; i++){
                    x = (padding + i*(buttonWidth+2*padding))/DEFAULT_PIXEL_SIZE;
                    y = bottomY                              /DEFAULT_PIXEL_SIZE;
                    if(mx >= x && mx < x + buttonWidth*1.6 /DEFAULT_PIXEL_SIZE && 
                        my >= y && my < y + buttonHeight/DEFAULT_PIXEL_SIZE){
                            if(i == 0){
                                saveCurrentState();
                            }
                            else if(i == 1){
                                
                            }
                            else if(i == 2){
                                
                            }
                    }
                }
                break;
            }
            case sf::Event::MouseButtonReleased:
                suppressClick = false;
        }
    }
    /* Ignore a click which closed the menu hovering over grid */
    if(!suppressClick) mouseInput();

    /* Update current state */
    if(!freezeSelect) {
        for(int i = 0; i < grid.size(); i++) {
            grid[i].Update(grid, i);
        }   
    }
    /* Draw current state
    */
    particles.clear();
    for(int y = 1; y < GRID_HEIGHT - 1; y++) {
        for(int x = 1; x < GRID_WIDTH - 1; x++) {
            Particle& pt = grid[x + y * GRID_WIDTH];

            ParticleType  type           = pt.getType();
            ParticleState state          = pt.getState();
            std::pair<float,float> pxy   = pt.getCoord();
            std::vector<short>     color = pt.getColor();
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

void Game::hoverSelect(int i){
    switch (i)
    {
    case 0:
        SolidHoverSelect = true;
        return;
    
    case 1:
        FluidHoverSelect = true;
        return;
    
    case 2:
        GasHoverSelect = true;
        return;
    }
}
void Game::hoverDisable(){
    SolidHoverSelect = false;
    FluidHoverSelect = false;
    GasHoverSelect   = false;
}
void Game::stateSelect(int i){
    SolidSelect = false;
    FluidSelect = false;
    GasSelect   = false;
    if(i == 0) SolidSelect = true;
    if(i == 1) FluidSelect = true;
    if(i == 2) GasSelect   = true;
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
    padding      = DEFAULT_PIXEL_SIZE * 2;
    buttonWidth  = DEFAULT_PIXEL_SIZE * 5;
    buttonHeight = DEFAULT_PIXEL_SIZE * 3;
    UI_HEIGHT_PXS = UI_HEIGHT_CELL * (DEFAULT_PIXEL_SIZE + 1);
}

void Game::drawUI(){
    drawBrushText();
    drawBorder();
    drawButton();
}
void Game::drawBrushText(){
    sf::Text brushText;
    brushText.setFont(font);
    brushText.setString("Brush: " + std::to_string(brushSize*2+1));
    brushText.setCharacterSize(defaultCharSize);
    brushText.setFillColor(sf::Color::White);
    brushText.setPosition(WINDOW_WIDTH_PXS-DEFAULT_PIXEL_SIZE*8, UI_HEIGHT_PXS - DEFAULT_PIXEL_SIZE*2);
    window->draw(brushText);
}


void Game::drawBorder(){
    sf::RectangleShape border;
    border.setFillColor(sf::Color::Black);
    
    // Top border
    border.setSize(sf::Vector2f(WIDTH, DEFAULT_PIXEL_SIZE));
    border.setPosition(0, UI_HEIGHT_PXS);
    window->draw(border);

    // Bottom border
    border.setPosition(0, HEIGHT - DEFAULT_PIXEL_SIZE);
    window->draw(border);

    border.setPosition(0, HEIGHT - UI_HEIGHT_PXS-DEFAULT_PIXEL_SIZE*2);
    window->draw(border);

    // Left border
    border.setSize(sf::Vector2f(DEFAULT_PIXEL_SIZE, HEIGHT));
    border.setPosition(0, 0);
    window->draw(border);

    // Right border
    border.setPosition(WIDTH - DEFAULT_PIXEL_SIZE, 0);
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
            text.setPosition(button.getPosition().x + DEFAULT_PIXEL_SIZE/2, button.getPosition().y + DEFAULT_PIXEL_SIZE/2);

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
            text.setPosition(button.getPosition().x + DEFAULT_PIXEL_SIZE/2, button.getPosition().y + PIXEL_SIZE/2);
            window->draw(button);
            window->draw(text);
        }
    }
    /* * * * CLEAR ALL  * * * */
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
    textClearAll.setPosition(buttonClearAll.getPosition().x + DEFAULT_PIXEL_SIZE/2, buttonClearAll.getPosition().y + DEFAULT_PIXEL_SIZE/2);
    window->draw(buttonClearAll);
    window->draw(textClearAll);
    }

    /*  * * *FREEZE ALL * * * */
    {
    sf::RectangleShape buttonFreezeAll(sf::Vector2f(buttonWidth*1.6, buttonHeight));
    buttonFreezeAll.setPosition(WINDOW_WIDTH_PXS-(buttonWidth+padding)*4.5, padding);

    if(freezeSelect) buttonFreezeAll.setFillColor(sf::Color(200, 200, 100));
    else buttonFreezeAll.setFillColor(sf::Color(150, 150, 150));
    
    buttonFreezeAll.setOutlineColor(sf::Color::Black);
    buttonFreezeAll.setOutlineThickness(2);

    sf::Text textFreezeAll;
    textFreezeAll.setFont(font);
    textFreezeAll.setString("FREEZE ALL");
    textFreezeAll.setCharacterSize(defaultCharSize);
    textFreezeAll.setFillColor(sf::Color::Black);
    textFreezeAll.setPosition(buttonFreezeAll.getPosition().x + DEFAULT_PIXEL_SIZE/2, buttonFreezeAll.getPosition().y + DEFAULT_PIXEL_SIZE/2);
    window->draw(buttonFreezeAll);
    window->draw(textFreezeAll);}   
    
    /*
        BOTTOM BUTTONS
    */

    for(int i = 0; i < 1; i++){
        sf::RectangleShape buttonBottom(sf::Vector2f(buttonWidth*1.6f, buttonHeight));
        buttonBottom.setPosition(padding + i*(buttonWidth+2*padding), bottomY);
        buttonBottom.setFillColor(sf::Color(150, 150, 150));
        buttonBottom.setOutlineColor(sf::Color::Black);
        buttonBottom.setOutlineThickness(2);

        sf::Text buttonBottomText;
        buttonBottomText.setFont(font);
        if(i == 0) buttonBottomText.setString("SCREENSHOT");
      //  if(i == 1) buttonBottomText.setString("DUM");
       // if(i == 2) buttonBottomText.setString("DUM");
        buttonBottomText.setCharacterSize(defaultCharSize);
        buttonBottomText.setFillColor(sf::Color::Black);
        buttonBottomText.setPosition(buttonBottom.getPosition().x + DEFAULT_PIXEL_SIZE/2, buttonBottom.getPosition().y + DEFAULT_PIXEL_SIZE/2);
        window->draw(buttonBottom);
        window->draw(buttonBottomText);
    }
}

/* Screenshotting
*/
sf::Color noneColor {Particle::getColorByType(ParticleType::None)[0],
                         Particle::getColorByType(ParticleType::None)[1],
                         Particle::getColorByType(ParticleType::None)[2]};
void Game::saveCurrentState(){
    sf::RenderTexture renderTexture;
    renderTexture.create(window->getSize().x, window->getSize().y);

    renderTexture.clear(noneColor);
    renderTexture.draw(particles);
    renderTexture.display();

    sf::Texture texture    = renderTexture.getTexture();
    sf::Image   screenshot = texture.copyToImage();
   
    int gridTop    = UI_HEIGHT_PXS;  
    int gridHeight = GRID_HEIGHT*PIXEL_SIZE;
    int gridWidth  = GRID_WIDTH*PIXEL_SIZE;

    sf::Image cropped;
    cropped.create(gridWidth, gridHeight);

    for (int y = 1; y < gridHeight - 1; ++y) {
        for (int x = 1; x < gridWidth - 1; ++x) {
            cropped.setPixel(x, y, screenshot.getPixel(x, y + gridTop));
        }
    }
    int i = 1;
    while(std::filesystem::exists("../pic/screenshot"+std::to_string(i)+".png"))
        i++;
    cropped.saveToFile("../pic/screenshot"+std::to_string(i)+".png");
}

void Game::resizeGrid(bool scaleDown){
    std::vector<Particle> reservedGrid = grid;
    int newHeight = GRID_HEIGHT;
    int newWidth  = GRID_WIDTH;
    if(scaleDown){
        if(PIXEL_SIZE == 2) return;
        newHeight *= 2;
        newWidth  *= 2;
        PIXEL_SIZE /= 2;
    }
    else{
        if(newHeight == 2 || newWidth == 2) return;
        newHeight /= 2;
        newWidth  /= 2;
        PIXEL_SIZE *= 2;
    }
    grid.clear();
    grid.resize(newHeight*newWidth+2);

    for(int x = 0; x < std::min(newWidth, GRID_WIDTH); x++){
        for(int y = 0; y < std::min(newHeight, GRID_HEIGHT); y++){
            grid[x + (newHeight - 1 - y)*newWidth] = 
                        reservedGrid[x + (GRID_HEIGHT - 1 - y) * GRID_WIDTH];
            grid[x + (newHeight - 1 - y)*newWidth].setCoord(x, newHeight - 1 - y);
        }
    }
    GRID_WIDTH  = newWidth;
    GRID_HEIGHT = newHeight;
    calculateWindowSize();
}