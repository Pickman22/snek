#ifndef SNEK_H
#define SNEK_H

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>

#define SNEK_BODY_MAX_SIZE (128u)

enum snek_move
{
    SNEK_MOVE_NONE,
    SNEK_MOVE_DOWN,
    SNEK_MOVE_UP,
    SNEK_MOVE_LEFT,
    SNEK_MOVE_RIGHT,
    SNEK_MOVE_NUM,
};

struct snek_point
{
    int x;
    int y;
};

struct snek;

#define snek_alloca() (struct snek *)alloca(snek_get_size())

#define snek_alloc() (struct snek *)malloc(snek_get_size())

#define snek_dealloc(x) free((x))

void snek_init(struct snek *snek, struct snek_point init_pos);

void snek_set_move_dir(struct snek *snek, enum snek_move move);

void snek_move(struct snek *snek);

void snek_eat(struct snek *snek);

size_t snek_get_size(void);

size_t snek_get_len(const struct snek *snek);

struct snek_point snek_get_pos(const struct snek *snek, size_t idx);

#endif
