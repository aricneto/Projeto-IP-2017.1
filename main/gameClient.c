#include <ncurses.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// map size
// should be screen_size + 2 to accomodate borders
#define MAP_Y 33
#define MAP_X 92

typedef struct entity {
	unsigned char id;
	unsigned char pos[2];
	unsigned char hp;
	unsigned char icon;
	unsigned char color;
} Entity;

typedef struct map_s {
	char **screen;
	char **color;
} Map;

void readMap(Map *map);
void drawMap(Map *map, WINDOW *window);
void drawEntity(WINDOW *window, Entity entity);

int main() {
	WINDOW *game_window;
	Map *game_map = (Map *) malloc(sizeof(Map));

	// initialize char
	Entity player;
	player.pos[0] = 25;
	player.pos[1] = 50;
	player.icon = '@';
	player.color = 5;

	// allocate map memory
	game_map->screen = (char **) malloc(MAP_Y * sizeof(char *));
	game_map->color = (char **) malloc(MAP_Y * sizeof(char *));
	for (int i = 0; i < MAP_Y; i++) {
		game_map->screen[i] = (char *) malloc(MAP_X * sizeof(char));
		game_map->color[i] = (char *) malloc(MAP_X * sizeof(char));
	}

	// start curses
	initscr();
	// disable line buffering
	cbreak();
	// don't echo keys
	noecho();
	// hide cursor
	curs_set(0);

	/* create a new window for the game screen
	   why the -1? i don't know either. 
	   if it's not there there's a space at the
	   end of the map.
	*/
	game_window = newwin(MAP_Y, MAP_X - 1, 0, 0);

	// enable special keys
	keypad(game_window, true);
	// don't wait for new key
	wtimeout(game_window, 3000);

	// TODO: put this on map_color.rtxt file
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);
	init_pair(5, COLOR_CYAN, COLOR_BLACK);

	refresh();

	// read the map file and save it to a map struct
	readMap(game_map);

	// draw the map
	drawMap(game_map, game_window);
	drawEntity(game_window, player);
	wrefresh(game_window);

	int k;

	// DEBUG
	bool quit = false;

	while (true) {
		k = wgetch(game_window);

		drawMap(game_map, game_window);

		switch(k) {
			case KEY_UP:
				player.pos[0]--;
				break;
			case KEY_DOWN:
				player.pos[0]++;
				break;
			case KEY_LEFT:
				player.pos[1]--;
				break;
			case KEY_RIGHT:
				player.pos[1]++;
				break;
			case KEY_BACKSPACE:
				quit = true;
				break;
		}
		
		if (quit)
			break;

		drawEntity(game_window, player);
		wrefresh(game_window);
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
void redrawMapSpot(Map *map, WINDOW *window, int x, int y) {
	mvwaddch(window, x + 1, y + 1, map->screen[x][y] | COLOR_PAIR( (int) (map->color[x][y] - '0')) );
}

/*
	Draws an entity with the data given by Entity struct
*/
void drawEntity(WINDOW *window, Entity entity) {
	mvwaddch(window, entity.pos[0] + 1, entity.pos[1] + 1, entity.icon | COLOR_PAIR(entity.color));
}