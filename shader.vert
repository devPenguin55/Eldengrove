#version 330 compatibility

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texCoord;
layout(location = 2) in float layer;
layout(location = 3) in int gpuLightIndex;
layout(location = 4) in int face;

out DATA
{
    vec2 fragUV;
    flat float fragLayer;
    vec3 worldPos;
    flat int fragGpuLightIndex;
    flat int fragFace;
} vsOut;

void main()
{
    vsOut.fragUV = texCoord;
    vsOut.fragLayer = layer;
    vsOut.worldPos = position;
    vsOut.fragGpuLightIndex = gpuLightIndex;
    vsOut.fragFace = face;
    
    gl_Position = gl_ModelViewProjectionMatrix * vec4(position, 1.0);
}