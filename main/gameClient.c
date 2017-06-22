#include "../lib/client.h"
#include "common.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void readMap(Map *map);
void drawMap(Map *map, WINDOW *window);
void drawEntity(WINDOW *window, Entity entity);
void redrawMapSpot(Map *map, WINDOW *window, int y, int x);

int main() {
	WINDOW *game_window;
	WINDOW *debug_window;
	Map *game_map = (Map *) malloc(sizeof(Map));

	Entity *entityData = (Entity *) malloc(MAX_ENTITIES * sizeof(Entity));
	Entity *entityDataOld = (Entity *) malloc(MAX_ENTITIES * sizeof(Entity));

	// initialize char
	Entity *player = (Entity *) malloc(sizeof(Entity));

	// allocate map memory
	game_map->screen = (char **) malloc(MAP_Y * sizeof(char *));
	game_map->color = (char **) malloc(MAP_Y * sizeof(char *));
	for (int i = 0; i < MAP_Y; i++) {
		game_map->screen[i] = (char *) malloc(MAP_X * sizeof(char));
		game_map->color[i] = (char *) malloc(MAP_X * sizeof(char));
	}

	int playerId;
	char serverIP[30] = "127.0.0.1";
	char name[13] = "Testificate";

	/*
	printf("Please enter the server IP: ");
	scanf("%s", serverIP);

	printf("Please enter your name: ");
	scanf("%s", name);
	*/

	connectToServer(serverIP);
	sendMsgToServer(name, strlen(name) + 1);
	recvMsgFromServer(player, WAIT_FOR_IT);

	playerId = player->id;
	
	// start curses
	initscr();
	// disable line buffering
	cbreak();
	// don't echo keys
	noecho();
	// hide cursor
	curs_set(0);

	// create a new window for the game screen
	game_window = newwin(MAP_Y, MAP_X - 1, 0, 0);
	debug_window = newwin(5, MAP_X - 1, MAP_Y, 0);
	box(debug_window, 0, 0);

	// enable special keys
	keypad(game_window, true);
	// wait for key press
	wtimeout(game_window, 0);
	wtimeout(debug_window, 0);
	timeout(0);

	// TODO: put this on map_color.rtxt file
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);
	init_pair(6, COLOR_BLACK, COLOR_RED);
	init_pair(7, COLOR_BLUE, COLOR_BLACK);


	refresh();

	// read the map file and save it to a map struct
	readMap(game_map);

	// draw the map
	drawMap(game_map, game_window);
	drawEntity(game_window, *player);
	wrefresh(game_window);

	int k;

	// DEBUG
	bool quit = false;

	drawMap(game_map, game_window);

	while (true) {
		k = wgetch(game_window);

		switch(k) {
			case KEY_UP:
				moveEntity(player, UP);
				sendMsgToServer(player, sizeof(Entity));
				break;
			case KEY_DOWN:
				moveEntity(player, DOWN);
				sendMsgToServer(player, sizeof(Entity));
				break;
			case KEY_LEFT:
				moveEntity(player, LEFT);
				sendMsgToServer(player, sizeof(Entity));
				break;
			case KEY_RIGHT:
				moveEntity(player, RIGHT);
				sendMsgToServer(player, sizeof(Entity));
				break;
			case 'a':
				stageAttack(player, UP, 0);
				sendMsgToServer(player, sizeof(Entity));
				break;
			case KEY_BACKSPACE:
				quit = true;
				break;
		}
		
		// if backspace was pressed
		if (quit)
			break;

		if (recvMsgFromServer(entityData, DONT_WAIT) != NO_MESSAGE) {
			// update local player entity
			*player = entityData[player->id];

			// debug
			mvwprintw(debug_window, 1, 1, "color: %d", player->color);

			// erase entities at previous frame
			for (int i = 0; i < MAX_ENTITIES; i++) {
				redrawMapSpot(game_map, game_window, entityDataOld[i].pos[POS_Y], entityDataOld[i].pos[POS_X]);
			}

			// redraw entities at new positions
			for (int i = 0; i < MAX_ENTITIES; i++) {
				entityDataOld[i] = entityData[i];
				if (entityData[i].isAlive)
					drawEntity(game_window, entityData[i]);
			}
			
			// refresh windows
			wrefresh(game_window);
			wrefresh(debug_window);
		}
	}

	endwin();

	return 0;
}

/*
	Reads map from resource files and saves it on a Map struct
	Only needs to be used once per initialization
*/
void readMap(Map *map) {
	FILE *map_screen = fopen("res/map_screen.rtxt", "r");
	FILE *map_color = fopen("res/map_color.rtxt", "r");

	for (int i = 0; i < MAP_Y; i++) {
		fgets(map->screen[i], MAP_X, map_screen);
		fgets(map->color[i], MAP_X, map_color);
		strtok(map->screen[i], "\n");
		strtok(map->color[i], "\n");
	}

	fclose(map_screen);
	fclose(map_color);
}

/*
	Takes a Map struct and prints it on the screen.
	Should be used after casting readMap();
*/
void drawMap(Map *map, WINDOW *window) {
	for (int i = 0; i < MAP_Y - 2; i++) {
		for (int j = 0; j < MAP_X - 3; j++) {
			mvwaddch(window, i + 1, j + 1, map->screen[i][j] | COLOR_PAIR( (int) (map->color[i][j] - '0')) );
		}
	}
	// surround game window with a box
	wattron(window, COLOR_PAIR(2));
	box(window, 0 , 0);
	wattroff(window, COLOR_PAIR(2));
}

/*
	Takes a coordinate and redraws the character
	present on the map at this coordinate, so we don't have
	to redraw the entire map every frame.
*/
void redrawMapSpot(Map *map, WINDOW *window, int y, int x) {
	mvwaddch(window, y + 1, x + 1, map->screen[y][x] | COLOR_PAIR( (int) (map->color[y][x] - '0')) );
}

/*
	Draws an entity with the data given by Entity struct
*/
void drawEntity(WINDOW *window, Entity entity) {
	mvwaddch(window, entity.pos[0] + 1, entity.pos[1] + 1, entity.icon | COLOR_PAIR(entity.color));
}

/* 
	Draws a clientside hitmark
*/
void drawHitmark(WINDOW *window, int y, int x) {
	mvwaddch(window, y, x, 'X' | COLOR_PAIR(6));
}

/* 
	Draws a clientside melee hitmark from entity position
*/
void drawMeleeHitmark(WINDOW *window, Entity entity, int direction) {
	//drawHitmark();
}