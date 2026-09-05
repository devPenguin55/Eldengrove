#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

#include "vectors.h"

typedef struct Entity {
    Vec3 position;
    Vec3 rotation;
    float    size;

    int animatedModelIndex;
    int curAnimationIndex;
    int animatedModelInstanceIndex;

    Vec3 velocity;
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

#endif