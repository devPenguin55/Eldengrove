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

uniform mat4 finalBonesMatrices[MAX_BONES];
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);

    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (boneIds[i] < 0)
            continue;

        vec4 localPosition =
            finalBonesMatrices[boneIds[i]] * vec4(position, 1.0);

        totalPosition += localPosition * boneWeights[i];
        totalNormal +=
            mat3(finalBonesMatrices[boneIds[i]]) *
            normal *
            boneWeights[i];
    }

    if (totalPosition == vec4(0.0))
    {
        totalPosition = vec4(position, 1.0);
        totalNormal = normal;
    }

    totalPosition.xyz *= instanceScale;
    totalPosition.xyz += instancePosition;

    fragPos = totalPosition.xyz;
    fragNormal = gl_NormalMatrix * normalize(totalNormal);

    fragUV = texCoord;
    fragLayer = layer;
    fragLightPos = lightPos;
    fragViewPos = viewPos;

    gl_Position = gl_ModelViewProjectionMatrix * totalPosition;
}