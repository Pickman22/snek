#include "game.h"
#include "raylib.h"
#include "snek.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr)-offsetof(type, member)))

#define INPUT_FLAG_NONE          (0u)
#define INPUT_FLAG_LEFT          (1u)
#define INPUT_FLAG_RIGHT         (2u)
#define INPUT_FLAG_UP            (4u)
#define INPUT_FLAG_DOWN          (8u)
#define INPUT_FLAG_QUIT          (16u)
#define INPUT_FLAG_CONTINUE      (32u)
#define INPUT_FLAG_DIR_MASK      (0xFu)
#define INPUT_FLAG_GAME_CTRL_MSK (0x30u)

#define IS_GAME_QUIT(x)     (((x) & INPUT_FLAG_QUIT) || WindowShouldClose())
#define IS_GAME_CONTINUE(x) ((x) & INPUT_FLAG_CONTINUE)

#define fruit_alloc()    (struct fruit *)malloc(sizeof(struct fruit))
#define fruit_dealloc(x) free((x))

#define MAX_NUM_LISTENERS (8u)

enum game_evt
{
    GAME_EVT_NONE = 0,
    GAME_EVT_SNEK_EATS,
    GAME_EVT_GAME_OVER,
    GAME_EVT_NUM,
};

enum window_state
{
    WINDOW_STATE_CONTINUE = 0,
    WINDOW_STATE_QUIT,
    WINDOW_STATE_NUM,
};

struct fruit
{
    struct snek_point pos;
};

struct game_ctx;

typedef enum window_state (*game_window)(void);

struct notification
{
    void (*notify)(struct notification *notify);
};

struct listener
{
    struct notification *notification;
    enum game_evt evt;
};

struct game_over_listener
{
    struct notification notification;
    Sound game_over_sound;
    Music game_music;
};

struct snek_eat_listener
{
    struct notification notification;
    struct snek *snek;
    struct fruit *fruit;
    Sound bite_sound;
};

struct game_ctx
{
    game_window active_window;
    unsigned int input_flags;
    bool is_game_over;
    int score;
    struct game_config config;
    struct snek *snek;
    struct fruit *fruit;
    Music game_music;
    Sound game_over_sound;
    Sound bite_sound;
    struct listener listeners[MAX_NUM_LISTENERS];
    struct game_over_listener game_over_listener;
    struct snek_eat_listener snek_eat_listener;
};

static struct game_ctx game_ctx;
static void draw_begin(void);
static void draw_end(void);
static void update_snek(struct snek *snek, unsigned int dir_flags);
static void draw_snek(const struct snek *snek);
static void draw_score(size_t score);
static void draw_grid(void);
static void draw_fruit(const struct fruit *fruit);
static void draw_game_over(void);
static void calc_fruit_new_pos(struct fruit *fruit, const struct snek *snek);
static bool is_snek_dead(const struct snek *snek);
static bool is_snek_eating(struct snek *snek, const struct fruit *fruit);
static bool is_snek_colliding(const struct snek *snek);
static unsigned int get_game_input(void);
static enum window_state snek_window_init(void);
static enum window_state snek_window(void);
static enum window_state game_over_window(void);
static void game_notify(enum game_evt game_evt);
static void game_listen(struct notification *notification, enum game_evt evt);
static void game_over_event(struct notification *notification);
static void snek_eat_event(struct notification *notification);

void game_init(struct game_config config)
{
    game_ctx = (struct game_ctx){0};
    game_ctx.config = config;
    InitAudioDevice();
    while (!IsAudioDeviceReady())
        ;

    game_ctx.game_music = LoadMusicStream("sounds/music.wav");
    game_ctx.game_over_sound = LoadSound("sounds/game_over.wav");
    game_ctx.bite_sound = LoadSound("sounds/bite.wav");

    SetMasterVolume(0.5f);
    InitWindow(game_ctx.config.width, game_ctx.config.height,
               game_ctx.config.title);
    SetTargetFPS(game_ctx.config.fps);
    srand(time(NULL));
    game_ctx.active_window = snek_window_init;
    game_ctx.snek = snek_alloc();
    game_ctx.fruit = fruit_alloc();

    /* Ideally this should be it's own "objects". */
    game_ctx.game_over_listener = (struct game_over_listener){
        .notification = {.notify = game_over_event},
        .game_over_sound = game_ctx.game_over_sound,
        .game_music = game_ctx.game_music,
    };

    /* Ideally this should be it's own "objects". */
    game_ctx.snek_eat_listener = (struct snek_eat_listener){
        .notification = {.notify = snek_eat_event},
        .snek = game_ctx.snek,
        .fruit = game_ctx.fruit,
        .bite_sound = game_ctx.bite_sound,
    };

    game_listen(&game_ctx.game_over_listener.notification, GAME_EVT_GAME_OVER);
    game_listen(&game_ctx.snek_eat_listener.notification, GAME_EVT_SNEK_EATS);
}

void game_run(void)
{
    enum window_state state;
    do {
        game_ctx.input_flags = get_game_input();
        state = game_ctx.active_window();
    } while (state != WINDOW_STATE_QUIT && !IS_GAME_QUIT(game_ctx.input_flags));
}

enum window_state game_over_window(void)
{
    draw_begin();
    draw_grid();
    draw_game_over();
    draw_end();
    if (IS_GAME_CONTINUE(game_ctx.input_flags)) {
        game_ctx.active_window = snek_window_init;
        StopSound(game_ctx.game_over_sound);
    }
    return WINDOW_STATE_CONTINUE;
}

enum window_state snek_window_init(void)
{
    snek_init(game_ctx.snek,
              (struct snek_point){
                  .x = game_ctx.config.width / 2 / game_ctx.config.grid_width,
                  .y = game_ctx.config.height / 2 / game_ctx.config.grid_width,
              });

    calc_fruit_new_pos(game_ctx.fruit, game_ctx.snek);
    game_ctx.is_game_over = false;
    game_ctx.score = 0u;

    /* Start running game! */
    game_ctx.active_window = snek_window;
    PlayMusicStream(game_ctx.game_music);
    return WINDOW_STATE_CONTINUE;
}

enum window_state snek_window(void)
{
    UpdateMusicStream(game_ctx.game_music);
    draw_begin();
    update_snek(game_ctx.snek, game_ctx.input_flags & INPUT_FLAG_DIR_MASK);
    if (is_snek_eating(game_ctx.snek, game_ctx.fruit)) {
        game_notify(GAME_EVT_SNEK_EATS);
    }
    draw_grid();
    draw_fruit(game_ctx.fruit);
    draw_snek(game_ctx.snek);
    draw_score(game_ctx.score);
    draw_end();
    if (is_snek_dead(game_ctx.snek)) {
        game_notify(GAME_EVT_GAME_OVER);
    }

    return WINDOW_STATE_CONTINUE;
}

static void game_notify(enum game_evt evt)
{
    for (size_t i = 0; i < MAX_NUM_LISTENERS; i++) {
        if (evt == game_ctx.listeners[i].evt) {
            game_ctx.listeners[i].notification->notify(
                game_ctx.listeners[i].notification);
        }
    }
}

static void game_listen(struct notification *notification, enum game_evt evt)
{
    for (size_t i = 0; i < MAX_NUM_LISTENERS; i++) {
        if (game_ctx.listeners[i].notification == NULL) {
            game_ctx.listeners[i].notification = notification;
            game_ctx.listeners[i].evt = evt;
            break;
        }
    }
}

void game_exit(void)
{
    snek_dealloc(game_ctx.snek);
    fruit_dealloc(game_ctx.fruit);
    CloseWindow();
    CloseAudioDevice();
    UnloadMusicStream(game_ctx.game_music);
    UnloadSound(game_ctx.game_over_sound);
    UnloadSound(game_ctx.bite_sound);
}

static void draw_begin(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
}

static void draw_end(void)
{
    EndDrawing();
}

static void draw_grid(void)
{
    for (int pos_x = 0; pos_x < game_ctx.config.width;
         pos_x += game_ctx.config.grid_width) {
        DrawLine(pos_x, 0, pos_x, game_ctx.config.height, LIGHTGRAY);
    }

    for (int pos_y = 0; pos_y < game_ctx.config.height;
         pos_y += game_ctx.config.grid_width) {
        DrawLine(0, pos_y, game_ctx.config.width, pos_y, LIGHTGRAY);
    }
}

static bool is_snek_dead(const struct snek *snek)
{
    bool snek_is_dead = false;

    /* Check for snek out of bounds. */
    int max_pos_x = game_ctx.config.width / game_ctx.config.grid_width;
    int max_pos_y = game_ctx.config.height / game_ctx.config.grid_width;

    struct snek_point head_pos = snek_get_pos(snek, 0u);

    if (head_pos.x < 0 || head_pos.x >= max_pos_x || head_pos.y < 0 ||
        head_pos.y >= max_pos_y || is_snek_colliding(snek)) {
        snek_is_dead = true;
    }

    return snek_is_dead;
}

static bool is_snek_colliding(const struct snek *snek)
{
    bool is_colliding = false;
    struct snek_point head_pos = snek_get_pos(snek, 0u);
    struct snek_point body_pos;

    for (size_t i = 1; i < snek_get_len(snek); i++) {
        body_pos = snek_get_pos(snek, i);
        if (body_pos.x == head_pos.x && body_pos.y == head_pos.y) {
            is_colliding = true;
            break;
        }
    }
    return is_colliding;
}

static bool is_snek_eating(struct snek *snek, const struct fruit *fruit)
{
    struct snek_point snek_pos = snek_get_pos(snek, 0);
    return (snek_pos.x == fruit->pos.x && snek_pos.y == fruit->pos.y);
}

static void calc_fruit_new_pos(struct fruit *fruit, const struct snek *snek)
{
    bool valid_fruit_pos = false;
    struct snek_point snek_pos;
    int max_pos_x = game_ctx.config.width / game_ctx.config.grid_width;
    int max_pos_y = game_ctx.config.height / game_ctx.config.grid_width;

    while (valid_fruit_pos != true) {

        fruit->pos.x = rand() % max_pos_x;
        fruit->pos.y = rand() % max_pos_y;

        for (size_t i = 0; i < snek_get_len(snek); i++) {
            snek_pos = snek_get_pos(snek, i);
            if (snek_pos.x == fruit->pos.x && snek_pos.y == fruit->pos.y) {
                /* Try a new position. */
                break;
            }
        }

        /* If loop ends, a valid position was generated. */
        valid_fruit_pos = true;
    }
}

static void draw_fruit(const struct fruit *fruit)
{
    DrawRectangle(game_ctx.config.grid_width * fruit->pos.x,
                  game_ctx.config.grid_width * fruit->pos.y,
                  game_ctx.config.grid_width, game_ctx.config.grid_width,
                  game_ctx.config.fruit_color);
}

static void update_snek(struct snek *snek, unsigned int dir_flags)
{
    enum snek_move snek_dir = SNEK_MOVE_NONE;
    static int loop_count;

    snek_dir = (dir_flags & INPUT_FLAG_LEFT) ? SNEK_MOVE_LEFT : snek_dir;
    snek_dir = (dir_flags & INPUT_FLAG_RIGHT) ? SNEK_MOVE_RIGHT : snek_dir;
    snek_dir = (dir_flags & INPUT_FLAG_UP) ? SNEK_MOVE_UP : snek_dir;
    snek_dir = (dir_flags & INPUT_FLAG_DOWN) ? SNEK_MOVE_DOWN : snek_dir;
    snek_set_move_dir(snek, snek_dir);
    if (loop_count++ > (game_ctx.config.fps / game_ctx.config.snek_speed)) {
        snek_move(snek);
        loop_count = 0;
    }
}

static void draw_snek(const struct snek *snek)
{

    struct snek_point pos;

    for (size_t i = 0; i < snek_get_len(snek); i++) {
        pos = snek_get_pos(snek, i);
        pos.x = game_ctx.config.grid_width * pos.x;
        pos.y = game_ctx.config.grid_width * pos.y;
        DrawRectangle(pos.x, pos.y, game_ctx.config.grid_width,
                      game_ctx.config.grid_width, game_ctx.config.snek_color);
    }
}

static void draw_score(size_t score)
{
    char strbuf[128] = {0};
    sprintf(strbuf, "Score: %lu", score);
    DrawText(strbuf, game_ctx.config.font_size,
             game_ctx.config.height - 2 * game_ctx.config.font_size,
             game_ctx.config.font_size, game_ctx.config.score_color);
}

static void draw_game_over(void)
{
    const char exit_msg[] = "Press Q to quit. Press Enter to play again.";
    const char game_over_msg[] = "Game Over...";
    const int blink_timeout_freq = 2;
    static int timer = 0;
    int pos_x, pos_y;

    if (timer++ < (game_ctx.config.fps / blink_timeout_freq)) {
        pos_x = game_ctx.config.width -
                MeasureText(game_over_msg, game_ctx.config.large_font_size);
        pos_y = game_ctx.config.height - game_ctx.config.large_font_size;
        DrawText(game_over_msg, pos_x / 2, pos_y / 2,
                 game_ctx.config.large_font_size, RED);
    }
    else if (timer >= (game_ctx.config.fps / blink_timeout_freq) &&
             timer < game_ctx.config.fps) {
        /* Do not draw anything */
    }
    else {
        timer = 0;
    }

    pos_x = game_ctx.config.width -
            MeasureText(exit_msg, game_ctx.config.font_size);
    pos_y = game_ctx.config.height + 1.4 * game_ctx.config.large_font_size;
    DrawText(exit_msg, pos_x / 2, pos_y / 2, game_ctx.config.font_size, GRAY);
}

static unsigned int get_game_input(void)
{
    unsigned int input_flags = 0u;
    input_flags |= IsKeyPressed(KEY_Q) * INPUT_FLAG_QUIT;
    input_flags |= IsKeyPressed(KEY_ENTER) * INPUT_FLAG_CONTINUE;

    /* Down and Up key-window directions are reversed. */
    input_flags |= IsKeyPressed(KEY_DOWN) * INPUT_FLAG_UP;
    input_flags |= IsKeyPressed(KEY_UP) * INPUT_FLAG_DOWN;

    input_flags |= IsKeyPressed(KEY_LEFT) * INPUT_FLAG_LEFT;
    input_flags |= IsKeyPressed(KEY_RIGHT) * INPUT_FLAG_RIGHT;
    return input_flags;
}

static void game_over_event(struct notification *notif)
{
    struct game_over_listener *listener =
        CONTAINER_OF(notif, struct game_over_listener, notification);

    game_ctx.active_window = game_over_window;
    game_ctx.is_game_over = true;
    StopMusicStream(listener->game_music);
    PlaySound(listener->game_over_sound);
}

static void snek_eat_event(struct notification *notif)
{
    struct snek_eat_listener *listener =
        CONTAINER_OF(notif, struct snek_eat_listener, notification);
    PlaySound(listener->bite_sound);
    snek_eat(listener->snek);
    calc_fruit_new_pos(listener->fruit, listener->snek);
    game_ctx.score =
        game_ctx.config.food_score_value * (snek_get_len(listener->snek) - 1);
}
