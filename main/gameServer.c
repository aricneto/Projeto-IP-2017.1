#include "../lib/server.h"
#include "common.h"
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
void *mobLogicThread(void *entity_data);

typedef struct closest_player {
	int id;
	int distance;
} ClosestPlayer;

int getEntityIdFromReference(Entity *entity_data, int direction, int refId);
int getClosestPlayerDirectionFromReference(Entity *entity_data, int refId);
ClosestPlayer findClosestPlayer(Entity *entity_data, Entity *entity);
bool pathfind(Entity *entity, Entity *target, int distance);

// attack functions
void attack(Entity *entity_data, int direction, int idAttacker, int reach, int damage);
bool dealDamage(Entity *entity, int damage);

/*
typedef struct {
	Entity *entity_data[MAX_ENTITIES];
	bool **entityHitbox;
} ThreadInfo;*/


// TODO: tirar variaveis globais
bool **mapHitbox;
int **entityHitbox;


int main(){
	char client_names[MAX_GAME_CLIENTS][MAX_LOGIN_SIZE];

	Entity *entity_data = (Entity *) calloc(MAX_ENTITIES, sizeof(Entity));
	Entity *entityBuffer = (Entity *) malloc(sizeof(Entity));

	for (int i = 0; i < MAX_ENTITIES; i++) {
		entity_data[i].isAlive = 0;
		//entity_data[i].id = i;
	}

	mapHitbox = (bool **) malloc(MAP_Y * sizeof(bool *));
	entityHitbox = (int **) malloc(MAP_Y * sizeof(int *));
	for (int i = 0; i < MAP_Y; i++) {
		mapHitbox[i] = (bool *) malloc(MAP_X * sizeof(bool));
		entityHitbox[i] = (int *) malloc(MAP_X * sizeof(int));
	}

	// initialize entity hitbox
	for (int i = 0; i < MAP_Y; i++) {
		for (int j = 0; j < MAP_X; j++) {
			entityHitbox[i][j] = NO_HITBOX;
		}
	}

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

		// receive a message from client
		struct msg_ret_t client_data = recvMsg(entityBuffer);
		if(client_data.status == MESSAGE_OK){
			// if there's a hitbox in new position, update player with previous position data
			if (hitbox(entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X])) {
				entityBuffer->pos[POS_Y] = entity_data[client_data.client_id].pos[POS_Y];
				entityBuffer->pos[POS_X] = entity_data[client_data.client_id].pos[POS_X];
			} else {
				entityHitbox[entity_data[client_data.client_id].pos[POS_Y]][entity_data[client_data.client_id].pos[POS_X]] = NO_HITBOX;
				entityHitbox[entityBuffer->pos[POS_Y]][entityBuffer->pos[POS_X]] = client_data.client_id;
			}

			if (entityBuffer->attack[ATK_DIR] >= 0) {
				// getDamageFromAttackType() 
				attack(entity_data, entityBuffer->attack[ATK_DIR], client_data.client_id, 1, 5);
				entityBuffer->attack[ATK_DIR] = NO_ATK;
				mvprintw(1, 0, "hit!");
			}

			// save buffer on server data
			entity_data[client_data.client_id] = *entityBuffer;
			// print message on server console
			mvprintw(entityBuffer->id + 5, 0,"%d pos: %.2d %.2d", entityBuffer->id, entityBuffer->pos[POS_Y], entityBuffer->pos[POS_X]);
		}
		else if(client_data.status == DISCONNECT_MSG){
			printw("%s disconnected, id = %d is free\n", client_names[client_data.client_id], client_data.client_id);
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
		int y, x, j = 0;
		//memset(entityHitbox, 0, sizeof(bool) * MAP_Y * MAP_X);
		// 0 to MAX_CLIENTS entity ids are reserved for players
        for (int i = 0; i < MAX_ENTITIES; i++) {
			if (entity_data[i].isAlive) {
				// refresh hitmark
				if (entity_data[i].state == STATE_HIT) {
					entity_data[i].state ^= STATE_HIT;
					entity_data[i].icon ^= A_REVERSE;
				}
				if (entity_data[i].attack[ATK_CLD] > 0) {
					entity_data[i].attack[ATK_CLD] -= 40;
				}
				if (i > MAX_CLIENTS) {
					// if entity is alive and not a player, use pathfind() 
					// function to move it towards the closest player
					ClosestPlayer closest = findClosestPlayer(entity_data, &entity_data[i]);
					y = (int) entity_data[i].pos[POS_Y];
					x = (int) entity_data[i].pos[POS_X];
					if (closest.distance <= 1) {
						int closestDir = getClosestPlayerDirectionFromReference(entity_data, i);
						if (closestDir != -1) {
							attack(entity_data, closestDir, i, 1, 2);
						}
					}
					// if entity has moved with pathfind()
					if (pathfind(&entity_data[i], 
							&entity_data[closest.id], 1)) {
						// clear hitbox at old position
						entityHitbox[y][x] = NO_HITBOX;
						// save new position
						y = (int) entity_data[i].pos[POS_Y];
						x = (int) entity_data[i].pos[POS_X];
						entityHitbox[y][x] = i;
					}
				}
				//mvprintw(i + 2, 0, "%.2d %.2d", y, x);
				//mvprintw(i + 2, 8, "id: %d", findClosestPlayer(entity_data, &entity_data[i]));
				//mvprintw(i + 2, 14, "alive: %d", entity_data[findClosestPlayer(entity_data, &entity_data[i])].isAlive);
			} else if ( i > MAX_CLIENTS && !entity_data[i].isAlive && (rand() % 200) <= 200 && entitiesSpawned < 5) {
				entity_data[i] = newMonster(BERSERK, (rand() % (MAP_Y - 4)) + 2, (rand() % (MAP_X - 5)) + 2);
				entitiesSpawned++;
			}
			
		}
    }
    return 0;
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
ClosestPlayer findClosestPlayer(Entity *entity_data, Entity *entity) {
	ClosestPlayer closestPlayer;
	closestPlayer.id = 0;
	closestPlayer.distance = hypotenuse( entity_data[closestPlayer.id].pos[POS_X] - entity->pos[POS_X],
						  				   entity_data[closestPlayer.id].pos[POS_Y] - entity->pos[POS_Y]);
	for (int i = 0; i < MAX_CLIENTS; i++) {
		if (entity_data[i].isAlive) {
			// hypothenuse of (player_x - entity_x), (player_y - entity_y)
			if (hypotenuse( entity_data[i].pos[POS_X] - entity->pos[POS_X], 
							entity_data[i].pos[POS_Y] - entity->pos[POS_Y]) < closestPlayer.distance) {
				closestPlayer.id = i;
				closestPlayer.distance = hypotenuse( entity_data[closestPlayer.id].pos[POS_X] - entity->pos[POS_X],
												entity_data[closestPlayer.id].pos[POS_Y] - entity->pos[POS_Y]);
			}
		}
	}

	return closestPlayer;
}


/*
 * very primitive pathfinding that moves entity to target
 * distance is the closest distance the entity should be from target
 * returns true if entity moves
 */
bool pathfind(Entity *entity, Entity *target, int distance) {
	bool hasMoved = false;
	if ((int) hypotenuse(entity->pos[POS_X] - target->pos[POS_X], 
				   		 entity->pos[POS_Y] - target->pos[POS_Y]) > distance) {
		// move x towards target
		if (entity->pos[POS_X] + distance < target->pos[POS_X] 
			&& !hitbox(entity->pos[POS_Y], entity->pos[POS_X] + 1)) {
			entity->pos[POS_X]++;
			hasMoved = true;
		} else if (entity->pos[POS_X] + distance > target->pos[POS_X] 
			&& !hitbox(entity->pos[POS_Y], entity->pos[POS_X] - 1)) {
			entity->pos[POS_X]--;
			hasMoved = true;
		}
		
		// move y towards target
		if (entity->pos[POS_Y] + distance < target->pos[POS_Y] 
			&& !hitbox(entity->pos[POS_Y] + 1, entity->pos[POS_X])) {
			entity->pos[POS_Y]++;
			hasMoved = true;
		} else if (entity->pos[POS_Y] + distance > target->pos[POS_Y] 
			&& !hitbox(entity->pos[POS_Y] - 1, entity->pos[POS_X])) {
			entity->pos[POS_Y]--;
			hasMoved = true;
		}
	}
	return hasMoved;
}

/*
 * returns true if entity hits a hitbox
 */
bool hitbox(int y, int x) {
	return (y > MAP_Y - 3)
		|| (y < 0)
		|| (x > MAP_X - 4)
		|| (x < 0)
		|| (mapHitbox[y][x])
		|| (entityHitbox[y][x] != NO_HITBOX);
}

/*
	Reads map hitbox from resource files and saves it on a bool array
	Only needs to be used once per initialization
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

/*
 * gets the direction of the closest player from a reference point
 */
int getClosestPlayerDirectionFromReference(Entity *entity_data, int refId) {
	int id;
	// go through all directions (up, down, left, right)
	for (int dir = 0; dir < 4; dir++) {
		id = getEntityIdFromReference(entity_data, dir, refId);
		if (id >= 0 && id < MAX_CLIENTS) {
			return dir;
		}
	}
	return -1;
}

/*
 * gets an entity id from a reference point and direction
 */
int getEntityIdFromReference(Entity *entity_data, int direction, int refId) {
	int refY = entity_data[refId].pos[POS_Y];
    int refX = entity_data[refId].pos[POS_X];
	// TODO: prevent segfault in this
	switch(direction) {
		case UP:
			return entityHitbox[refY - 1][refX];
		case DOWN:
			return entityHitbox[refY + 1][refX];
		case LEFT:
			return entityHitbox[refY][refX - 1];
		case RIGHT:
			return entityHitbox[refY][refX + 1];
	}
}

/*
 * attack an entity
 * entity_data - current entity_data
 * direction - direction of the attack (direction enum on common.h)
 * idAttacker - id of the attacker
 * idTarget - id of the target
 * reach - how far the attack goes
 * damage - how much damage the attack does
 */
void attack(Entity *entity_data, int direction, int idAttacker, int reach, int damage){
	if (entity_data[idAttacker].attack[ATK_CLD] <= 0) {
		// armazena os valores das posicoes
		int posAtkY = entity_data[idAttacker].pos[POS_Y];
		int posAtkX = entity_data[idAttacker].pos[POS_X];

		int idTarget = getEntityIdFromReference(entity_data, direction, idAttacker);

		if (idTarget >= 0) {
			int posTgtY = entity_data[idTarget].pos[POS_Y];
			int posTgtX = entity_data[idTarget].pos[POS_X];

			bool isPossible = false;

			// verificar o ataque a partir da direção
			switch(direction) {
				// verifica se a coluna eh a mesma, se esta numa linha menor(ou igual?) e a diferenca esta dentro do alcance
				case UP:
					isPossible = (posAtkX == posTgtX && posAtkY >= posTgtY
							&& posAtkY - posTgtY <= reach);
					break;
				case DOWN: 
					isPossible = (posAtkX == posTgtX && posAtkY <= posTgtY 
							&& posTgtY - posAtkY <= reach);
					break;
				case LEFT: 
					isPossible = (posAtkY == posTgtY && posAtkX >= posTgtX
							&& posAtkX - posTgtX <= reach);
					break;		
				case RIGHT: 
					isPossible = (posAtkY == posTgtY && posAtkX <= posTgtX
							&& posTgtX - posAtkX <= reach);
					break;		
			}
			if (isPossible)
				dealDamage(&entity_data[idTarget], damage);
		}
		// reset cooldown
		entity_data[idAttacker].attack[ATK_CLD] = ATTACK_COOLDOWN;
	}
}

/*
 * deal damage
 * returns true if entity dies
 */
bool dealDamage(Entity *entity, int damage){
    entity->hp -= damage;
	if (entity->hp <= 0) {
		entity->isAlive = false;
		entityHitbox[entity->pos[POS_Y]][entity->pos[POS_X]] = NO_HITBOX;
		return true;
	} else {
		entity->icon |= A_REVERSE;
		entity->state |= STATE_HIT;
		return false;
	}
}