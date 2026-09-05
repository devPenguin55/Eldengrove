#include "entitySystem.h"
#include "render.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>

WorldEntities worldEntities;


void initEntitySystem() {
    worldEntities.amtEntities = 0;
    worldEntities.capacity = 16;
    worldEntities.entities = malloc(sizeof(Entity) * worldEntities.capacity);
}

void createEntity(int animatedModelIndex, int curAnimationIndex, Vec3 *position, Vec3 *velocity, Vec3 *rotation, float size) {
    if (worldEntities.amtEntities >= worldEntities.capacity) {
        worldEntities.capacity *= 2;
        worldEntities.entities = realloc(worldEntities.entities, sizeof(Entity) * worldEntities.capacity);
    }

    worldEntities.entities[worldEntities.amtEntities].position.x = position->x;
    worldEntities.entities[worldEntities.amtEntities].position.y = position->y;
    worldEntities.entities[worldEntities.amtEntities].position.z = position->z;
    worldEntities.entities[worldEntities.amtEntities].velocity.x = velocity->x;
    worldEntities.entities[worldEntities.amtEntities].velocity.y = velocity->y;
    worldEntities.entities[worldEntities.amtEntities].velocity.z = velocity->z;
    worldEntities.entities[worldEntities.amtEntities].rotation.x = rotation->x;
    worldEntities.entities[worldEntities.amtEntities].rotation.y = rotation->y;
    worldEntities.entities[worldEntities.amtEntities].rotation.z = rotation->z;
    worldEntities.entities[worldEntities.amtEntities].animatedModelIndex = animatedModelIndex;
    worldEntities.entities[worldEntities.amtEntities].curAnimationIndex = curAnimationIndex;
    worldEntities.entities[worldEntities.amtEntities].size = size;
    
    
    
    worldEntities.entities[worldEntities.amtEntities].animatedModelInstanceIndex = createAnimatedModelInstance(&modelManager.animatedModels[animatedModelIndex], position, rotation, size, curAnimationIndex);
    worldEntities.amtEntities++;
    
    
}

void worldEntityUpdate() {
    for (int i = 0; i < worldEntities.amtEntities; i++) {
        Entity *curEntity = &worldEntities.entities[i];
        
        curEntity->position = vec3Add(curEntity->position, vec3Scale(curEntity->velocity, DELTA_TIME));

        if ((curEntity->velocity.x + curEntity->velocity.y + curEntity->velocity.z) > 0.0f) {
            AnimatedModel *model = &modelManager.animatedModels[curEntity->animatedModelIndex];
            updateAnimatedModelInstanceTransformOnly(
                model,
                curEntity->animatedModelInstanceIndex,
                &curEntity->position,
                &curEntity->rotation,
                curEntity->size
            );
        }
    }

    for (int animatedModelIndex = 0; animatedModelIndex < modelManager.amtAnimatedModels; animatedModelIndex++) {
        AnimatedModel *model = &modelManager.animatedModels[animatedModelIndex];
        for (unsigned int i = 0; i < model->instanceCount; i++) {
            updateAnimatedModelInstanceAnimationOnly(
                model,
                i,
                DELTA_TIME
            );
            
        }

        uploadBoneMatrices(model);
        renderAnimatedModel(model);
    }
}


