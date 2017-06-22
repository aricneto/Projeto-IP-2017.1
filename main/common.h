#ifndef HEAD_H
#define HEAD_H

#include <ncurses.h>
#include <stdbool.h>

// max number of entities allowed to exist
#define MAX_ENTITIES 50

// delay between each packet sent in microseconds
#define PACKET_WAIT 33333

// entity AI delay in microseconds
#define AI_DELAY 500000

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
	RIGHT,
	UP_LEFT,
	UP_RIGHT,
	DOWN_RIGHT,
	DOWN_LEFT
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

// entity constants
// position
#define POS_Y 0
#define POS_X 1
// attack
#define ATK_DIR 0
#define ATK_TYP 1
#define NO_ATK -1

/*
 * represents an entity
 */
typedef struct entity {
	unsigned char id;	    // id used by players
	unsigned char pos[2];   // entity position
	bool isAlive;			// if entity is alive
	unsigned char type; 	// entity type (WARRIOR, RUNNER, etc)
	int icon;				/* entity icon displayed in map
							   it's an int so we can add attributes
							   to it using ncurses */
	unsigned char color;	// entity icon color
	unsigned char hp;		// entity health points
	char attack[2];		    /* entity attack information 
							   first arg (direction): 
							   		-1 if entity isn't attacking.
							   		direction enum if it is
							   second arg (type)
							   		attack type.
							   depends on the class.
							   eg. Warrior only has 1 attack (melee) (maybe add AoE?)
							   Archer has melee and ranged */
} Entity;


/*
 * creates a new entity
 */
Entity newEntity(unsigned char type, unsigned char initPosY, unsigned char initPosX, 
					char icon, unsigned char color, 
					unsigned char initHp) {
	Entity entity;
	entity.isAlive = true;
	entity.pos[POS_Y] = initPosY;
	entity.pos[POS_X] = initPosX;
	entity.icon = icon;
	entity.color = color;
	entity.hp = initHp;
	entity.attack[ATK_DIR] = NO_ATK;
	return entity;
}

/*
 * creates an entity with predefined aspects
 */
Entity newMonster(unsigned char type, 
					unsigned char initPosY, unsigned char initPosX) {
	switch (type) {
		case RUNNER:
			return newEntity(RUNNER, initPosY, initPosX, 'r', 0, 5);
		case CASTER:
			return newEntity(CASTER, initPosY, initPosX, 'c', 7, 5);
		case BERSERK:
			return newEntity(BERSERK, initPosY, initPosX, 'B', 2, 20);
	}
}

/*
 * creates a new player entity
 */
Entity newPlayer(unsigned char type, unsigned char id, 
					unsigned char initPosY, unsigned char initPosX) {
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
 * stages an attack and saves it on the entity
 */
void stageAttack(Entity *entity, int direction, int type) {
	entity->attack[ATK_DIR] = direction;
	entity->attack[ATK_TYP] = type;
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