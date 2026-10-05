#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "chunks.h"
#include "chunkLoaderManager.h"
#include "input.h"
#include "entitySystem.h"
#include "render.h"

Chunk *chunkAtPosition(int voxelX, int voxelY, int voxelZ) {
    int playerChunkX = (int)floor(player.position.x / (ChunkWidthX * BlockWidthX));
    int playerChunkZ = (int)floor(player.position.z / (ChunkLengthZ * BlockLengthZ));

    int chunkX = (voxelX >= 0)
        ? voxelX / ChunkWidthX
        : (voxelX - (ChunkWidthX - 1)) / ChunkWidthX;

    int chunkZ = (voxelZ >= 0)
        ? voxelZ / ChunkLengthZ
        : (voxelZ - (ChunkLengthZ - 1)) / ChunkLengthZ;

    if (
        (chunkX > (playerChunkX + CHUNK_PRELOAD_RADIUS) || (chunkX < (playerChunkX - CHUNK_PRELOAD_RADIUS))) ||
        (chunkZ > (playerChunkZ + CHUNK_PRELOAD_RADIUS) || (chunkZ < (playerChunkZ - CHUNK_PRELOAD_RADIUS))))
    { 
        return NULL;
    }

    
    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    BucketEntry* result = getHashmapEntry(chunkKey);

    if (result == NULL) {
        return NULL;
    }

    Chunk* chunk = result->chunkEntry;
    return chunk;
}

Block *blockAtPosition(int voxelX, int voxelY, int voxelZ) {
    int chunkX = (voxelX >= 0)
        ? voxelX / ChunkWidthX
        : (voxelX - (ChunkWidthX - 1)) / ChunkWidthX;

    int chunkZ = (voxelZ >= 0)
        ? voxelZ / ChunkLengthZ
        : (voxelZ - (ChunkLengthZ - 1)) / ChunkLengthZ;

    int localX = voxelX - chunkX * ChunkWidthX;
    int localY = voxelY;
    int localZ = voxelZ - chunkZ * ChunkLengthZ;

    uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
    BucketEntry* result = getHashmapEntry(chunkKey);

    if (result == NULL) {
        return NULL;
    }

    Chunk* chunk = result->chunkEntry;

    int index =
    localX +
    ChunkWidthX * localZ +
    (ChunkWidthX * ChunkLengthZ) * localY;
    
    if (index < 0 ||
        index >= ChunkWidthX * ChunkLengthZ * ChunkHeightY)
        {
            return NULL;
        }
        
    return &chunk->blocks[index];
}

int isSolidVoxel(int voxelX, int voxelY, int voxelZ)
{
    Block* block = blockAtPosition(voxelX, voxelY, voxelZ);

    if (block == NULL) { return 0; }

    return (
        blockRegistry[block->blockType].isPhysicsSolid &&
        !block->isAir
    );
}

float playerHalfWidth(Player *player)
{
    // return 0;
    return player->width * 0.5f;
}

float entityHalfWidthX(Entity *entity)
{
    Vec3 dimensions = modelManager.animatedModels[entity->animatedModelIndex].dimensions;

    float halfX = dimensions.x * 0.5f;
    float halfZ = dimensions.z * 0.5f;
    float angle = radians(entity->rotation.y);

    return entity->size * (
        fabsf(cosf(angle)) * halfX +
        fabsf(sinf(angle)) * halfZ
    );
}

float entityHalfWidthZ(Entity *entity)
{
    Vec3 dimensions = modelManager.animatedModels[entity->animatedModelIndex].dimensions;

    float halfX = dimensions.x * 0.5f;
    float halfZ = dimensions.z * 0.5f;
    float angle = radians(entity->rotation.y);

    return entity->size * (
        fabsf(sinf(angle)) * halfX +
        fabsf(cosf(angle)) * halfZ
    );
}

float getSlopeHeight(Block* block, float x, float z)
{
    float localX = x - block->x;
    float localZ = z - block->z;

    switch (block->isSlope)
    {
        case 1:
            return block->y + (1.0f - localZ);

        case 2:
            return block->y + (1.0f - localX);

        case 3:
            return block->y + (1.0f - fabs(localZ));

        case 4:
            return block->y + (1.0f - fabs(localX));

        default:
            return block->y;
    }
}

int playerCollides(Player* player) { 
    // return 0;
    float minX = player->position.x - playerHalfWidth(player); 
    float maxX = player->position.x + playerHalfWidth(player); 
    float minY = player->position.y; 
    float maxY = player->position.y + player->height; 
    float minZ = player->position.z - playerHalfWidth(player); 
    float maxZ = player->position.z + playerHalfWidth(player); 

    int voxelMinX = (int)round(minX); 
    int voxelMaxX = (int)round(maxX); 
    int voxelMinY = (int)round(minY); 
    int voxelMaxY = (int)round(maxY); 
    int voxelMinZ = (int)round(minZ); 
    int voxelMaxZ = (int)round(maxZ); 

    for (int x = voxelMinX; x <= voxelMaxX; x++) { 
        for (int y = voxelMinY; y <= voxelMaxY; y++) { 
            for (int z = voxelMinZ; z <= voxelMaxZ; z++) { 
                Block *block = blockAtPosition(x,y,z); 

                if (block == NULL) { continue; } 

                if (blockRegistry[block->blockType].isPhysicsSolid && !block->isAir) { 
                    if (block->isSlope) { 
                        float localBlockX = player->position.x - block->x; 
                        float localBlockZ = player->position.z - block->z; 
                        
                        float rampHeightLocal; 
                        switch (block->isSlope) { 
                            case 1: rampHeightLocal = 1 - localBlockZ; break; 
                            case 2: rampHeightLocal = 1 - localBlockX; break; 
                            case 3: rampHeightLocal = 1 - fabsf(localBlockZ); break; 
                            case 4: rampHeightLocal = 1 - fabsf(localBlockX); break; 
                            default: rampHeightLocal = 0; break; 
                        } 

                        rampHeightLocal += block->y; 
                        if ((rampHeightLocal - minY) > 0.55f) { 
                            return 1; 
                        } else { 
                            continue; 
                        } 
                    } else { 
                        return 1; 
                    } 
                } 
            } 
        } 
    } 

    return 0; 
}

float radians(float degrees)
{
    return degrees * 3.1415926535 / 180.0f;
}

float entityMaxY(Entity *entity)
{
    return entity->position.y + entityHeight(entity);
}

float entityHeight(Entity *entity)
{
    Vec3 dimensions = modelManager.animatedModels[entity->animatedModelIndex].dimensions;

    float halfX = dimensions.x * 0.5f;
    float halfY = dimensions.y * 0.5f;
    float halfZ = dimensions.z * 0.5f;

    float x = radians(entity->rotation.x);
    float z = radians(entity->rotation.z);

    float halfHeight =
        fabsf(cosf(x) * cosf(z)) * halfY +
        fabsf(sinf(x) * cosf(z)) * halfZ +
        fabsf(sinf(z)) * halfX;

    return entity->size * halfHeight * 2.0f;
}
                        
int entityCollides(Entity *entity) { 
    float halfWidthX = entityHalfWidthX(entity);
    float halfWidthZ = entityHalfWidthZ(entity);

    float minX = entity->position.x - halfWidthX;
    float maxX = entity->position.x + halfWidthX;
    float minY = entity->position.y;
    float maxY = entity->position.y + entityHeight(entity);
    float minZ = entity->position.z - halfWidthZ;
    float maxZ = entity->position.z + halfWidthZ;
//     printf("dims: %f %f %f\n",
//     modelManager.animatedModels[entity->animatedModelIndex].dimensions.x,
//     modelManager.animatedModels[entity->animatedModelIndex].dimensions.y,
//     modelManager.animatedModels[entity->animatedModelIndex].dimensions.z
// );

// printf("half X: %f\n", entityHalfWidthX(entity));
// printf("half Z: %f\n", entityHalfWidthZ(entity));
    int voxelMinX = (int)round(minX); 
    int voxelMaxX = (int)round(maxX); 
    int voxelMinY = (int)round(minY); 
    int voxelMaxY = (int)round(maxY); 
    int voxelMinZ = (int)round(minZ); 
    int voxelMaxZ = (int)round(maxZ); 

    for (int x = voxelMinX; x <= voxelMaxX; x++) { 
        for (int y = voxelMinY; y <= voxelMaxY; y++) { 
            for (int z = voxelMinZ; z <= voxelMaxZ; z++) { 
                Block *block = blockAtPosition(x,y,z); 

                if (block == NULL) { continue; } 

                if (blockRegistry[block->blockType].isPhysicsSolid && !block->isAir) { 
                    if (block->isSlope) { 
                        float localBlockX = entity->position.x - block->x; 
                        float localBlockZ = entity->position.z - block->z; 
                        
                        float rampHeightLocal; 
                        switch (block->isSlope) { 
                            case 1: rampHeightLocal = 1 - localBlockZ; break; 
                            case 2: rampHeightLocal = 1 - localBlockX; break; 
                            case 3: rampHeightLocal = 1 - fabsf(localBlockZ); break; 
                            case 4: rampHeightLocal = 1 - fabsf(localBlockX); break; 
                            default: rampHeightLocal = 0; break; 
                        } 

                        rampHeightLocal += block->y; 
                        if ((rampHeightLocal - minY) > 0.55f) { 
                            return 1; 
                        } else { 
                            continue; 
                        } 
                    } else { 
                        return 1; 
                    } 
                } 
            } 
        } 
    } 

    return 0; 
}
float totTime = 0.0f;

void updatePlayerPhysics(Player* player)
{
    if (totTime < 5.0) { totTime += DELTA_TIME; return; }

    float gravity = (player->isInWater) ? (5.0f) : (20.0f);
    if (player->isOnGround == -1) {
        gravity = 0.0f;
    }
    // gravity = 0.0;
    int slopeDirCur = slopeDir(player);
    player->velocity.y -= gravity * DELTA_TIME;
    
    player->isOnGround = 0;

    

    player->position.x += player->velocity.x * DELTA_TIME;
    player->position.y += 0.01;
    if (playerCollides(player))
    {
        
        player->position.x -= player->velocity.x * DELTA_TIME;
        player->velocity.x = 0;
    }
    player->position.y -= 0.01;
    
    player->position.y += 0.01;
    player->position.z += player->velocity.z * DELTA_TIME;
    if (playerCollides(player))
    {
        player->position.z -= player->velocity.z * DELTA_TIME;
        player->velocity.z = 0;
    }
    player->position.y -= 0.01;

    player->position.y += player->velocity.y * DELTA_TIME;

    if (playerCollides(player))
    {
        if (player->velocity.y > 0)
        {
            float startY = player->position.y;
            while (playerCollides(player))
            {
                player->position.y -= 0.001f;
                if (startY - player->position.y > 2.0f)
                {
                    player->position.y = startY; // couldn't resolve - bail instead of falling forever
                    break;
                }
            }
        }
        else if (player->velocity.y < 0)
        {
            float startY = player->position.y;
            while (playerCollides(player))
            {
                player->position.y += 0.001f;
                if (player->position.y - startY > 2.0f)
                {
                    player->position.y = startY;
                    break;
                }
            }

            player->isOnGround = 1;
        }

        player->velocity.y = 0;
    }
}

void updateEntityPhysics(Entity *entity)
{
    if (totTime < 5.0) { return; }
    float gravity = (entity->isInWater) ? (5.0f) : (20.0f);

    int slopeDirCur = slopeDirEntity(entity);
    entity->velocity.y -= gravity * DELTA_TIME;
    
    entity->isOnGround = 0;

    

    entity->position.x += entity->velocity.x * DELTA_TIME;
    entity->position.y += 0.01;
    if (entityCollides(entity))
    {
        
        entity->position.x -= entity->velocity.x * DELTA_TIME;
        entity->velocity.x = 0;
    }
    entity->position.y -= 0.01;
    
    entity->position.y += 0.01;
    entity->position.z += entity->velocity.z * DELTA_TIME;
    if (entityCollides(entity))
    {
        entity->position.z -= entity->velocity.z * DELTA_TIME;
        entity->velocity.z = 0;
    }
    entity->position.y -= 0.01;

    entity->position.y += entity->velocity.y * DELTA_TIME;

    if (entityCollides(entity))
    {
        if (entity->velocity.y > 0)
        {
            float startY = entity->position.y;
            while (entityCollides(entity))
            {
                entity->position.y -= 0.001f;
                if (startY - entity->position.y > 2.0f)
                {
                    entity->position.y = startY; // couldn't resolve - bail instead of falling forever
                    break;
                }
            }
        }
        else if (entity->velocity.y < 0)
        {
            float startY = entity->position.y;
            while (entityCollides(entity))
            {
                entity->position.y += 0.001f;
                if (entity->position.y - startY > 2.0f)
                {
                    entity->position.y = startY;
                    break;
                }
            }

            entity->isOnGround = 1;
        }

        entity->velocity.y = 0;
    }

    // printf("finished entity physics\n");
}
