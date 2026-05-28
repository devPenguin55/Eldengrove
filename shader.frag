#version 330 core
in vec2 fragUV;
flat in float fragLayer;
in vec3 worldPos;

out vec4 fragColor;

uniform sampler2DArray blockTextures;
uniform sampler3D lightVolume;

void main()
{
    int layer = int(fragLayer);
    vec4 texColor = texture(blockTextures, vec3(fragUV, layer));

    float brightness = texture(lightVolume, worldPos).r;

    texColor.rgb *= brightness;

    if (layer == 4)
    {
        texColor.a = 0.6;
    }

    if (texColor.a < 0.1)
    {
        discard;
    }

    fragColor = texColor;
}