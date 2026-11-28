#include "sceneManager.h"
#include "mountainBellScene.h"
#include "chunkLoaderManager.h"
#include "chunks.h"
#include "render.h"

void drawScene() {
    free(chunkMeshQuads.quads);
    // ! make sure to delete like all of the memory
    for (int i = 0; i < chunkLoaderManager.loadedChunks.amtLoadedChunks; i++) {
        if (chunkLoaderManager.loadedChunks.loadedChunks[i]->lightData != NULL) {
            free(chunkLoaderManager.loadedChunks.loadedChunks[i]->lightData);
        }
    }

    for (int i = 0; i < chunkLoaderManager.renderChunks.amtRenderChunks; i++) {
        if (chunkLoaderManager.renderChunks.renderChunks[i]->lightData != NULL) {
            free(chunkLoaderManager.renderChunks.renderChunks[i]->lightData);
        }
    }

    for (int i = 0; i < chunkLoaderManager.chunksToUnload.amtChunksToUnload; i++) {
        if (chunkLoaderManager.chunksToUnload.chunksToUnload[i]->lightData != NULL) {
            free(chunkLoaderManager.chunksToUnload.chunksToUnload[i]->lightData);
        }
    }

    free(chunkLoaderManager.loadedChunks.loadedChunks);
    free(chunkLoaderManager.renderChunks.renderChunks);
    free(chunkLoaderManager.chunksToUnload.chunksToUnload);
    free(chunkLoaderManager.chunksToSaveToDisk.chunksToSaveToDisk);




    for (int i = 0; i < worldEntities.amtEntities; i++) {
        free(worldEntities.entities[i].path);
    }
    free(worldEntities.entities);

    initChunkLoaderManager();
    initChunkMeshingSystem();
    initGraphics();


    mountainBellSceneRender();
}