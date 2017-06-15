#ifndef HEAD_H
#define HEAD_H

#include <stdbool.h>

// max number of entities allowed to exist
#define MAX_ENTITIES 50

// delay between each packet sent and received in milliseconds
#define PACKET_WAIT 0

// map size
// should be screen_size + 2 to accomodate borders
#define MAP_Y 33
#define MAP_X 92

typedef struct map_s {
	char **screen;
	char **color;
} Map;

// direction constants
enum {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

// entity type constants
enum {
	// player types
	WARRIOR = 1, // melee
	ARCHER,      // arrows
	CLERIC,		 // heals in AoE
	// monster types
	RUNNER, 	 // runs after the player
	CASTER, 	 // stays at least 6 tiles from the player and attacks from a distance
	BERSERK  	 // high damage, slow
};

// constants for entity
enum {
	POS_Y,
	POS_X
};

/*
 * represents an entity
 */
typedef struct entity {
	short int id;			// id used by players
	short int pos[2];		// entity position
	bool isAlive;			// if entity is alive
	unsigned char type; 	// entity type (WARRIOR, RUNNER, etc)
	char icon;				// entity icon displayed in map
	unsigned char color;	// entity icon color
	unsigned char hp;		// entity health points
} Entity;


/*
 * creates a new entity
 */
Entity newEntity(unsigned char type, short int initPosY, short int initPosX, 
					char icon, unsigned char color, 
					unsigned char initHp) {
	Entity entity;
	entity.isAlive = true;
	entity.pos[POS_Y] = initPosY;
	entity.pos[POS_X] = initPosX;
	entity.icon = icon;
	entity.color = color;
	entity.hp = initHp;
	return entity;
}

/*
 * creates an entity with predefined aspects
 */
Entity newMonster(unsigned char type, 
					short int initPosY, short int initPosX) {
	switch (type) {
		case RUNNER:
			return newEntity(RUNNER, initPosY, initPosX, 'r', 0, 5);
		case CASTER:
			return newEntity(CASTER, initPosY, initPosX, 'c', 7, 5);
		case BERSERK:
			return newEntity(BERSERK, initPosY, initPosX, 'B', 6, 20);
	}
}

/*
 * creates a new player entity
 */
Entity newPlayer(unsigned char type, short int id, 
					short int initPosY, short int initPosX) {
	Entity player;
	switch (type) {
		case WARRIOR:
			player = newEntity(WARRIOR, initPosY, initPosX, 'W', 5, 30);
			break;
		case ARCHER:
			player = newEntity(ARCHER, initPosY, initPosX, 'A', 4, 20);		
			break;
		case CLERIC:
			player = newEntity(CLERIC, initPosY, initPosX, 'C', 6, 20);		
			break;
	}
	player.id = id;
	return player;
}

/*
 * updates an entity's position respecting the constraints of the map
 * hitbox is also checked server-side
 */
void moveEntity(Entity *entity, int direction) {
	switch(direction) {
		case UP:
			if (entity->pos[POS_Y] - 1 >= 0)
				entity->pos[POS_Y]--;
			break;
		case DOWN:
			if (entity->pos[POS_Y] + 1 < MAP_Y)
				entity->pos[POS_Y]++;
			break;
		case LEFT:
			if (entity->pos[POS_X] - 1 >= 0)
				entity->pos[POS_X]--;
			break;
		case RIGHT:
			if (entity->pos[POS_X] + 1 < MAP_X)
				entity->pos[POS_X]++;
			break;
	}
}

#endif