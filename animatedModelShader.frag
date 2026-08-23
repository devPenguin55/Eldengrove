#version 330 compatibility  
  
in vec2 fragUV;  
flat in float fragLayer;  
in vec3 fragNormal;  
in vec3 fragPos;  
in vec3 fragLightPos;  
in vec3 fragViewPos;  
  
out vec4 color;  
  
uniform sampler2DArray modelTextures;  
  
void main()  
{  
    vec4 texColor = texture(modelTextures, vec3(fragUV, int(fragLayer)));  
  
    if(texColor.a < 0.1)  
        discard;  
  
    vec3 norm = normalize(fragNormal);  
    vec3 lightDir = normalize(fragLightPos - fragPos);  
  
    float diffuse = max(dot(norm, lightDir), 0.0);  
  
    vec3 viewDir = normalize(fragViewPos - fragPos);  
    vec3 reflectDir = reflect(-lightDir, norm);  
  
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);  
  
    vec3 lighting = vec3(0.15) + diffuse + specular * 0.25;  
  
    color = vec4(texColor.rgb * lighting, texColor.a);
}