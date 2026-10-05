#ifndef PLAYER_H
#define PLAYER_H
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdint.h>
#include "chunks.h"
#include "vectors.h"
#include "entitySystem.h"

typedef struct Player {
    Vec3 position;
    Vec3 velocity;
    int isOnGround;
    int isInWater;
    float width;
    float height;
} Player;

Chunk *chunkAtPosition(int voxelX, int voxelY, int voxelZ);
Block *blockAtPosition(int voxelX, int voxelY, int voxelZ);
int isSolidVoxel(int voxelX, int voxelY, int voxelZ);
float playerHalfWidth(Player* player);
float entityHalfWidthX(Entity* entity);
float entityHalfWidthZ(Entity* entity);
float entityMaxY(Entity *entity);
float entityHeight(Entity *entity);
int playerCollides(Player* player);
int entityCollides(Entity *entity);
float radians(float degrees);
void updatePlayerPhysics(Player *player);
void updateEntityPhysics(Entity *entity);

#endif