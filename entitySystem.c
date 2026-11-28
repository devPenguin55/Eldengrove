#include "entitySystem.h"
#include "render.h"
#include "input.h"
#include "chunks.h"
#include "chunkLoaderManager.h"
#include <stdio.h>
#include <stdlib.h>

#define ENTITY_MAX_SEARCH_DIST 30.0f

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
    worldEntities.entities[worldEntities.amtEntities].pillarTimer = 0.0f;
    worldEntities.entities[worldEntities.amtEntities].isInViewToRender = 1;
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
            return y+1;
    }

    return -1.0f;
}

float getWalkableY(Entity *entity, int x, float currentY, int z)
{
    Entity testEntity = *entity;

    float groundY = getGroundY(x, z);

    if (groundY < 0) {
        return -1.0f;
    }
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
        // printf("Target is not walkable\n");
        free(entity->path);
        entity->path = NULL;
        entity->pathLength = 0;
        entity->pathIndex = 0;
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
            if (neighborY < 0 || neighborY > ChunkHeightY-1) { continue; }
            
            int dy = neighborY - current.position.y;
            if (blockAtPosition(neighborX, (int)round(neighborY)-1, neighborZ)->blockType == BLOCK_TYPE_WATER) {
                continue; 
            }
            float dxFromStart = neighborX - startPosition.x;
            float dyFromStart = neighborY - startPosition.y;
            float dzFromStart = neighborZ - startPosition.z;

            if (dxFromStart * dxFromStart +
                dyFromStart * dyFromStart +
                dzFromStart * dzFromStart > ENTITY_MAX_SEARCH_DIST * ENTITY_MAX_SEARCH_DIST) {
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
    // printf(
    //     "closed: %d | collision: %d | distance: %d | closedSize: %d\n",
    //     closedCount,
    //     collisionCount,
    //     distanceCount,
    //     closedSet.size
    // );

    free(openSet.nodes);
    free(closedSet.nodes);

    free(entity->path);
    entity->path = NULL;
    entity->pathLength = 0;
    entity->pathIndex = 0;
}



static int solidCell(int x, int y, int z) {
    if (y < 0) return 1;
    if (y >= ChunkHeightY) return 0;
    Block *b = blockAtPosition(x, y, z);
    return b == NULL || !b->isAir;
}

static void rebuildChunkAt(int chunkIdxX, int chunkIdxZ) {
    BucketEntry *r = getHashmapEntry(packChunkKey(chunkIdxX, chunkIdxZ));
    if (r != NULL) triggerRenderChunkRebuild(r->chunkEntry);
}

static void placeBlockAndRebuild(int x, int y, int z, int type) {
    if (y < 0 || y >= ChunkHeightY) return;
    Block *b = blockAtPosition(x, y, z);
    if (b == NULL || !b->isAir) return;

    b->isAir = 0;
    b->isSlope = 0;
    b->blockType = type;

    Chunk *c = chunkAtPosition(x, y, z);
    if (c == NULL) return;
    triggerRenderChunkRebuild(c);

    int chunkXUnit = ChunkWidthX * BlockWidthX;
    int chunkZUnit = ChunkLengthZ * BlockLengthZ;
    int ci = (int)(c->chunkStartX / chunkXUnit);
    int cj = (int)(c->chunkStartZ / chunkZUnit);
    int lx = x - c->chunkStartX;
    int lz = z - c->chunkStartZ;

    if (lx == 0)                    rebuildChunkAt(ci - 1, cj);
    else if (lx == ChunkWidthX - 1) rebuildChunkAt(ci + 1, cj);
    if (lz == 0)                    rebuildChunkAt(ci, cj - 1);
    else if (lz == ChunkLengthZ-1)  rebuildChunkAt(ci, cj + 1);
}

static int collidesAt(Entity *e, float x, float y, float z) {
    Entity t = *e;
    t.position.x = x; t.position.y = y; t.position.z = z;
    return entityCollides(&t);
}

static void fallbackSteer(Entity *e) {
    Vec3 p = e->position;
    float dx = player.position.x - p.x;
    float dz = player.position.z - p.z;
    float len = sqrtf(dx*dx + dz*dz);
    float ux = (len > 0.001f) ? dx / len : 0.0f;
    float uz = (len > 0.001f) ? dz / len : 0.0f;

    int grounded   = collidesAt(e, p.x, p.y - 0.1f, p.z);
    int blocked    = collidesAt(e, p.x + ux*0.6f, p.y,        p.z + uz*0.6f);
    int clearAbove = !collidesAt(e, p.x + ux*0.6f, p.y + 1.0f, p.z + uz*0.6f);
    int headroom   = !collidesAt(e, p.x, p.y + 1.0f, p.z);
    float heightGap = player.position.y - p.y;

    // want to go up if the player is higher and we're either under them
    // or the obstacle ahead is too tall to hop
    int needUp = heightGap > 1.0f && (len < 3.0f || (blocked && !clearAbove));

    float speed = 3.0f;
    e->velocity.x = 0.0f;
    e->velocity.z = 0.0f;

    if (needUp) {
        // stay in the column while building
        if (grounded && headroom) {
            e->velocity.y = 7.5f * 2;
            e->pillarTimer = 0.5f; // jump in progress
        } else if (!grounded && e->pillarTimer > 0.0f && e->velocity.y <= 0.0f) {
            // block under feet
            int bx = (int)floorf(p.x);
            int bz = (int)floorf(p.z);
            int by = (int)floorf(p.y + 0.01f) - 1;
            if (!solidCell(bx, by, bz)) {
                placeBlockAndRebuild(bx, by, bz, BLOCK_TYPE_ENTITY_PLACED);
            }
            e->pillarTimer = 0.0f; // only one block for each jump
        }
        if (e->pillarTimer > 0.0f) e->pillarTimer -= DELTA_TIME;
        return;
    }

    e->pillarTimer = 0.0f;

    // towards player
    if (len > 1.0f) {
        e->velocity.x = ux * speed;
        e->velocity.z = uz * speed;
    }
    // jump 1 blocks 
    if (grounded && blocked && clearAbove && headroom) {
        e->velocity.y = 7.5f * 2;
    }
}

void worldEntityUpdate() {
    for (int i = 0; i < worldEntities.amtEntities; i++) {
        Entity *curEntity = &worldEntities.entities[i];

        if (
            ((curEntity->position.x - player.position.x) * (curEntity->position.x - player.position.x) +
            (curEntity->position.y - player.position.y) * (curEntity->position.y - player.position.y) +
            (curEntity->position.z - player.position.z) * (curEntity->position.z - player.position.z)) > (ENTITY_MAX_SEARCH_DIST * ENTITY_MAX_SEARCH_DIST)
        ) {
            curEntity->isInViewToRender = 0;
            AnimatedModel *model = &modelManager.animatedModels[curEntity->animatedModelIndex];
            updateAnimatedModelInstanceTransformOnly(
                model,
                curEntity->animatedModelInstanceIndex,
                &curEntity->position,
                &curEntity->rotation,
                0.0f
            );
            continue;
        } else {
            curEntity->isInViewToRender = 1;
        }

        curEntity->isInWater = heightInWater(&curEntity->position);

        aStarPathfinding(curEntity, &player.position);

        curEntity->pathIndex = 1;

        if (curEntity->pathLength > 1 && curEntity->path) {
            // real A* path: follow it (no block placing here)
            Vec3 t = curEntity->path[1];
            float dx = t.x - curEntity->position.x;
            float dy = t.y - curEntity->position.y;
            float dz = t.z - curEntity->position.z;
            curEntity->velocity.x = dx*2;
            curEntity->velocity.y = dy*2;
            curEntity->velocity.z = dz*2;
            if (dy > 0) curEntity->velocity.y = 7.5f*2;
            curEntity->pillarTimer = 0.0f;
        } else if (curEntity->pathLength == 1) {
            curEntity->velocity.x = 0.0f;
            curEntity->velocity.z = 0.0f;
        } else {
            fallbackSteer(curEntity);
        }

        if (curEntity->pillarTimer) {
            int bx = (int)floorf(curEntity->position.x);
            int bz = (int)floorf(curEntity->position.z);
            int by = (int)floorf(curEntity->position.y + 0.01f) - 1;
            if (!solidCell(bx, by, bz)) {
                placeBlockAndRebuild(bx, by, bz, BLOCK_TYPE_ENTITY_PLACED);
            }
        }

        updateEntityPhysics(curEntity);
        
        AnimatedModel *model = &modelManager.animatedModels[curEntity->animatedModelIndex];
        updateAnimatedModelInstanceTransformOnly(
            model,
            curEntity->animatedModelInstanceIndex,
            &curEntity->position,
            &curEntity->rotation,
            curEntity->size
        );
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


