#include <stdio.h>
#include <stdlib.h>
#include "player.h"
#include "chunks.h"
#include "chunkLoaderManager.h"
#include "input.h"

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

float playerHalfWidth(Player* player)
{
    // return 0;
    return player->width * 0.5f;
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
                        float rampHeightLocal; switch (block->isSlope) { 
                            case 1: rampHeightLocal = 1 - localBlockZ; break; 
                            case 2: rampHeightLocal = 1 - localBlockX; break; 
                            case 3: rampHeightLocal = 1 - fabs(localBlockZ); break; 
                            case 4: rampHeightLocal = 1 - fabs(localBlockX); break; 
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


void updatePlayerPhysics(Player* player)
{
    const float gravity = (player->isInWater) ? (5.0f) : (20.0f);
    
    
    player->velocity.y -= gravity * DELTA_TIME;
    
    player->isOnGround = 0;


    float oldX = player->position.x;
    float oldY = player->position.y;

    player->position.x += player->velocity.x * DELTA_TIME;

    if (playerCollides(player))
    {
        int steppedUp = 0;

        for (int i = 1; i <= 8; i++)
        {
            player->position.y = oldY + i * 0.01f;

            if (!playerCollides(player))
            {
                steppedUp = 1;
                break;
            }
        }

        if (!steppedUp)
        {
            player->position.x = oldX;
            player->position.y = oldY;
            player->velocity.x = 0;
        }
    }

    player->position.z += player->velocity.z * DELTA_TIME;
    if (playerCollides(player))
    {
        player->position.z -= player->velocity.z * DELTA_TIME;
        player->velocity.z = 0;
    }

    // slope handling
    int feetY = (int)floor(player->position.y - 0.05f);
    Block *feetBlock = blockAtPosition(
        (int)floor(player->position.x),
        feetY,
        (int)floor(player->position.z));

    // if (feetBlock == NULL || feetBlock->isAir || !feetBlock->isSlope)
    // {
    //     // fallback: also check one layer up in case the rounding above
    //     // picked the wrong side of a block boundary
    //     Block *altBlock = blockAtPosition(
    //         (int)floor(player->position.x),
    //         feetY + 1,
    //         (int)floor(player->position.z));

    //     if (altBlock != NULL && !altBlock->isAir && altBlock->isSlope)
    //     {
    //         feetBlock = altBlock;
    //     }
    // }

    int slopeHandledThisFrame = 0;
    // if (feetBlock != NULL && !feetBlock->isAir && feetBlock->isSlope && player->velocity.y <= 0.0f)
    // {
    //     // printf("stuck %f\n", DELTA_TIME);
    //     float localBlockX = player->position.x - feetBlock->x;
    //     float localBlockZ = player->position.z - feetBlock->z;
        
    //     float rampHeightLocal;
    //     switch (feetBlock->isSlope)
    //     {
    //         case 1: rampHeightLocal = 1 - localBlockZ; break;
    //         case 2: rampHeightLocal = 1 - localBlockX; break; // 
    //         case 3: rampHeightLocal = 1 - fabsf(localBlockZ);  break;
    //         case 4: rampHeightLocal = 1 - fabsf(localBlockX);  break;
    //         default: rampHeightLocal = 0; break;
    //     }
    //     // printf("%f %f %f\n", localBlockX, localBlockZ, rampHeightLocal);

    //     float rampHeightWorld = feetBlock->y + rampHeightLocal-1;
    //     if (player->position.y < rampHeightWorld || fabsf(player->position.y - rampHeightWorld) < 0.1f) {
    //         player->position.y = rampHeightWorld;
    //         player->velocity.y = 0;
    //         player->isOnGround = 1;
    //         slopeHandledThisFrame = 1;

    //         // while (playerCollides(player))
    //         // {
    //         //     // switch (feetBlock->isSlope)
    //         //     // {
    //         //     //     case 1: rampHeightLocal = player->position.x-=0.0001;
    //         //     //     case 2: rampHeightLocal = player->position.z-=0.0001;
    //         //     //     case 3: rampHeightLocal = player->position.x+=0.0001; break;
    //         //     //     case 4: rampHeightLocal = player->position.z+=0.0001; break;
    //         //     //     default: rampHeightLocal = 0; break;
    //         //     // }

    //         //     player->position.y+=0.00001;
    //         // }
    //     }
    // }

    if (!slopeHandledThisFrame) {
        player->position.y += player->velocity.y * DELTA_TIME;
        if (playerCollides(player))
        {
            if (player->velocity.y > 0)
            {
                while (playerCollides(player))
                {
                    player->position.y -= 0.001f;
                }
            }
            else if (player->velocity.y < 0)
            {
                while (playerCollides(player))
                {
                    player->position.y += 0.001f;
                }

                player->isOnGround = 1;
            }

            player->velocity.y = 0;
        }
    }
}

