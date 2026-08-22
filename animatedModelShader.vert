#version 330 core

#define MAX_BONE_INFLUENCE 4
#define MAX_BONES 200

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in int layer;
layout(location = 4) in ivec4 boneIds;
layout(location = 5) in vec4 boneWeights;

out vec2 fragUV;
flat out int fragLayer;
out vec3 fragNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 finalBonesMatrices[MAX_BONES];

void main()
{
    
    vec4 totalPosition = vec4(0.0);
    vec3 totalNormal = vec3(0.0);

    for(int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if(boneIds[i] < 0)
            continue;

        vec4 localPosition = finalBonesMatrices[boneIds[i]] * vec4(position,1.0);
        totalPosition += localPosition * boneWeights[i];

        totalNormal += mat3(finalBonesMatrices[boneIds[i]]) * normal * boneWeights[i];
    }

    if(totalPosition == vec4(0.0))
    {
        totalPosition = vec4(position,1.0);
        totalNormal = normal;
    }

    vec4 worldPosition = model * totalPosition;


    fragPos = worldPosition.xyz;
    fragNormal = mat3(transpose(inverse(model))) * normalize(totalNormal);
    fragUV = texCoord;
    fragLayer = layer;

    gl_Position = projection * view * worldPosition;
}