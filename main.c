#include "game.h"
#include "snek.h"
#include <stdio.h>

int main(void)
{
    game_init(GAME_DEFAULT_CONFIG);
    game_run();
    game_exit();
}
