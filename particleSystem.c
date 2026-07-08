#include "particleSystem.h"
#include <stdio.h>
#include <stdlib.h>

WorldParticles worldParticles;


void initParticleSystem() {
    worldParticles.amtParticles = 0;
    worldParticles.capacity = 16;
    worldParticles.particles = malloc(sizeof(Particle) * worldParticles.capacity);
}

void createParticle(Vec3 position, Vec3 velocity) {
    if (worldParticles.amtParticles >= worldParticles.capacity) {
        worldParticles.capacity *= 2;
        worldParticles.particles = realloc(worldParticles.particles, sizeof(Particle) * worldParticles.capacity);
    }

    worldParticles.particles[worldParticles.amtParticles].position = position;
    worldParticles.particles[worldParticles.amtParticles].velocity = velocity;
    worldParticles.particles[worldParticles.amtParticles++].color  =        5;
}

void worldParticleSimulationStep() {
    for (int i = 0; i < worldParticles.amtParticles; i++) {
        Particle *curParticle = &worldParticles.particles[i];

        curParticle->position = vec3Add(curParticle->position, curParticle->velocity);
    }
}