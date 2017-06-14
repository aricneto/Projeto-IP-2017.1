#ifndef HEAD_H
#define HEAD_H

#include <stdbool.h>

#define MAX_ENTITIES 50
#define PACKET_WAIT 30

enum {
	POS_X,
	POS_Y
};

typedef struct entity {
	bool isAlive;
	unsigned char id;
	short int pos[2];
	unsigned char hp;
	unsigned char icon;
	int color;
} Entity;

#endif