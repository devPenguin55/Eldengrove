#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "vectors.h"

typedef struct Particle {
    Vec3 position;
    Vec3 velocity;
    int color;
} Particle;

typedef struct WorldParticles {
    Particle *particles;
    int amtParticles;
    int capacity;
} WorldParticles;

#endif