#ifndef GAME_H
#define GAME_H

#include "raylib.h"

struct game_config
{
    int width;
    int height;
    int fps;
    const char *title;
    int grid_width;
    int grid_line_width;
    Color snek_color;
    Color fruit_color;
    Color grid_color;
    int grid_size;
    int snek_speed;
    int font_size;
    int large_font_size;
    int food_score_value;
    Color score_color;
};

#define GAME_DEFAULT_CONFIG                                              \
    (struct game_config)                                                 \
    {                                                                    \
        .fps = 60, .width = 800, .height = 600, .title = "Snek Game!",   \
        .snek_color = LIME, .fruit_color = RED, .grid_color = RAYWHITE,  \
        .grid_line_width = 2, .grid_width = 20, .font_size = 20,         \
        .large_font_size = 60, .snek_speed = 20, .food_score_value = 10, \
        .score_color = GRAY                                              \
    }

void game_init(struct game_config config);

void game_run(void);

void game_exit(void);

#endif // !GAME_H
