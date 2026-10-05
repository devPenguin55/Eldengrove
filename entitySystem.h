#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

#include "vectors.h"

typedef struct AStarNode {
    Vec3 position;
    Vec3 parent;
    float g;
    float h;
    float f;
} AStarNode;

typedef struct PriorityQueue {
    AStarNode *nodes;
    int size;
    int capacity;
} PriorityQueue;

typedef struct ClosedSetNode {
    int x;
    int y;
    int z;
    int parentX;
    int parentY;
    int parentZ;
} ClosedSetNode;

typedef struct ClosedSet {
    int size;
    int capacity;
    ClosedSetNode *nodes;
} ClosedSet;

typedef struct Entity {
    Vec3 position;
    Vec3 rotation;
    float    size;

    int animatedModelIndex;
    int curAnimationIndex;
    int animatedModelInstanceIndex;

    Vec3 velocity;
    int isOnGround;
    int isInWater;

    Vec3 *path;
    int pathLength;
    int pathIndex;
} Entity;

typedef struct WorldEntities {
    Entity *entities;
    int amtEntities;
    int capacity;
} WorldEntities;

#define ANIMATED_MODEL_INDEX_CLERIC 0

void initEntitySystem();
void createEntity(int animatedModelIndex, int curAnimationIndex, Vec3 *position, Vec3 *velocity, Vec3 *rotation, float size);
void worldEntityUpdate();

extern WorldEntities worldEntities;

#endif