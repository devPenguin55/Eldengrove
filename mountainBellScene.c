#include <stdint.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include "render.h"
#include "input.h"
#include "chunks.h"
#include "chunkLoaderManager.h"
#include "raycast.h"
#include "entitySystem.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MIN4(a, b, c, d) (MIN(MIN(a, b), MIN(c, d)))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MAX4(a, b, c, d) (MAX(MAX(a, b), MAX(c, d)))



void mountainBellSceneRender()
{
    frameCount++;
    double currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0; // seconds
    // update FPS every 0.25 seconds
    if (currentTime - lastFpsTime >= 0.25)
    {
        fps = frameCount / (currentTime - lastFpsTime);
        frameCount = 0;
        lastFpsTime = currentTime;
    }
    DELTA_TIME = (currentTime - lastTime);
    lastTime = currentTime;


    float timeSun = glutGet(GLUT_ELAPSED_TIME) / 4000.0f;
    float orbitRadius = ChunkWidthX * CHUNK_RENDER_RADIUS * 2;

    float sunAngle = timeSun;
    float sunX = player.position.x + orbitRadius * cosf(sunAngle);
    float sunY = 50.0f + orbitRadius * sinf(sunAngle);
    float dayCycleProgress = fmodf(sunAngle, 2.0f * M_PI);

    float sunZ = player.position.z;
    
    float sunSize = 30.0f;

    handleUserMovement();
    updatePlayerPhysics(&player);
    raycastFromCamera();
    chunkSaveOnInterval();

    eyeX = player.position.x;
    eyeY = player.position.y + EYE_HEIGHT_OFFSET;
    eyeZ = player.position.z;

    GLfloat playerCoords[2] = {player.position.x, player.position.z};
    loadChunks(playerCoords);
    createWorldLightingDataFromAllChunks();
    uploadLighting();

    GLfloat Vertices[8][3] = {
        // front face
        {-0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5},
        {-0.5, -0.5, 0.5},
        // back face
        {-0.5, 0.5, -0.5},
        {0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5},
        {-0.5, -0.5, -0.5},
    };

    // clear color buffer to clear background, uses preset color setup in initGraphics
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float sunset[] = {0.85f, 0.45f, 0.30f};
    float skyBlue[] = {0.20f, 0.42f, 0.75f};
    float nightSky[] = {0.015f, 0.025f, 0.10f};

    float t = dayCycleProgress;

    float* colorA;
    float* colorB;
    float blend;

    if (t >= 3.0f*M_PI/2.0f && t <= (2.0*M_PI - 0.5f)) {
        // night
        dayNightPhase = NIGHT;
        colorA = nightSky;
        colorB = nightSky;
        blend = 1.0f;
    } else if (t > (2*M_PI -0.5f) || t < 0.30f) {
        // sunrise
        colorA = nightSky;
        colorB = sunset;

        dayNightPhase = SUNRISE;
        if (t >M_PI) {
            blend = (t - (2*M_PI-0.5f)) / (0.3f); 
        } else {
            blend = 1.0f;
        }

        if (blend > 1.0f || blend < 0.0f) {
            blend = 1.0f;
        }
        blend = MIN(blend, 1.0f);

    } else if (t < (M_PI - 0.6f)) {
        // day
        colorA = sunset;
        colorB = skyBlue;
        blend = (t - 0.3) / 0.30f;
        dayNightPhase = DAY;
        if (blend > 1.0f || blend < 0.0f) {

            blend = 1.0f;
        }
        blend = MIN(blend, 1.0f);
    } else if (t < (M_PI + 0.5f)) {
        // if (dayNightPhase != SUNSET) {
        //     transformChunkIntoNightChunk(selectedBlockToRender.chunk);
        //     lastTime = currentTime;
        //     currentTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
        //     DELTA_TIME = 0.0f;
        // }
        // sunset
        dayNightPhase = SUNSET;
        colorA = skyBlue;
        colorB = sunset;

        blend = (t - (M_PI - 0.6f)) / 0.30f;
        if (blend > 1.0f || blend < 0.0f) {
            blend = 1.0f;
        }
        blend = MIN(blend, 1.0f);
    } else {
        dayNightPhase = NIGHT;
        // back to night
        colorA = sunset;
        colorB = nightSky;
        blend = (t - (M_PI+0.5)) / 0.30f;
        if (blend > 1.0f || blend < 0.0f) {
            blend = 1.0f;
        }
        blend = MIN(blend, 1.0f);
    } 
    

    float result[] = {
        colorA[0] + (colorB[0] - colorA[0]) * blend,
        colorA[1] + (colorB[1] - colorA[1]) * blend,
        colorA[2] + (colorB[2] - colorA[2]) * blend
    };

    glClearColor(result[0], result[1], result[2], 0.0f);

    // camera
    glLoadIdentity();
    // eye -> where the camera is located in the world space
    // center -> where the camera is looking at
    // up -> what direction is up for the camera
    // eyeX = cos(T)*15;
    // eyeZ = sin(T)*15;
    gluLookAt(
        eyeX, eyeY, eyeZ,
        eyeX + PlayerDirX, eyeY + PlayerDirY, eyeZ + PlayerDirZ,
        0, 1, 0);

    glPointSize(5);

    // clip = modelview * projection
    // clip = proj * modelview
    GLfloat clip[16];
    glGetFloatv(GL_PROJECTION_MATRIX, clip);

    GLfloat modelview[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

    glPushMatrix();
    glLoadMatrixf(clip);
    glMultMatrixf(modelview);
    glGetFloatv(GL_MODELVIEW_MATRIX, clip);
    glPopMatrix();

    if (pressedKeys['c'])
    {
        // optionally draw the chunk borders for debug purposes
        int chunkX = (player.position.x >= 0)
                         ? player.position.x / ChunkWidthX
                         : (player.position.x - (ChunkWidthX - 1)) / ChunkWidthX;

        int chunkZ = (player.position.z >= 0)
                         ? player.position.z / ChunkLengthZ
                         : (player.position.z - (ChunkLengthZ - 1)) / ChunkLengthZ;

        int localX = player.position.x - chunkX * ChunkWidthX;
        int localY = player.position.y;
        int localZ = player.position.z - chunkZ * ChunkLengthZ;

        uint64_t chunkKey = packChunkKey(chunkX, chunkZ);
        BucketEntry *result = getHashmapEntry(chunkKey);

        if (result)
        {
            Chunk *chunk = result->chunkEntry;
            for (int dy = 0; dy <= 50; dy++)
            {
                glBegin(GL_LINE_LOOP);
                glVertex3f(chunk->chunkStartX-0.5, ChunkHeightY * dy / 50.0f+0.5, chunk->chunkStartZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5, ChunkHeightY * dy / 50.0f+0.5, chunk->chunkStartZ + ChunkLengthZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX, ChunkHeightY * dy / 50.0f+0.5, chunk->chunkStartZ + ChunkLengthZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX, ChunkHeightY * dy / 50.0f+0.5, chunk->chunkStartZ-0.5);
                glEnd();
            }
            glBegin(GL_LINES);
            for (int dx = 0; dx <= 5; dx++)
            {
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX * dx / 5.0f, 0.5, chunk->chunkStartZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX * dx / 5.0f, ChunkHeightY+0.5, chunk->chunkStartZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX * dx / 5.0f, 0.5, chunk->chunkStartZ + ChunkLengthZ-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX * dx / 5.0f, ChunkHeightY+0.5, chunk->chunkStartZ + ChunkLengthZ-0.5);
            }

            for (int dz = 0; dz <= 5; dz++)
            {
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX, 0.5, chunk->chunkStartZ + (ChunkLengthZ * dz / 5.0f)-0.5);
                glVertex3f(chunk->chunkStartX-0.5 + ChunkWidthX, ChunkHeightY+0.5, chunk->chunkStartZ + (ChunkLengthZ * dz / 5.0f)-0.5);
                glVertex3f(chunk->chunkStartX-0.5, 0.5, chunk->chunkStartZ + (ChunkLengthZ * dz / 5.0f)-0.5);
                glVertex3f(chunk->chunkStartX-0.5, ChunkHeightY+0.5, chunk->chunkStartZ + (ChunkLengthZ * dz / 5.0f)-0.5);
            }
            glEnd();
        }
    }

    

    // for (unsigned int i = 0; i < modelManager.amtModels; i++) {
    //     renderModel(&modelManager.models[i]);
    // }
    // updateModelInstance(&modelManager.models[0], 0, &(Vec3){33.0f, 60.0f, 0.0f}, &(Vec3){currentTime, 0.0f, 0.0f}, 1.0f);
    // updateModelInstance(&modelManager.models[0], 1, &(Vec3){27.0f, 60.0f, 0.0f}, &(Vec3){0.0f, currentTime, 0.0f}, 1.0f);
    // updateModelInstance(&modelManager.models[1], 0, &(Vec3){39.0f, 60.0f, 0.0f}, &(Vec3){0.0f, 0.0f, currentTime}, 1.0f);
    worldEntityUpdate();    

    buildWorldMesh(); // fills worldVertices and worldVertexCount

    glUseProgram(worldShader);
    // printf("%f %f %f\n", player.position.x, player.position.y, player.position.z);
    glUniform3f(
        glGetUniformLocation(worldShader, "cameraPosition"),
        player.position.x,
        player.position.y,
        player.position.z
    );

    glUniform3f(
        glGetUniformLocation(worldShader, "fogColor"),
        result[0],
        result[1],
        result[2]
    );

    glUniform1f(
        glGetUniformLocation(worldShader, "fogStart"),
        0.0f
    );

    glUniform1f(
        glGetUniformLocation(worldShader, "fogEnd"),
        90.0f
    );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, blockTextureArray);
    glUniform1i(glGetUniformLocation(worldShader, "blockTextures"), 0);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, lightTex);
    glUniform1i(glGetUniformLocation(worldShader, "lightBuffer"), 1);
    GLint playerPositionLocation = glGetUniformLocation(worldShader, "playerPosition");
    glUniform3fv(playerPositionLocation, 1, &player.position.x);
    GLint timeLocation = glGetUniformLocation(worldShader, "time");
    glUniform1f(timeLocation, currentTime);

    glBindVertexArray(worldVAO);
    // for (int i = 0; i < chunkLoaderManager.renderChunks.amtRenderChunks; i++) {
    //     Chunk *chunk = chunkLoaderManager.renderChunks.renderChunks[i];
    //     // if (!chunk->hasVertices || chunk->firstVertex == -1 || chunk->lastVertex == -1) {
    //     //     printf(
    //     //         "Chunk %d: first=%d last=%d count=%d\n",
    //     //         i,
    //     //         chunk->firstVertex,
    //     //         chunk->lastVertex,
    //     //         chunk->lastVertex - chunk->firstVertex + 1
    //     //     );
    //     //     continue;
    //     // }
    //     glDrawArrays(GL_TRIANGLES, chunk->firstVertex, chunk->lastVertex - chunk->firstVertex + 1);
    // }
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Wireframe
    glDrawArrays(GL_TRIANGLES, 0, worldVertexCount);
    glBindVertexArray(0);


    // * ////////////

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);

    glDisable(GL_CULL_FACE);


    glBindVertexArray(waterVAO);
    // for (int i = 0; i < chunkLoaderManager.renderChunks.amtRenderChunks; i++)
    // {
    //     Chunk *c = chunkLoaderManager.renderChunks.renderChunks[i];
    //     if (!c->hasWaterVertices || c->firstWaterVertex == -1 || c->lastWaterVertex == -1)
    //         continue;
    //     // if (!chunkInFrustum(c, frustumPlanes))
    //     //     continue;

    //     glDrawArrays(GL_TRIANGLES, c->firstWaterVertex, c->lastWaterVertex - c->firstWaterVertex + 1);
    // }
    glDrawArrays(GL_TRIANGLES, 0, waterVertexCount);

    glBindVertexArray(0);
    glEnable(GL_CULL_FACE);

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glUseProgram(0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    renderEntityBoundingBox(&worldEntities.entities[0]);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_TEXTURE_2D_ARRAY);
    glDisable(GL_TEXTURE_3D);   

    for (int i = 0; i < 16; i++)
        glDisableVertexAttribArray(i);

    glPushAttrib(GL_ALL_ATTRIB_BITS);



    float rightX = -sinf(sunAngle);
    float rightY = cosf(sunAngle);
    float rightZ = 0.0f;

    float upX = 0.0f;
    float upY = 0.0f;
    float upZ = 1.0f;

    glDisable(GL_CULL_FACE);
    glUseProgram(0);
    glColor3f(1.0f, 0.85f, 0.0f);
    
    glBegin(GL_QUADS);

    glVertex3f(sunX - rightX*sunSize - upX*sunSize,
               sunY - rightY*sunSize - upY*sunSize,
               sunZ - rightZ*sunSize - upZ*sunSize);

    glVertex3f(sunX + rightX*sunSize - upX*sunSize,
        sunY + rightY*sunSize - upY*sunSize,
        sunZ + rightZ*sunSize - upZ*sunSize);
        
    glVertex3f(sunX + rightX*sunSize + upX*sunSize,
        sunY + rightY*sunSize + upY*sunSize,
               sunZ + rightZ*sunSize + upZ*sunSize);
               
    glVertex3f(sunX - rightX*sunSize + upX*sunSize,
        sunY - rightY*sunSize + upY*sunSize,
        sunZ - rightZ*sunSize + upZ*sunSize);
    glEnd();
    glPopAttrib();
        
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST); 


    if (selectedBlockToRender.active)
    {
        GLfloat translation[3];
        translation[0] = selectedBlockToRender.worldX;
        translation[1] = selectedBlockToRender.worldY;
        translation[2] = selectedBlockToRender.worldZ;
        GLfloat size[2] = {BlockWidthX, BlockLengthZ};

        float outlineScale = 1.f;
        float offset = (outlineScale - 1.0f) * 0.5f;

        translation[0] -= offset;
        translation[1] -= offset;
        translation[2] -= offset;

        size[0] *= outlineScale;
        size[1] *= outlineScale;

        glPushAttrib(GL_ALL_ATTRIB_BITS);
        for (int i = 1; i < 7; i++)
        {
            cubeFace(Vertices, translation, size, i, BLOCK_TYPE_SELECT);
        }

        glPopAttrib();

        glColor3f(1.0f, 1.0f, 1.0f);
    }

    glUseProgram(0);
    glBindVertexArray(0);
    glDepthMask(GL_TRUE);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    int windowWidth = glutGet(GLUT_WINDOW_WIDTH);
    int windowHeight = glutGet(GLUT_WINDOW_HEIGHT);

    gluOrtho2D(0, windowWidth, windowHeight, 0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);

    // draw 2d things
    int inWater = isCameraInWater();
    if (inWater)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        float depthAlpha = MAX(MIN((SEA_LEVEL + 2 - eyeY) / 25, 0.0), 0.05);
        glColor4f(0.0f, 0.3f, 0.8f, depthAlpha);
        glBegin(GL_QUADS);
        glVertex2f(0, 0);
        glVertex2f(0, windowHeight);
        glVertex2f(windowWidth, windowHeight);
        glVertex2f(windowWidth, 0);
        glEnd();
        glDisable(GL_BLEND);
        player.isInWater = 1;
    }
    else
    {
        player.isInWater = 0;
    }

    glColor3f(1.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2f(windowWidth / 2, windowHeight / 2 - 15);
    glVertex2f(windowWidth / 2, windowHeight / 2 + 15);
    glVertex2f(windowWidth / 2 - 15, windowHeight / 2);
    glVertex2f(windowWidth / 2 + 15, windowHeight / 2);
    glEnd();

    char text[64];
    snprintf(text, sizeof(text), "FPS: %.1f, Vertices: %d", fps, worldVertexCount + waterVertexCount);

    glColor3f(1, 1, 1);
    drawText(text, 5, 15);

    glColor3f(1.0f, 1.0f, 1.0f);

    ////////////////////////////////////////

    float slotSize = 40.0f;
    float padding = 4.0f;
    int slotCount = 9;

    float barWidth = slotCount * slotSize + (slotCount - 1) * padding;
    float barHeight = slotSize;

    float barLeft = (windowWidth - barWidth) / 2.0f;

    float barBottom = windowHeight - 50.0f;
    float barTop = barBottom + barHeight;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
    glVertex2f(barLeft, barBottom);
    glVertex2f(barLeft, barTop);
    glVertex2f(barLeft + barWidth, barTop);
    glVertex2f(barLeft + barWidth, barBottom);
    glEnd();
    glColor3f(0.6f, 0.6f, 0.6f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(barLeft, barBottom);
    glVertex2f(barLeft, barTop);
    glVertex2f(barLeft + barWidth, barTop);
    glVertex2f(barLeft + barWidth, barBottom);
    glEnd();

    for (int i = 0; i < slotCount; i++)
    {
        float x0 = barLeft + i * (slotSize + padding);
        float x1 = x0 + slotSize;

        glColor3f(0.7f, 0.7f, 0.7f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(x0, barBottom);
        glVertex2f(x0, barTop);
        glVertex2f(x1, barTop);
        glVertex2f(x1, barBottom);
        glEnd();

        if (hotbarBlocks[i] == -1)
            continue;

        float layer = blockRegistry[hotbarBlocks[i]].sideTexture;

        float pad = 6.0f;
        float ix0 = x0 + pad;
        float ix1 = x1 - pad;
        float iy0 = barBottom + pad;
        float iy1 = barTop - pad;

        glUseProgram(worldShader);
        glBindAttribLocation(worldShader, 2, "layer");
        glBindAttribLocation(worldShader, 3, "gpuLightIndex");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, blockTextureArray);

        glBegin(GL_TRIANGLES);
        layer = -55 + layer;

        glVertexAttrib2f(1, 0.0f, 0.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix0, iy0);

        glVertexAttrib2f(1, 1.0f, 0.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix1, iy0);

        glVertexAttrib2f(1, 1.0f, 1.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix1, iy1);

        glVertexAttrib2f(1, 0.0f, 0.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix0, iy0);

        glVertexAttrib2f(1, 1.0f, 1.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix1, iy1);

        glVertexAttrib2f(1, 0.0f, 1.0f);
        glVertexAttrib1f(2, layer);
        glVertexAttrib1d(3, 55555);
        glVertex2f(ix0, iy1);

        glEnd();
        glUseProgram(0);
    }

    if (hotbarActiveSlot != -1)
    {
        int selected = hotbarActiveSlot;

        float selX0 = barLeft + selected * (slotSize + padding);
        float selX1 = selX0 + slotSize;

        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(3.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(selX0, barBottom);
        glVertex2f(selX0, barTop);
        glVertex2f(selX1, barTop);
        glVertex2f(selX1, barBottom);
        glEnd();
    }

    ////////////////////

    glColor3f(1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // switch the content of color and depth buffers
    glutSwapBuffers();
    // printf("%f\n", player.position.y);
    if (userBlockBreakingTimeElapsed != -1.f)
    {
        userBlockBreakingTimeElapsed += DELTA_TIME;

        if (!selectedBlockToRender.active)
        {
            return;
        }
        int x, y, z;
        x = selectedBlockToRender.localX;
        y = selectedBlockToRender.localY;
        z = selectedBlockToRender.localZ;

        int chunkXUnit = ChunkWidthX * BlockWidthX;
        int chunkZUnit = ChunkLengthZ * BlockLengthZ;

        // break blocks
        int blockType = selectedBlockToRender.chunk->blocks[x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y].blockType;


        if (userBlockBreakingTimeElapsed >= blockRegistry[blockType].blockBreakingTime && beginBlockBreakingIndex == (x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y))
        {
            selectedBlockToRender.chunk->blocks[x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y].isAir = 1;
            selectedBlockToRender.chunk->blocks[x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y].isSlope = 0;
            userBlockBreakingTimeElapsed = -1.f;
            selectedBlockToRender.chunk->isDirty = 1;

            for (int i = 0; i < 9; i++)
            {
                if (hotbarBlocks[i] == blockType)
                {
                    break;
                }
                if (hotbarBlocks[i] == -1)
                {
                    hotbarBlocks[i] = blockType;
                    break;
                }
            }

            triggerRenderChunkRebuild(selectedBlockToRender.chunk);
        }
        else
        {
            if (beginBlockBreakingIndex == -1)
            {
                beginBlockBreakingIndex = (x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y);
                beginBlockBreakingBlockType = blockType;
            }
            else if (beginBlockBreakingIndex != (x + (ChunkWidthX)*z + (ChunkWidthX * ChunkLengthZ) * y))
            {
                userBlockBreakingTimeElapsed = -1.f;
            }
            return;
        }

        
        if (x == 0)
        {
            uint64_t chunkKey = packChunkKey(
                (int)((selectedBlockToRender.chunk->chunkStartX - chunkXUnit) / (chunkXUnit)),
                (int)((selectedBlockToRender.chunk->chunkStartZ) / (chunkZUnit)));
            BucketEntry *result = getHashmapEntry(chunkKey);
            triggerRenderChunkRebuild(result->chunkEntry);
        }
        else if (x == ChunkWidthX - 1)
        {
            uint64_t chunkKey = packChunkKey(
                (int)((selectedBlockToRender.chunk->chunkStartX + chunkXUnit) / (chunkXUnit)),
                (int)((selectedBlockToRender.chunk->chunkStartZ) / (chunkZUnit)));
            BucketEntry *result = getHashmapEntry(chunkKey);
            triggerRenderChunkRebuild(result->chunkEntry);
        }

        if (z == 0)
        {
            uint64_t chunkKey = packChunkKey(
                (int)((selectedBlockToRender.chunk->chunkStartX) / (chunkXUnit)),
                (int)((selectedBlockToRender.chunk->chunkStartZ - chunkZUnit) / (chunkZUnit)));
            BucketEntry *result = getHashmapEntry(chunkKey);
            triggerRenderChunkRebuild(result->chunkEntry);
        }
        else if (z == ChunkLengthZ - 1)
        {
            uint64_t chunkKey = packChunkKey(
                (int)((selectedBlockToRender.chunk->chunkStartX) / (chunkXUnit)),
                (int)((selectedBlockToRender.chunk->chunkStartZ + chunkZUnit) / (chunkZUnit)));
            BucketEntry *result = getHashmapEntry(chunkKey);
            triggerRenderChunkRebuild(result->chunkEntry);
        }

        blockPlacingOrBreakingLightingRecalculation(selectedBlockToRender.chunk);
    }
}
