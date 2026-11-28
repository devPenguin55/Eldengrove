#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GL/glut.h>
#include "sceneManager.h"
#include "mountainBellScene.h"
#include "chunkLoaderManager.h"
#include "worldDiskStorage.h"
#include "chunks.h"
#include "render.h"
#include "entitySystem.h"
#include "input.h"
#include "player.h"

static int sceneResetRequested = 0;

void requestSceneReset() {
    sceneResetRequested = 1;
}

static void freeChunkWorld(void) {
    for (int b = 0; b < HASHMAP_AMOUNT_BUCKETS; b++) {
        BucketEntry *entry = chunkLoaderManager.hashmap.buckets[b].head;
        while (entry != NULL) {
            BucketEntry *next = entry->next;
            if (entry->chunkEntry != NULL) {
                free(entry->chunkEntry->lightData);
                free(entry->chunkEntry);
            }
            free(entry);
            entry = next;
        }
        chunkLoaderManager.hashmap.buckets[b].head = NULL;
    }

    free(chunkLoaderManager.loadedChunks.loadedChunks);
    chunkLoaderManager.loadedChunks.loadedChunks = NULL;
    chunkLoaderManager.loadedChunks.amtLoadedChunks = 0;
    chunkLoaderManager.loadedChunks.capacity = 0;

    free(chunkLoaderManager.renderChunks.renderChunks);
    chunkLoaderManager.renderChunks.renderChunks = NULL;
    chunkLoaderManager.renderChunks.amtRenderChunks = 0;
    chunkLoaderManager.renderChunks.capacity = 0;

    free(chunkLoaderManager.chunksToUnload.chunksToUnload);
    chunkLoaderManager.chunksToUnload.chunksToUnload = NULL;
    chunkLoaderManager.chunksToUnload.amtChunksToUnload = 0;
    chunkLoaderManager.chunksToUnload.capacity = 0;

    free(chunkLoaderManager.chunksToSaveToDisk.chunksToSaveToDisk);
    chunkLoaderManager.chunksToSaveToDisk.chunksToSaveToDisk = NULL;
    chunkLoaderManager.chunksToSaveToDisk.amtChunksToSaveToDisk = 0;
    chunkLoaderManager.chunksToSaveToDisk.capacity = 0;
    chunkLoaderManager.chunksToSaveToDisk.elapsedTime = 0;

    chunkLoaderManager.amtFreeSlots = 0;

    free(chunkMeshQuads.quads);
    chunkMeshQuads.quads = NULL;
    chunkMeshQuads.amtQuads = 0;
    chunkMeshQuads.capacity = 0;

    initialAmtLoadedChunksUntilMeshing = 0;
}

static void freeEntities(void) {
    for (int i = 0; i < worldEntities.amtEntities; i++) {
        free(worldEntities.entities[i].path);
        worldEntities.entities[i].path = NULL;
    }
    free(worldEntities.entities);
    worldEntities.entities = NULL;
    worldEntities.amtEntities = 0;
    worldEntities.capacity = 0;
}

static void freeModelNodeTree(ModelNode *node) {
    if (node == NULL) return;
    for (unsigned int i = 0; i < node->childCount; i++) {
        freeModelNodeTree(node->children[i]);
    }
    free(node->children);
    free(node);
}

static void freeModelManager(void) {
    for (int i = 0; i < modelManager.amtModels; i++) {
        Model *m = &modelManager.models[i];
        glDeleteVertexArrays(1, &m->vao);
        glDeleteBuffers(1, &m->vbo);
        glDeleteBuffers(1, &m->ebo);
        glDeleteBuffers(1, &m->instanceVBO);
        glDeleteTextures(1, &m->textureArray);
        free(m->instances);
    }
    free(modelManager.models);

    for (int i = 0; i < modelManager.amtAnimatedModels; i++) {
        AnimatedModel *m = &modelManager.animatedModels[i];

        glDeleteVertexArrays(1, &m->vao);
        glDeleteBuffers(1, &m->vbo);
        glDeleteBuffers(1, &m->ebo);
        glDeleteBuffers(1, &m->instanceVBO);
        glDeleteTextures(1, &m->textureArray);
        glDeleteBuffers(1, &m->boneMatrixBuffer);
        glDeleteTextures(1, &m->boneMatrixTexture);

        free(m->bones);

        for (unsigned int a = 0; a < m->animationCount; a++) {
            Animation *anim = &m->animations[a];
            for (unsigned int c = 0; c < anim->channelCount; c++) {
                free(anim->channels[c].positionKeys);
                free(anim->channels[c].rotationKeys);
                free(anim->channels[c].scaleKeys);
            }
            free(anim->channels);
        }
        free(m->animations);

        freeModelNodeTree(m->rootNode);

        for (unsigned int k = 0; k < m->instanceCount; k++) {
            free(m->instances[k].finalBoneMatrices);
        }
        free(m->instances);
    }
    free(modelManager.animatedModels);

    memset(&modelManager, 0, sizeof(modelManager));
}

static void freeRenderResources(void) {
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glUseProgram(0);

    free(worldVertices);
    worldVertices = NULL;
    worldVertexCount = 0;
    worldVertexCapacity = 0;

    free(waterVertices);
    waterVertices = NULL;
    waterVertexCount = 0;
    waterVertexCapacity = 0;

    glDeleteVertexArrays(1, &worldVAO);
    glDeleteBuffers(1, &worldVBO);
    glDeleteVertexArrays(1, &waterVAO);
    glDeleteBuffers(1, &waterVBO);
    worldVAO = worldVBO = waterVAO = waterVBO = 0;

    free(allChunkLighting);
    allChunkLighting = NULL;
    glDeleteBuffers(1, &lightBuffer);
    glDeleteTextures(1, &lightTex);
    lightBuffer = 0;
    lightTex = 0;

    glDeleteTextures(1, &blockTextureArray);
    blockTextureArray = 0;
    glDeleteTextures(9, destroyStageTextureArray);
    memset(destroyStageTextureArray, 0, sizeof(destroyStageTextureArray));

    glDeleteProgram(worldShader);
    glDeleteProgram(modelShader);
    glDeleteProgram(animatedModelShader);
    glDeleteShader(gs);
    worldShader = modelShader = animatedModelShader = gs = 0;
}

static void resetSceneGlobals(void) {
    memset(&selectedBlockToRender, 0, sizeof(selectedBlockToRender));

    for (int i = 0; i < 9; i++) {
        hotbarBlocks[i] = -1;
    }
    hotbarActiveSlot = -1;

    dayNightPhase = DAY;

    double now = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    lastTime = now;
    lastFpsTime = now;
    fps = 0.0;
    frameCount = 0;

    memset(blockRegistry, 0, sizeof(blockRegistry));

    player = (Player){
        .position.x = 33.3f,
        .position.y = 65.0f,
        .position.z = 0.0f,
        .velocity.x = 0.0f,
        .velocity.y = 0.0f,
        .velocity.z = 0.0f,
        .isOnGround = 0,
        .isInWater = 0,
        .width = 0.75f,
        .height = 1.8f};

    yaw = 0.0f;
    pitch = 0.0f;
    PlayerDirX = -5;
    PlayerDirY = 0;
    PlayerDirZ = -1;
    eyeX = player.position.x;
    eyeY = player.position.y + EYE_HEIGHT_OFFSET;
    eyeZ = player.position.z;

    memset(pressedKeys, 0, sizeof(pressedKeys));
    userBlockBreakingTimeElapsed = -1.0f;
    beginBlockBreakingIndex = -1;
    beginBlockBreakingBlockType = -1;

    totTime = 0.0f;
}

static void unloadCurrentScene(void) {
    freeChunkWorld();
    freeEntities();
    freeModelManager();
    freeRenderResources();
    resetSceneGlobals();
    initWorldDiskStorage();
}

static void loadScene(void) {
    initChunkLoaderManager();
    initChunkMeshingSystem();
    initGraphics(); 
}

void drawScene() {
    if (sceneResetRequested) {
        sceneResetRequested = 0;
        unloadCurrentScene();
        loadScene();
    }

    mountainBellSceneRender();
}