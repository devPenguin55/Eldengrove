#ifndef RENDER_H
#define RENDER_H
#include <GL/glu.h>
#include <GL/glut.h>
#include "chunks.h"
#include "vectors.h"


#define MAX_BONE_INFLUENCE 4

typedef struct UV
{
    float u;
    float v;
    float u1;
    float v1;
} UV;

typedef struct SelectedBlockToRender
{
    int active;
    float worldX;
    float worldY;
    float worldZ;
    float localX;
    float localY;
    float localZ;
    int hitFace;
    Chunk *chunk;
    int amtSteps;
} SelectedBlockToRender;

typedef struct Vertex
{
    float x, y, z; // position
    float u, v;    // texcoords
    float layer;
    int gpuLightIndex;
    int face;
} Vertex;

typedef struct ModelVertex {
    float position[3];
    float normal[3];
    float texCoord[2];
    float layer;
} ModelVertex;

typedef struct AnimatedModelVertex{
    float position[3];
    float normal[3];
    float texCoord[2];
    float layer;
    int boneIds[MAX_BONE_INFLUENCE];
    float boneWeights[MAX_BONE_INFLUENCE];
} AnimatedModelVertex;

typedef struct ModelNode {  
    char name[128];  
    Mat4 transformation; // local bind-pose transform (from aiNode->mTransformation)  
    struct ModelNode *parent;  
    struct ModelNode **children;  
    unsigned int childCount;  
} ModelNode;

typedef struct TextureData {
    unsigned char *pixels;
    int width;
    int height;
    int channels;
} TextureData;

typedef struct ModelInstance {
    float position[3];
    float rotation[3];
    float scale;
} ModelInstance;

// typedef struct AnimatedModelInstance{
//     float position[3];
//     float rotation[3];
//     float scale;

//     int animationIndex;
//     float animationTime;
// } AnimatedModelInstance;

typedef struct Model {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint instanceVBO;
    GLuint textureArray;
    unsigned int indexCount;
    
    ModelInstance *instances;
    unsigned int instanceCount;
    unsigned int instanceCapacity;
} Model;

typedef struct BoneInfo {
    char name[128];

    Mat4 offsetMatrix;
} BoneInfo;

typedef struct Bone {
    char name[128];
    int id;
    Mat4 offsetMatrix;
    Mat4 localTransform;
    Mat4 globalTransform;
    struct Bone *parent;
} Bone;

typedef struct PositionKey {
    double time;
    Vec3 value;
} PositionKey;

typedef struct RotationKey {
    double time;
    Quat value;
} RotationKey;

typedef struct ScaleKey {
    double time;
    Vec3 value;
} ScaleKey;

typedef struct AnimationChannel {
    char nodeName[128];

    PositionKey *positionKeys;
    unsigned int positionKeyCount;

    RotationKey *rotationKeys;
    unsigned int rotationKeyCount;

    ScaleKey *scaleKeys;
    unsigned int scaleKeyCount;
} AnimationChannel;

typedef struct Animation {
    char name[128];

    double duration;
    double ticksPerSecond;

    AnimationChannel *channels;
    unsigned int channelCount;
} Animation;


typedef struct AnimatedModelInstanceData {
    float position[3];
    float rotation[3];
    float scale;
} AnimatedModelInstanceData;

typedef struct AnimatedModelInstance {
    AnimatedModelInstanceData transform;

    unsigned int animationIndex;
    float animationTime;

    Mat4 *finalBoneMatrices;
} AnimatedModelInstance;

typedef struct AnimatedModel {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint instanceVBO;
    GLuint textureArray;
    unsigned int indexCount;

    GLuint boneMatrixBuffer;
    GLuint boneMatrixTexture;
    
    BoneInfo *bones;
    unsigned int boneCount;

    Animation *animations;
    unsigned int animationCount;

    ModelNode *rootNode;
    Mat4 globalInverseTransform;


    AnimatedModelInstance *instances;
    unsigned int instanceCount;
    unsigned int instanceCapacity;
} AnimatedModel;

typedef struct ModelManager {
    int amtModels;
    int  capacity;
    Model *models;

    int amtAnimatedModels;
    int animatedModelCapacity;
    AnimatedModel *animatedModels;
} ModelManager;

extern GLfloat T;
extern GLfloat PlayerDirX;
extern GLfloat PlayerDirY;
extern GLfloat PlayerDirZ;
extern SelectedBlockToRender selectedBlockToRender;
extern GLuint worldVBO;
extern GLuint worldVAO;
extern Vertex *worldVertices;
extern int worldVertexCount;
extern int worldVertexCapacity;
extern Vertex *waterVertices;
extern int waterVertexCount;
extern int waterVertexCapacity;
extern int hotbarBlocks[9];
extern int hotbarActiveSlot;
extern uint8_t *allChunkLighting;
extern ModelManager modelManager;

void initGraphics();
void createWorldLightingDataFromAllChunks();
void reshape(int width, int height);
void spinObject();
void adjustVerticesForQuadData(
    Vertex *v0, 
    Vertex *v1, 
    Vertex *v2, 
    Vertex *v3, 
    float x, 
    float y, 
    float z, 
    int face
);
void face(
    GLfloat A[3],
    GLfloat B[3],
    GLfloat C[3],
    GLfloat D[3],
    GLfloat transformation[3],
    GLuint texture,
    GLfloat size[2]);
void cubeFace(GLfloat Vertices[8][3], GLfloat transformation[3], GLfloat size[2], int faceType, int blockType);
void drawText(const char *text, float x, float y);
void drawGraphics();
void checkForWorldChunkVerticesDeletion();
void buildWorldMesh();
void uploadWorldMesh();

#endif