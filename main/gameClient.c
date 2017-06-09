#include "../lib/client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAP_X 50
#define MAP_Y 15

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

enum {
    Y,
    X
};

typedef struct charac {
    int position[2];
} Character;

void drawMap(char **map, Character player) {
    system("clear");
    for (int i = 0; i < MAP_Y; i++) {
        for (int j = 0; j < MAP_X; j++) {
            if (player.position[Y] == i && player.position[X] == j)
                map[i][j] = '@';
            else
                map[i][j] = ',';
        }
        printf(RED "%s\n" RESET, map[i]);
    }
}

int main() {

    Character player;
    player.position[X] = 0;
    player.position[Y] = 0;

    char **map = (char **) malloc(MAP_Y * sizeof(char *));
    for (int i = 0; i < MAP_Y; i++) {
        map[i] = (char *) malloc(MAP_X * sizeof(char));
    }

    while (true) {
        char input = getch();
        switch (input) {
            case 'w':
            player.position[Y] -= 1;
            break;
            case 's':
            player.position[Y] += 1;
            break;
            case 'd':
            player.position[X] += 1;
            break;
            case 'a':
            player.position[X] -= 1;
            break;
        }
        drawMap(map, player);
    }
    

}