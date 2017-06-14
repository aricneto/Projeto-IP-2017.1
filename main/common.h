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

// constants for entity
enum {
	POS_Y,
	POS_X
};

/*
 * Represents an entity
 */
typedef struct entity {
	unsigned char id;
	short int pos[2];
	unsigned char icon;
	unsigned char color;
	bool isAlive;
	unsigned char hp;
} Entity;

#endif