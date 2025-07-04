#include "src/game.h"
#include <iostream>

int main()
{
    int i;
    Game game;

    while(game.isRunning()){
        game.update();
        game.render();
    }
}