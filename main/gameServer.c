#include "../lib/server.h"
#include "common.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define MAX_LOGIN_SIZE 13
#define MAX_GAME_CLIENTS 4

int main(){
	char client_names[MAX_GAME_CLIENTS][MAX_LOGIN_SIZE];


	Entity *entityData = (Entity *) malloc(MAX_ENTITIES * sizeof(Entity));
	Entity *entityBuffer = (Entity *) malloc(sizeof(Entity));

	for (int i = 0; i < MAX_ENTITIES; i++) {
		entityData->isAlive = false;
	}

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
			player->pos[0] = 25;
			player->pos[1] = 50;
			player->icon = 'X';
			player->color = 5;
			
			entityData[id] = *player;

			// send id to client
			sendMsgToClient(player, sizeof(Entity), id);


			//broadcast(str_buffer, strlen(str_buffer) + 1);
			
			printw("%s connected id = %d\n", client_names[id], id);
		}

		struct msg_ret_t msg_ret = recvMsg(entityBuffer);
		if(msg_ret.status == MESSAGE_OK){
			mvprintw(entityBuffer->id + 4, 0,"%d pos: %d %d\n", entityBuffer->id, entityBuffer->pos[0], entityBuffer->pos[1]);

			entityData[msg_ret.client_id] = *entityBuffer;

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
		broadcast(entityData, MAX_ENTITIES * sizeof(Entity));
	}

	endwin();

	return 0;
}
