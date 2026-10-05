#include "entitySystem.h"
#include "render.h"
#include "input.h"
#include "chunks.h"
#include "chunkLoaderManager.h"
#include <stdio.h>
#include <stdlib.h>

WorldEntities worldEntities;


void initEntitySystem() {
    worldEntities.amtEntities = 0;
    worldEntities.capacity = 16;
    worldEntities.entities = malloc(sizeof(Entity) * worldEntities.capacity);
}

int heightInWater(Vec3 *position) {
    int voxelX = (int)round(position->x / BlockWidthX);
    int voxelY = (int)round(position->y / BlockHeightY);
    int voxelZ = (int)round(position->z / BlockLengthZ);

    int chunkX = (voxelX >= 0) ? voxelX / ChunkWidthX : (voxelX - (ChunkWidthX - 1)) / ChunkWidthX;
    int chunkZ = (voxelZ >= 0) ? voxelZ / ChunkLengthZ : (voxelZ - (ChunkLengthZ - 1)) / ChunkLengthZ;

    voxelX = voxelX - chunkX * ChunkWidthX;
    voxelY = voxelY;
    voxelZ = voxelZ - chunkZ * ChunkLengthZ;

    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    BucketEntry *result = getHashmapEntry(chunkKey);
    if (result == NULL)
    { 
        return 0;
    };

    Chunk *curChunk = result->chunkEntry;
    int index = voxelX + ChunkWidthX * voxelZ + (ChunkWidthX * ChunkLengthZ) * voxelY;

    if (index > ChunkWidthX * ChunkLengthZ * ChunkHeightY || index < 0) { return 0; }
    Block *block = &curChunk->blocks[index];
    return (block != NULL && block->blockType == BLOCK_TYPE_WATER);
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
    worldEntities.entities[worldEntities.amtEntities].isOnGround = 0;
    worldEntities.entities[worldEntities.amtEntities].isInWater = heightInWater(position);
    worldEntities.entities[worldEntities.amtEntities].animatedModelInstanceIndex = createAnimatedModelInstance(&modelManager.animatedModels[animatedModelIndex], position, rotation, size, curAnimationIndex);
    worldEntities.entities[worldEntities.amtEntities].pathIndex = -1;
    worldEntities.entities[worldEntities.amtEntities].pathLength = -1;
    worldEntities.entities[worldEntities.amtEntities].path = NULL;
    worldEntities.amtEntities++;
}
float h(Vec3 pos1, Vec3 pos2)
{
    float x = pos1.x - pos2.x;
    float y = pos1.y - pos2.y;
    float z = pos1.z - pos2.z;

    return sqrtf(x * x + y * y + z * z);
}

float movementCost(int dx, int dy, int dz)
{
    return sqrtf(dx * dx + dy * dy + dz * dz);
}

int pqFindNode(PriorityQueue *queue, Vec3 position)
{
    for (int i = 0; i < queue->size; i++)
    {
        if (queue->nodes[i].position.x == position.x &&
            queue->nodes[i].position.y == position.y &&
            queue->nodes[i].position.z == position.z)
            return i;
    }

    return -1;
}

void pqPush(PriorityQueue *queue, AStarNode node)
{
    if (queue->size >= queue->capacity)
    {
        queue->capacity *= 2;
        queue->nodes = realloc(
            queue->nodes,
            sizeof(AStarNode) * queue->capacity
        );
    }

    int i = queue->size++;
    queue->nodes[i] = node;

    while (i > 0)
    {
        int parent = (i - 1) / 2;

        if (queue->nodes[parent].f <= queue->nodes[i].f)
            break;

        AStarNode temp = queue->nodes[parent];
        queue->nodes[parent] = queue->nodes[i];
        queue->nodes[i] = temp;

        i = parent;
    }
}

AStarNode pqPop(PriorityQueue *queue)
{
    AStarNode result = queue->nodes[0];

    queue->size--;

    if (queue->size == 0)
        return result;

    queue->nodes[0] = queue->nodes[queue->size];

    int i = 0;

    while (1)
    {
        int left = i * 2 + 1;
        int right = i * 2 + 2;
        int smallest = i;

        if (left < queue->size &&
            queue->nodes[left].f < queue->nodes[smallest].f)
            smallest = left;

        if (right < queue->size &&
            queue->nodes[right].f < queue->nodes[smallest].f)
            smallest = right;

        if (smallest == i)
            break;

        AStarNode temp = queue->nodes[i];
        queue->nodes[i] = queue->nodes[smallest];
        queue->nodes[smallest] = temp;

        i = smallest;
    }

    return result;
}

void pqUpdate(PriorityQueue *queue, int index)
{
    while (index > 0)
    {
        int parent = (index - 1) / 2;

        if (queue->nodes[parent].f <= queue->nodes[index].f)
            break;

        AStarNode temp = queue->nodes[parent];
        queue->nodes[parent] = queue->nodes[index];
        queue->nodes[index] = temp;

        index = parent;
    }
}

int csNodeInSet(ClosedSet *closedSet, int x, int y, int z)
{
    for (int i = 0; i < closedSet->size; i++)
    {
        if (closedSet->nodes[i].x == x &&
            closedSet->nodes[i].y == y &&
            closedSet->nodes[i].z == z)
            return 1;
    }

    return 0;
}

void csPush(ClosedSet *closedSet,int x,int y,int z,int parentX,int parentY,int parentZ)
{
    if (csNodeInSet(closedSet,x,y,z))
        return;

    if (closedSet->size >= closedSet->capacity)
    {
        closedSet->capacity *= 2;
        closedSet->nodes = realloc(
            closedSet->nodes,
            sizeof(ClosedSetNode) * closedSet->capacity
        );
    }

    closedSet->nodes[closedSet->size++] = (ClosedSetNode){
        x,y,z,parentX,parentY,parentZ
    };
}

int csFindNode(ClosedSet *closedSet,int x,int y,int z)
{
    for (int i = 0; i < closedSet->size; i++)
    {
        if (closedSet->nodes[i].x == x &&
            closedSet->nodes[i].y == y &&
            closedSet->nodes[i].z == z)
            return i;
    }

    return -1;
}

void reconstructPath(Entity *entity,AStarNode goalNode,ClosedSet *closedSet,Vec3 startPosition)
{
    // printf("got to the reconstruct path step!\n");
    Vec3 reversePath[1024];
    int pathLength = 0;

    Vec3 current = goalNode.position;
    Vec3 parent = goalNode.parent;

    while (1)
    {
        if (pathLength >= 1024)
        {
            printf("A* ERROR: path exceeded 1024 nodes\n");
            break;
        }
        
        reversePath[pathLength++] = current;
        
        if (current.x == startPosition.x &&
            current.y == startPosition.y &&
            current.z == startPosition.z)
            break;

            current = parent;
            
            if (current.x == startPosition.x &&
                current.y == startPosition.y &&
                current.z == startPosition.z)
                {
                    reversePath[pathLength++] = current;
                    break;
                }
                
        int index = csFindNode(
            closedSet,
            current.x,
            current.y,
            current.z
        );

        if (index == -1)
        {
            printf("A* ERROR: could not find parent node\n");
            break;
        }

        parent.x = closedSet->nodes[index].parentX;
        parent.y = closedSet->nodes[index].parentY;
        parent.z = closedSet->nodes[index].parentZ;
    }

    free(entity->path);

    entity->path = malloc(sizeof(Vec3) * pathLength);
    entity->pathLength = pathLength;
    entity->pathIndex = 0;

    for (int i = 0; i < pathLength; i++) {
        entity->path[i] = reversePath[pathLength - 1 - i];
        entity->path[i].y -= 0.5;
    }
}

int hasGroundBelow(float x, float y, float z) {
    Block *block = blockAtPosition((int)x, (int)y-1, (int)z);

    if (block != NULL && block->isAir != 1) {
        return 1;
    }

    return 0;
}
float getGroundY(int x, int z)
{
    for (int y = 127; y >= 0; y--)
    {
        Block *block = blockAtPosition(x, y, z);

        if (block != NULL && block->isAir != 1)
            return y + 1;
    }

    return -1.0f;
}

float getWalkableY(Entity *entity, int x, float currentY, int z)
{
    Entity testEntity = *entity;

    float groundY = getGroundY(x, z);

    if (groundY < 0)
        return -1.0f;

    testEntity.position.x = x;
    testEntity.position.y = groundY;
    testEntity.position.z = z;

    if (!entityCollides(&testEntity) && groundY <= currentY+1) {
        return groundY;
    }

    return -1.0f;
}

void aStarPathfinding(Entity *entity, Vec3 *targetPosition)
{
    Vec3 startPosition = {
        roundf(entity->position.x),
        roundf(entity->position.y),
        roundf(entity->position.z)
    };

    int goalX = roundf(targetPosition->x);
    int goalZ = roundf(targetPosition->z);

    float goalY = getWalkableY(
        entity,
        goalX,
        roundf(targetPosition->y),
        goalZ
    );

    if (goalY < 0)
    {
        printf("Target is not walkable\n");
        return;
    }

    Vec3 goalPosition = {
        goalX,
        goalY,
        goalZ
    };

    PriorityQueue openSet;
    openSet.capacity = 64;
    openSet.size = 0;
    openSet.nodes = malloc(sizeof(AStarNode) * openSet.capacity);

    ClosedSet closedSet;
    closedSet.capacity = 64;
    closedSet.size = 0;
    closedSet.nodes = malloc(
        sizeof(ClosedSetNode) * closedSet.capacity
    );

    AStarNode start = {
        .position = startPosition,
        .parent = startPosition,
        .g = 0.0f,
        .h = h(startPosition, goalPosition),
        .f = 0.0f
    };

    start.f = start.g + start.h;

    pqPush(&openSet, start);
     
    float maxDist = 15.0f;
    int collisionCount = 0;
    int closedCount = 0;
    int distanceCount = 0;

    int directions[6][3] = {
        { 1, 0, 0},
        {-1, 0, 0},
        { 0, 0, 1},
        { 0, 0,-1},
        { 0, 1, 0},
        { 0,-1, 0}
    };

    while (openSet.size > 0)
    {
        
        AStarNode current = pqPop(&openSet);

        if (current.position.x == goalPosition.x &&
            current.position.y == goalPosition.y &&
            current.position.z == goalPosition.z)
        {
            reconstructPath(
                entity,
                current,
                &closedSet,
                startPosition
            );

            free(openSet.nodes);
            free(closedSet.nodes);

            return;
        }

        csPush(
            &closedSet,
            current.position.x,
            current.position.y,
            current.position.z,
            current.parent.x,
            current.parent.y,
            current.parent.z
        );

        for (int dirIdx = 0; dirIdx < 4; dirIdx++)
        {   
            int dx = directions[dirIdx][0];
            int dz = directions[dirIdx][2];
            
            int neighborX = current.position.x + dx;
            int neighborZ = current.position.z + dz;
            float neighborY = getWalkableY(entity, neighborX, current.position.y, neighborZ);
            if (neighborY < 0) { continue; }
            
            int dy = neighborY - current.position.y;
            float dxFromStart = neighborX - startPosition.x;
            float dyFromStart = neighborY - startPosition.y;
            float dzFromStart = neighborZ - startPosition.z;

            if (dxFromStart * dxFromStart +
                dyFromStart * dyFromStart +
                dzFromStart * dzFromStart > maxDist * maxDist) {
                    distanceCount++;
                    continue;
                }

            if (csNodeInSet(
                &closedSet,
                neighborX,
                neighborY,
                neighborZ)) {
                    closedCount++;
                    continue;
                }

            Vec3 originalPosition = entity->position;

            entity->position.x = neighborX;
            entity->position.y = neighborY;
            entity->position.z = neighborZ;

            int collision = entityCollides(entity);

            entity->position = originalPosition;

            if (collision) {
                collisionCount++;
                continue;
            }

            Vec3 neighborPosition = {
                neighborX,
                neighborY,
                neighborZ
            };

            float tentativeGScore =
                current.g +
                movementCost(dx, dy, dz);

            int existingIndex =
                pqFindNode(
                    &openSet,
                    neighborPosition
                );

            if (existingIndex == -1)
            {
                AStarNode neighbor = {
                    .position = neighborPosition,
                    .parent = current.position,
                    .g = tentativeGScore,
                    .h = h(
                        neighborPosition,
                        goalPosition
                    ),
                    .f = 0.0f
                };

                neighbor.f =
                    neighbor.g +
                    neighbor.h;

                pqPush(&openSet, neighbor);
            }
            else if (
                tentativeGScore <
                openSet.nodes[existingIndex].g
            )
            {
                openSet.nodes[existingIndex].g =
                    tentativeGScore;

                openSet.nodes[existingIndex].f =
                    tentativeGScore +
                    openSet.nodes[existingIndex].h;

                openSet.nodes[existingIndex].parent =
                    current.position;

                pqUpdate(
                    &openSet,
                    existingIndex
                );
            }
        }
        // printf("%d\n",openSet.size);
    }
    printf(
        "closed: %d | collision: %d | distance: %d | closedSize: %d\n",
        closedCount,
        collisionCount,
        distanceCount,
        closedSet.size
    );

    free(openSet.nodes);
    free(closedSet.nodes);

    free(entity->path);
    entity->path = NULL;
    entity->pathLength = 0;
    entity->pathIndex = 0;
}

float totEntityTime = 0.0f;
void worldEntityUpdate() {
    totEntityTime += DELTA_TIME;
    for (int i = 0; i < worldEntities.amtEntities; i++) {
        Entity *curEntity = &worldEntities.entities[i];
        curEntity->isInWater = heightInWater(&curEntity->position);

        // apply movement code in terms of the velocity before the physics apply
        // * Note that if the entity stops moving after a collision, that's because it needs velocity inputs before physics calculations
        // if (curEntity->isOnGround && curEntity->path == NULL) {
        //     aStarPathfinding(curEntity, &player.position);
        //     // printf("completed the astar pathfinding with path length %d", curEntity->pathLength);
        //     curEntity->pathIndex = 0;
        // }

        // if (curEntity->path && curEntity->pathIndex < curEntity->pathLength) {
        //     Vec3 newPos = curEntity->path[curEntity->pathIndex++];
        //     curEntity->velocity.x = newPos.x - curEntity->position.x;
        //     curEntity->velocity.y = newPos.y - curEntity->position.y;
        //     curEntity->velocity.z = newPos.z - curEntity->position.z;
        // } else if (curEntity->path && curEntity->pathIndex == curEntity->pathLength) {
        //     curEntity->velocity.x = 0;
        //     curEntity->velocity.y = 0;
        //     curEntity->velocity.z = 0;
        //     aStarPathfinding(curEntity, &player.position);
        //     // printf("completed the astar pathfinding with path length %d", curEntity->pathLength);
        //     curEntity->pathIndex = 0;
        // }

        if (1)
        {
            if (curEntity->path == NULL || curEntity->pathIndex >= curEntity->pathLength || 1)
            {
                printf("started pathfinding %f\n", DELTA_TIME);
                aStarPathfinding(curEntity,&player.position);
                printf("   -> got length %d\n", curEntity->pathLength);
                
                curEntity->pathIndex = 1;
            }

            if (curEntity->path && curEntity->pathIndex < curEntity->pathLength && (int)totEntityTime % 1 == 0 && totEntityTime > 1.0f)
            {
                Vec3 target = curEntity->path[curEntity->pathIndex];

                // float dx = target.x - curEntity->position.x;
                // float dy = target.y - curEntity->position.y;
                // float dz = target.z - curEntity->position.z;
                curEntity->position = target;

                // float distance = sqrtf(dx * dx + dy * dy + dz * dz);

                // if (distance < 0.1f)
                // {
                //     curEntity->pathIndex++;

                //     if (curEntity->pathIndex >= curEntity->pathLength)
                //     {
                //         free(curEntity->path);
                //         curEntity->path = NULL;

                        curEntity->velocity.x = 0;
                        curEntity->velocity.y = 0;
                        curEntity->velocity.z = 0;
                //     }
                // }
                // else
                // {
                //     curEntity->velocity.x = dx;
                //     curEntity->velocity.y = dy;
                //     curEntity->velocity.z = dz;
                // }
                curEntity->pathIndex++;
            }
            // printf("ended pathfinding\n");
        }
        // printf("%f %f %f\n", curEntity->velocity.x, curEntity->velocity.y, curEntity->velocity.z);
        // updateEntityPhysics(curEntity);
        

        AnimatedModel *model = &modelManager.animatedModels[curEntity->animatedModelIndex];
        updateAnimatedModelInstanceTransformOnly(
            model,
            curEntity->animatedModelInstanceIndex,
            &curEntity->position,
            &curEntity->rotation,
            curEntity->size
        );
    }

    if ((int)totEntityTime % 1 == 0 && totEntityTime > 1.0f) {
        totEntityTime = 0.0;
    }
    // printf("tot entity time: %f", totEntityTime);

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


