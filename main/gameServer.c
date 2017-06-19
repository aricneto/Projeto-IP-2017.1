#include "../lib/server.h"
#include "common.h"
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

// TODO: implement time spent to beat game?

#define MAX_LOGIN_SIZE 13
#define MAX_GAME_CLIENTS 4

void readMapHitbox();
bool hitbox(int y, int x);
void refreshEntityHitbox(Entity *entity_data);

void *mobLogicThread(void *entity_data);

int findClosestPlayer(Entity *entity_data, Entity *entity);
void pathfind(Entity *entity, Entity *target, int distance);

/*
typedef struct {
	Entity *entity_data[MAX_ENTITIES];
	bool **entityHitbox;
} ThreadInfo;*/


// TODO: tirar variaveis globais
bool **mapHitbox;
bool **entityHitbox;


int main(){
	char client_names[MAX_GAME_CLIENTS][MAX_LOGIN_SIZE];

	Entity *entity_data = (Entity *) calloc(MAX_ENTITIES, sizeof(Entity));
	Entity *entityBuffer = (Entity *) malloc(sizeof(Entity));

	for (int i = 0; i < MAX_ENTITIES; i++)
		entity_data->isAlive = false;

	mapHitbox = (bool **) malloc(MAP_Y * sizeof(bool *));
	for (int i = 0; i < MAP_Y; i++)
		mapHitbox[i] = (bool *) malloc(MAP_X * sizeof(bool));

	entityHitbox = (bool **) malloc(MAP_Y * sizeof(bool *));
	for (int i = 0; i < MAP_Y; i++)
		entityHitbox[i] = (bool *) malloc(MAP_X * sizeof(bool));

	//memset(entityHitbox, 0, sizeof(bool) * MAP_Y * MAP_X);

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
	
	timeout(0);

	pthread_t mob_thread;
	pthread_create(&mob_thread, NULL, &mobLogicThread, (void *) entity_data);

	printw("Server Running!\n");

	//box(0, 0);

	while(true){
		// sleeps for PACKET_WAIT microseconds every cycle
		usleep(PACKET_WAIT);

		int id = acceptConnection();

		if(id != NO_CONNECTION){
			recvMsgFromClient(client_names[id], id, WAIT_FOR_IT);
			
			Entity player = newPlayer(WARRIOR, id, 25, 40 + (id * 3));

			// entity_data[0] to entity_data[MAX_CLIENTS] are reserved for players
			entity_data[id] = player;

			// send player entity to client
			sendMsgToClient(&player, sizeof(Entity), id);

			printw("%s connected id = %d\n", client_names[id], id);
		}

		refreshEntityHitbox(entity_data);

		// receive a message from client
		struct msg_ret_t msg_ret = recvMsg(entityBuffer);
		if(msg_ret.status == MESSAGE_OK){
			// if player doesn't walk into a wall, update entity_data array with new
			// entityBuffer.
			// this part should be redone as to update each part of an entity separately
			if (!hitbox(entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X])) {
				entity_data[msg_ret.client_id] = *entityBuffer;

				// print message on server console
				mvprintw(entityBuffer->id + 5, 0,"%d pos: %.2d %.2d", entityBuffer->id, entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X]);
			}
		}
		else if(msg_ret.status == DISCONNECT_MSG){
			printw("%s disconnected, id = %d is free\n", client_names[msg_ret.client_id], msg_ret.client_id);
		}


		// stop server on backspace
		if (getch() == KEY_BACKSPACE){
			printw("Server closed!");
			break;
		}
		
		refresh();

		// send entity data to clients
		broadcast(entity_data, MAX_ENTITIES * sizeof(Entity));
	}

	endwin();

	return 0;
}

/*
 * separate thread that calculates monster ai and spawning every second
 */
void *mobLogicThread(void *entityData) {
	// TODO: while program is running!
	// TODO: monsters should spawn in waves
	srand(time(NULL));

	Entity *entity_data = (Entity *) entityData;

	int loops = 0;
    while(true) {
        usleep(AI_DELAY);
		// how many entities have been spawned in this cycle
		bool entitiesSpawned = 0;
		int y, x;
		//memset(entityHitbox, 0, sizeof(bool) * MAP_Y * MAP_X);
		// 0 to MAX_CLIENTS entity ids are reserved for players
        for (int i = MAX_CLIENTS; i < MAX_ENTITIES; i++) {
			// refresh entity hitbox information
			if (entity_data[i].isAlive) {
				//y = (int) entity_data[i].pos[POS_Y];
				//x = (int) entity_data[i].pos[POS_X];
				// if entity is alive, use pathfind() function to move it towards the closest player
				pathfind(&entity_data[i], &entity_data[findClosestPlayer(entity_data, &entity_data[i])], 3);
				//entityHitbox[y][x] = 1;
			} else if ( !entity_data[i].isAlive && (rand() % 200) <= 3 && entitiesSpawned < 5) {
				entity_data[i] = newMonster(BERSERK, rand() % (MAP_Y - 2), rand() % (MAP_X - 2));
				entitiesSpawned++;
			}
			
		}
    }
    return 0;
}

/*
 * populates entityHitbox with current entity position data
 */
void refreshEntityHitbox(Entity *entity_data) {
	// clear array TODO: CHANGE THIS
	for (int i = 0; i < MAP_Y; i++) {
		for (int j = 0; j < MAP_X; j++) {
			entityHitbox[i][j] = 0;
		}
	}
	// populate array with entity data
	int x, y;
	for (int i = 0; i < MAX_ENTITIES; i++) {
		if (entity_data[i].isAlive) {
			y = (int) entity_data[i].pos[POS_Y];
			x = (int) entity_data[i].pos[POS_X];
			mvprintw(i, 0, "%d %d\n", y, x);
			entityHitbox[y][x] = 1;
		}
	}
}


/*
 * uses pythagoras to get a hypotenuse
 */
double hypotenuse(int x, int y) {
	return sqrt((double) ((x * x) + (y * y)));
}

/*
 * finds closest player to an entity
 */
int findClosestPlayer(Entity *entity_data, Entity *entity) {
	int closestPlayer = 0;
	double closestHypotenuse = hypotenuse( entity_data[closestPlayer].pos[POS_X] - entity->pos[POS_X],
						  				   entity_data[closestPlayer].pos[POS_Y] - entity->pos[POS_Y]);
	for (int i = 1; i < MAX_CLIENTS; i++) {
		// hypothenuse of (player_x - entity_x), (player_y - entity_y)
		if (hypotenuse( entity_data[i].pos[POS_X] - entity->pos[POS_X], 
					    entity_data[i].pos[POS_Y] - entity->pos[POS_Y]) < closestHypotenuse) {
			closestPlayer = i;
			closestHypotenuse = hypotenuse( entity_data[closestPlayer].pos[POS_X] - entity->pos[POS_X],
						  				    entity_data[closestPlayer].pos[POS_Y] - entity->pos[POS_Y]);
		}
	}

	return closestPlayer;
}


/*
 * very primitive pathfinding that moves entity to target
 * distance is the closest distance the entity should be from target
 */
void pathfind(Entity *entity, Entity *target, int distance) {
	if ((int) hypotenuse(entity->pos[POS_X] - target->pos[POS_X], 
				   		 entity->pos[POS_Y] - target->pos[POS_Y]) > distance) {
		// move x towards target
		if (entity->pos[POS_X] + distance < target->pos[POS_X] && !hitbox(entity->pos[POS_Y], entity->pos[POS_X] + 1))
			entity->pos[POS_X]++;
		else if (entity->pos[POS_X] + distance > target->pos[POS_X] && !hitbox(entity->pos[POS_Y], entity->pos[POS_X] - 1))
			entity->pos[POS_X]--;
		
		// move y towards target
		if (entity->pos[POS_Y] + distance < target->pos[POS_Y] && !hitbox(entity->pos[POS_Y] + 1, entity->pos[POS_X]))
			entity->pos[POS_Y]++;
		else if (entity->pos[POS_Y] + distance > target->pos[POS_Y] && !hitbox(entity->pos[POS_Y] - 1, entity->pos[POS_X]))
			entity->pos[POS_Y]--;
	}
}

/*
 * returns true if entity hits a hitbox
 */
bool hitbox(int y, int x) {
	return (mapHitbox[y][x])
		|| (entityHitbox[y][x])
		|| (y > MAP_Y - 3)
		|| (y < 0)
		|| (x > MAP_X - 4)
		|| (x < 0);
}

/*
	Reads map hitbox from resource files and saves it on a bool array
	Only needs to be used once per initialization
	TODO: CONVERT MAP FROM CHAR TO BOOL ARRAY
*/
void readMapHitbox() {
	FILE *map_hitbox = fopen("res/map_hitbox.rtxt", "r");
	char tempMap[MAP_Y][MAP_X];

	for (int i = 0; i < MAP_Y; i++) {
		fgets(tempMap[i], MAP_X, map_hitbox);
		strtok(tempMap[i], "\n");
		for (int j = 0; j < MAP_X; j++) {
			// convert string to int
			mapHitbox[i][j] = tempMap[i][j] - '0'; 
		}
	}

	fclose(map_hitbox);
}
