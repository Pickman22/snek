# Snek

Classical simple game implemented in C using Raylib as frontend.
This simple project was an experiment to test and understand a couple
of programming concepts.

Features:
- Simple 8-bit style music.
- Sound effect when snek eats & grows.
- Game over window with its own sound effect.
- Score counter.

Implementation:
- snek.c: implements the movement and growing logic for the snake.
- game.c: implements game rules and animations. This file should
	actually be split into different modules.
- build.sh: simple shell script to compile and run the game.

Supported platforms:
This game has only been tested on a Macbook Air with M2 processor.
Although thanks to Raylib it should be pretty simple to port to any
other platform.

## Snek Gameplay:

![Snek game gameplay](https://github.com/Pickman22/snek/blob/master/snek_game.gif)

