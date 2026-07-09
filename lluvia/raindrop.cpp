#include "include/RainDrop.h"
#include <cstdlib>
#include <cmath>

// Helpers de aleatoriedad
static float randF(float lo, float hi) {
    return lo + (hi - lo) * ((float)rand() / (float)RAND_MAX);
}

// ─────────────────────────────────────────────────────────────
RainDrop::RainDrop()
    : position(0,0,0)
    , velocity(0,-8,0)
    , acceleration(0,-9.8f,0)
    , size(0.07f)
    , active(true)
{}

void RainDrop::update(float dt) {
    if (!active) return;
    velocity += acceleration * dt;
    position += velocity     * dt;
}

void RainDrop::reset(const Vec3& origin,
                     float volumeW, float volumeD, float spawnH)
{
    // Posición aleatoria dentro del volumen XZ + altura de spawn
    position.x = origin.x + randF(-volumeW * 0.5f, volumeW * 0.5f);
    position.y = spawnH + randF(0.0f, 5.0f); // variación en altura
    position.z = origin.z + randF(-volumeD * 0.5f, volumeD * 0.5f);

    // Velocidad: componente Y (caída) + pequeño viento lateral
    float fallSpeed = randF(-12.0f, -7.0f);
    velocity.x = randF(-0.3f, 0.3f); // viento X
    velocity.y = fallSpeed;
    velocity.z = randF(-0.3f, 0.3f); // viento Z

    // Aceleración = gravedad pura
    acceleration = Vec3(0.0f, -9.8f, 0.0f);

    // Tamaño aleatorio de la gota
    size = randF(0.04f, 0.10f);

    active = true;
}