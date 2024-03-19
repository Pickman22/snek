#include "snek.h"
#include <stdio.h>

#define SNEK_MIN(x, y) ((x) < (y)) ? (x) : (y);

static const struct snek_point snek_move_map[SNEK_MOVE_NUM] = {
    [SNEK_MOVE_NONE] = {.x = 0, .y = 0},  [SNEK_MOVE_DOWN] = {.x = 0, .y = -1},
    [SNEK_MOVE_UP] = {.x = 0, .y = 1},    [SNEK_MOVE_LEFT] = {.x = -1, .y = 0},
    [SNEK_MOVE_RIGHT] = {.x = 1, .y = 0},
};

static const enum snek_move snek_dir_not_ok_map[SNEK_MOVE_NUM] = {
    [SNEK_MOVE_NONE] = SNEK_MOVE_NONE,  [SNEK_MOVE_UP] = SNEK_MOVE_DOWN,
    [SNEK_MOVE_DOWN] = SNEK_MOVE_UP,    [SNEK_MOVE_LEFT] = SNEK_MOVE_RIGHT,
    [SNEK_MOVE_RIGHT] = SNEK_MOVE_LEFT,
};

static bool is_move_dir_possible(enum snek_move dir, enum snek_move tgt_dir);

struct snek
{
    enum snek_move move_dir;
    size_t len;
    struct snek_point body[SNEK_BODY_MAX_SIZE];
};

size_t snek_get_size(void)
{
    return sizeof(struct snek);
}

void snek_init(struct snek *snek, struct snek_point init_pos)
{
    snek->len = 1u;
    snek->move_dir = SNEK_MOVE_NONE;
    snek->body[0u] = init_pos;
}

bool is_move_dir_possible(enum snek_move dir, enum snek_move tgt_dir)
{
    return snek_dir_not_ok_map[dir] != tgt_dir;
}

void snek_set_move_dir(struct snek *snek, enum snek_move move)
{
    if (move == SNEK_MOVE_NONE) {
        /* Ignore. Continue moving in same direction. */
        return;
    }
    else if (move < SNEK_MOVE_NUM) {
        snek->move_dir = (is_move_dir_possible(snek->move_dir, move))
                             ? move
                             : snek->move_dir;
    }
    else {
        puts("Invalid moving direction.\n\r");
    }
}

void snek_move(struct snek *snek)
{
    if (snek->len == 0u) {
        puts("Error. Invalid length!");
        return;
    }

    for (size_t idx = snek->len; idx > 0; idx--) {
        /* Update body position starting from the tail. This also
         * updates the next node to the tail, to make sure, that when
         * the snek gets drawn, the new node (now the tail) is in the
         * correct position. */
        snek->body[idx] = snek->body[idx - 1u];
    }

    /* Update head */
    snek->body[0u].x += snek_move_map[snek->move_dir].x;
    snek->body[0u].y += snek_move_map[snek->move_dir].y;
}

void snek_eat(struct snek *snek)
{
    snek->len = SNEK_MIN(snek->len + 1, SNEK_BODY_MAX_SIZE);
}

size_t snek_get_len(const struct snek *snek)
{
    return snek->len;
}

struct snek_point snek_get_pos(const struct snek *snek, size_t idx)
{
    struct snek_point pos = {.x = -1, .y = -1};
    if (idx < snek->len) {
        pos = snek->body[idx];
    }
    return pos;
}
