rm -rf build 2> /dev/null
mkdir build 2> /dev/null
gcc -std=c2x -Wall -Wextra -Wno-gnu -pedantic -g0 -ggdb snek.c -c
gcc -std=c2x -Wall -Wextra -Wno-gnu -pedantic -g0 -ggdb -I/opt/homebrew/Cellar/raylib/5.0/include game.c -c
gcc -std=c2x -Wall -Wextra -Wno-gnu -pedantic -g0 -ggdb -I/opt/homebrew/Cellar/raylib/5.0/include main.c -c
mv *.o build
gcc -std=c2x -Wall -Wextra -pedantic -g0 -ggdb ./build/snek.o ./build/game.o ./build/main.o -L/opt/homebrew/Cellar/raylib/5.0/lib -lraylib -o snek
./snek
