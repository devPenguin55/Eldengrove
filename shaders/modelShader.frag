#version 330 compatibility

in vec3 fragNormal;
in vec2 fragTexCoord;
flat in float fragLayer;

uniform sampler2DArray modelTextures;

uniform vec3 lightDirection;
uniform vec3 lightColor;

void main()
{    
    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(-lightDirection);

    float diffuse = max(dot(normal, lightDir), 0.0);

    float ambient = 0.9;

    vec4 textureColor;
    
    if (fragLayer >= 0.0) {
        
        textureColor = texture(modelTextures, vec3(fragTexCoord, fragLayer));
    } else {
        textureColor = vec4(1.0, 1.0, 1.0, 1.0); // no texture material
    }

    if (textureColor.a < 0.1)
    {
        discard;
    }

    vec3 lighting = vec3(ambient) + diffuse * lightColor;

    gl_FragColor = vec4(
        textureColor.rgb * lighting,
        1.0
    );

}