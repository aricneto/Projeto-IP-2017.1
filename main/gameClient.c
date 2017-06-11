#include <ncurses.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SCREEN_SIZE_Y 33
#define MAX_SCREEN_SIZE_X 93

int main() {
	WINDOW *game_window;
	FILE *map_screen;
	FILE *map_color;

	// TODO: should this be dynamic?
	char map_buff[MAX_SCREEN_SIZE_X + 2];
	char map_color_buff[MAX_SCREEN_SIZE_X + 2];

	map_screen = fopen("res/map_screen.rtxt", "r");
	map_color = fopen("res/map_color.rtxt", "r");

	// start curses
	initscr();
	// disable line buffering
	raw();
	// don't echo keys
	noecho();

	// TODO: put this on map_color.rtxt file
	start_color();
	init_pair(1, COLOR_WHITE, COLOR_BLACK);
	init_pair(2, COLOR_RED, COLOR_BLACK);
	init_pair(3, COLOR_GREEN, COLOR_BLACK);
	init_pair(4, COLOR_YELLOW, COLOR_BLACK);

	// create a new window for the game screen
	game_window = newwin(MAX_SCREEN_SIZE_Y, MAX_SCREEN_SIZE_X, 0, 0);

	for (int i = 0; i < MAX_SCREEN_SIZE_Y; i++) {
		fgets(map_buff, MAX_SCREEN_SIZE_X, map_screen);
		fgets(map_color_buff, MAX_SCREEN_SIZE_X, map_color);
		strtok(map_color_buff, "\n");
		for (int j = 0; j < MAX_SCREEN_SIZE_X - 4; j++) {
			mvwaddch(game_window, i + 1, j + 2, map_buff[j] | COLOR_PAIR( (int) (map_color_buff[j] - 48)) );
		}
	}

	fclose(map_screen);
	fclose(map_color);

	// surround game window with a box
	box(game_window, 0 , 0);

	refresh();
	wrefresh(game_window);

    getch();

	endwin();

	return 0;
}