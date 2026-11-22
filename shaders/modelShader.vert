#version 330 compatibility

attribute vec3 position;
attribute vec3 normal;
attribute vec2 texCoord;
attribute float layer;
attribute vec3 instancePosition;
attribute vec3 instanceRotation;
attribute float instanceScale;

out vec3 fragNormal;
out vec2 fragTexCoord;
flat out float fragLayer;

mat3 rotationMatrix(vec3 rotation)
{
    float cx = cos(rotation.x);
    float sx = sin(rotation.x);

    float cy = cos(rotation.y);
    float sy = sin(rotation.y);

    float cz = cos(rotation.z);
    float sz = sin(rotation.z);

    mat3 rotationX = mat3(
        1.0, 0.0, 0.0,
        0.0, cx, -sx,
        0.0, sx, cx
    );

    mat3 rotationY = mat3(
        cy, 0.0, sy,
        0.0, 1.0, 0.0,
        -sy, 0.0, cy
    );

    mat3 rotationZ = mat3(
        cz, -sz, 0.0,
        sz, cz, 0.0,
        0.0, 0.0, 1.0
    );

    return rotationZ * rotationY * rotationX;
}

void main()
{
    mat3 rotation = rotationMatrix(instanceRotation);

    vec3 transformedPosition = position * instanceScale;

    transformedPosition = rotation * transformedPosition;

    transformedPosition += instancePosition;

    fragNormal = rotation * gl_NormalMatrix * normal;

    fragTexCoord = texCoord;
    fragLayer = layer;

    gl_Position =
        gl_ModelViewProjectionMatrix *
        vec4(transformedPosition, 1.0);
}


