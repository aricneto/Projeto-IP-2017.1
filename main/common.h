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

enum {
	UP,
	DOWN,
	LEFT,
	RIGHT
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
	unsigned char id;
	short int pos[2];
	unsigned char icon;
	unsigned char color;
	bool isAlive;
	unsigned char hp;
} Entity;


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