#include "../lib/server.h"
#include "common.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LOGIN_SIZE 13
#define MAX_GAME_CLIENTS 4

void readMapHitbox(char **map);
bool hitbox(char **map, int y, int x);

int main(){
	char client_names[MAX_GAME_CLIENTS][MAX_LOGIN_SIZE];

	Entity *entityData = (Entity *) malloc(MAX_ENTITIES * sizeof(Entity));
	Entity *entityBuffer = (Entity *) malloc(sizeof(Entity));

	for (int i = 0; i < MAX_ENTITIES; i++)
		entityData->isAlive = false;

	char **mapHitbox = (char **) malloc(MAP_Y * sizeof(char *));
	for (int i = 0; i < MAP_Y; i++)
		mapHitbox[i] = (char *) malloc(MAP_X * sizeof(char));

	readMapHitbox(mapHitbox);

	serverInit(MAX_GAME_CLIENTS);

	// start curses
	initscr();
	// disable line buffering
	cbreak();
	// don't echo keys
	noecho();
	// hide cursor
	curs_set(0);
	// enable special keys
	keypad(stdscr, true);
	// waits for a key for PACKET_WAIT milliseconds.
	// that means it sends a packet every PACKET_WAIT
	timeout(PACKET_WAIT);

	printw("Server Running!\n");

	//box(0, 0);

	while(true){
		int id = acceptConnection();

		if(id != NO_CONNECTION){
			recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);

			// initialize char
			Entity *player = (Entity *) malloc(sizeof(Entity));
			player->id = id;
			player->isAlive = true;
			player->pos[POS_Y] = 25;
			player->pos[POS_X] = 50;
			player->icon = '@';
			player->color = 5;
			
			entityData[id] = *player;

			// send id to client
			sendMsgToClient(player, sizeof(Entity), id);


			//broadcast(str_buffer, strlen(str_buffer) + 1);
			
			printw("%s connected id = %d\n", client_names[id], id);
		}

		struct msg_ret_t msg_ret = recvMsg(entityBuffer);
		if(msg_ret.status == MESSAGE_OK){

			if (!hitbox(mapHitbox, entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X])) {
				entityData[msg_ret.client_id] = *entityBuffer;
				mvprintw(entityBuffer->id + 5, 0,"%d pos: %.2d %.2d", entityBuffer->id, entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X]);
			}
			//broadcast(str_buffer, strlen(str_buffer) + 1);
		}
		else if(msg_ret.status == DISCONNECT_MSG){
			printw("%s disconnected, id = %d is free\n", client_names[msg_ret.client_id], msg_ret.client_id);
			//broadcast(str_buffer, strlen(str_buffer) + 1);
		}


		// stop server on backspace
		if (getch() == KEY_BACKSPACE){
			printw("Server closed!");
			break;
		}
		
		refresh();
		broadcast(entityData, MAX_ENTITIES * sizeof(Entity));
	}

	endwin();

	return 0;
}

bool hitbox(char **map, int y, int x) {
	return (map[y][x] - '0' == 1)
		|| (y > MAP_Y - 3)
		|| (y < 1)
		|| (x > MAP_X - 4)
		|| (x < 0);
}

/*
	Reads map hitbox from resource files and saves it on a bool array
	Only needs to be used once per initialization
	TODO: CONVERT MAP FROM CHAR TO BOOL ARRAY
*/
void readMapHitbox(char **map) {
	FILE *map_hitbox = fopen("res/map_hitbox.rtxt", "r");

	for (int i = 0; i < MAP_Y; i++) {
		fgets(map[i], MAP_X, map_hitbox);
		strtok(map[i], "\n");
	}

	fclose(map_hitbox);
}
