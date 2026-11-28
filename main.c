#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <GL/freeglut.h>
#include <math.h>
#include "input.h"
#include "render.h"
#include "chunks.h"
#include "chunkLoaderManager.h" 
#include "worldDiskStorage.h"
#include "audio.h"
#include "sceneManager.h"

int main(int argc, char *argv[]) {
    initWorldDiskStorage();
    initChunkLoaderManager();
    initChunkMeshingSystem();

    initAudio();

    glutInit(&argc, argv);
    
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(1000, 400);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH); // rgb model, double buffer -> (color, depth)
    glutCreateWindow("Eldengrove");
    glewInit();
    glutSetCursor(GLUT_CURSOR_NONE);
 
    initGraphics();

    glutReshapeFunc(reshape);
    glutDisplayFunc(drawScene); 
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutIdleFunc(spinObject);
    glutMouseFunc(handleMouse);
    glutPassiveMotionFunc(handleMovingMouse);
    glutMotionFunc(handleMovingMouse);
    glutCloseFunc(handleProgramClose);
    glutMainLoop();

    return 0;
}