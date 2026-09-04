#version 330 compatibility

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 200

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in float layer;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 boneWeights;

layout(location = 6) in vec3 instancePosition;
layout(location = 7) in vec3 instanceRotation;
layout(location = 8) in float instanceScale;

out vec2 fragUV;
flat out float fragLayer;
out vec3 fragNormal;
out vec3 fragPos;
out vec3 fragLightPos;
out vec3 fragViewPos;

uniform samplerBuffer boneMatrices;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform int boneCount;

mat4 getBoneMatrix(int instanceIndex, int boneIndex)
{
    int matrixIndex = instanceIndex * boneCount + boneIndex;
    int texelIndex = matrixIndex * 4;

    vec4 c0 = texelFetch(boneMatrices, texelIndex + 0);
    vec4 c1 = texelFetch(boneMatrices, texelIndex + 1);
    vec4 c2 = texelFetch(boneMatrices, texelIndex + 2);
    vec4 c3 = texelFetch(boneMatrices, texelIndex + 3);

    return transpose(mat4(c0, c1, c2, c3));
}

mat4 rotationX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, c,   -s,  0.0,
        0.0, s,   c,   0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        c,   0.0, s,   0.0,
        0.0, 1.0, 0.0, 0.0,
        -s,  0.0, c,   0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotationZ(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    return mat4(
        c,   -s,  0.0, 0.0,
        s,   c,   0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main()
{
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);

    bool hasBone = false;

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (boneIds[i] < 0)
            continue;

        hasBone = true;

        mat4 boneMatrix =
            getBoneMatrix(gl_InstanceID, boneIds[i]);

        vec4 localPosition =
            boneMatrix * vec4(position, 1.0);

        totalPosition +=
            localPosition * boneWeights[i];

        totalNormal +=
            mat3(boneMatrix) *
            normal *
            boneWeights[i];
    }

    if (!hasBone)
    {
        totalPosition = vec4(position, 1.0);
        totalNormal = normal;
    }

    vec3 rotationAngles = radians(instanceRotation);

    mat4 rotation =
        rotationZ(rotationAngles.z) *
        rotationY(rotationAngles.y) *
        rotationX(rotationAngles.x);

    totalPosition.xyz *= instanceScale;

    totalPosition = rotation * totalPosition;

    totalPosition.xyz += instancePosition;

    totalNormal =
        mat3(rotation) * totalNormal;

    fragPos = totalPosition.xyz;

    fragNormal =
        gl_NormalMatrix *
        normalize(totalNormal);

    fragUV = texCoord;
    fragLayer = layer;

    fragLightPos = lightPos;
    fragViewPos = viewPos;

    gl_Position =
        gl_ModelViewProjectionMatrix *
        totalPosition;
}