#version 330 compatibility

layout (triangles) in;
layout (triangle_strip, max_vertices = 67) out;

uniform vec3 playerPosition;

in DATA
{
    vec2 fragUV;
    flat float fragLayer;
    vec3 worldPos;
    flat int fragGpuLightIndex;
    flat int fragFace;
} geomIn[];

out DATA
{
    vec2 fragUV;
    flat float fragLayer;
    vec3 worldPos;
    flat int fragGpuLightIndex;
    flat int fragFace;
} geomOut;

#define FACE_TOP 1
#define FACE_BOTTOM 2
#define FACE_LEFT 3
#define FACE_RIGHT 4
#define FACE_FRONT 5
#define FACE_BACK 6
#define FACE_CROSS 7
#define FACE_SLOPE 8
#define FACE_GRASS 9

uniform float time;

float hash(vec2 p)
{   
    return fract(
        sin(dot(p, vec2(12.9898, 78.233))) *
        43758.5453
    );
}

void main()
{
    // original block triangle
    for (int i = 0; i < 3; i++)
    {
        gl_Position =
            gl_ModelViewProjectionMatrix *
            vec4(geomIn[i].worldPos, 1.0);

        geomOut.fragUV = geomIn[i].fragUV;
        geomOut.fragLayer = geomIn[i].fragLayer;
        geomOut.worldPos = geomIn[i].worldPos;
        geomOut.fragGpuLightIndex = geomIn[i].fragGpuLightIndex;
        geomOut.fragFace = geomIn[i].fragFace;

        EmitVertex();
    }

    EndPrimitive();

    if (!((geomIn[0].fragFace == FACE_TOP || geomIn[0].fragFace == FACE_SLOPE)&& geomIn[0].fragLayer == 1))
    {
        return;
    } else if (distance(playerPosition, geomIn[0].worldPos) > 20) {
        return;
    }
    
    float playerDistance =
    distance(playerPosition, geomIn[0].worldPos);

    
    vec3 p0 = geomIn[0].worldPos;
    vec3 p1 = geomIn[1].worldPos;
    vec3 p2 = geomIn[2].worldPos;

    


    vec3 edge1 = p1 - p0;
    vec3 edge2 = p2 - p0;

    vec3 faceNormal = vec3(0,1.0,0);
        // normalize(cross(edge1, edge2));

    vec3 up =
        abs(faceNormal.y) < 0.99
        ? vec3(0.0, 1.0, 0.0)
        : vec3(1.0, 0.0, 0.0);

    vec3 right =
        normalize(cross(faceNormal, up));

    // grass 
    for (int j = 0; j < 64; j++)
    {
        vec2 randomSeed =
        p0.xz +
        vec2(float(j) * 17.31, float(j) * 43.17);

        float r1 = hash(randomSeed);
        float r2 = hash(randomSeed + 13.37);

        float sqrtR1 = sqrt(r1);

        float a = 1.0 - sqrtR1;
        float b = sqrtR1 * (1.0 - r2);
        float c = sqrtR1 * r2;

        vec3 base =
            p0 * a +
            p1 * b +
            p2 * c;

        float h =
            mix(
                0.05,
                0.15,
                hash(randomSeed + 91.7)
            );

        

        // Random blade width
        float width =
            mix(
                0.015,
                0.035,
                hash(randomSeed + 37.2)
            );

        // Random orientation
        float angle =
            hash(randomSeed + 71.4) *
            6.2831853;

        vec3 bladeRight =
            normalize(
                right * cos(angle) +
                cross(faceNormal, right) * sin(angle)
            ) * width;

        // Random wind phase
        float windPhase =
            hash(randomSeed + 21.7) *
            6.2831853;

        // Wind movement
        float sway =
            sin(
                time * 2.0 +
                windPhase +
                base.x * 0.7 +
                base.z * 0.7
            ) * 0.05;

        vec3 tip =
            base +
            faceNormal * h * 2.0;

        // Only move the tip so the grass bends
        tip += right * sway;

        // Slightly raise the base to avoid z-fighting
        base += faceNormal * 0.005;

        vec3 verts[4] = vec3[](
            base - bladeRight * 3,
            base + bladeRight * 3,
            tip - bladeRight * 0.75,
            tip + bladeRight * 0.75
        );


        for (int i = 0; i < 4; i++)
        {
            gl_Position =
                gl_ModelViewProjectionMatrix *
                vec4(verts[i], 1.0);

            geomOut.fragUV = vec2(0.9,0.9);
            geomOut.fragLayer = FACE_GRASS;
            geomOut.worldPos = verts[i];
            geomOut.fragGpuLightIndex =
                geomIn[0].fragGpuLightIndex;
            geomOut.fragFace =
                FACE_GRASS;

            EmitVertex();
        }

        EndPrimitive();
    }
}