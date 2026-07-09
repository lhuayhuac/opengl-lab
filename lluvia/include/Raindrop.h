#pragma once
#include "Vec3.h"

struct RainDrop {
    Vec3  position;     // posición actual world-space
    Vec3  velocity;     // velocidad actual (m/s)
    Vec3  acceleration; // aceleración (principalmente gravedad)
    float size;         // tamaño del billboard (world units)
    bool  active;       // true = viva, false = muerta (pool)

    RainDrop();

    // Actualizar física (integración de Euler)
    // deltaTime en segundos
    void update(float deltaTime);

    // Reiniciar la gota con nueva posición aleatoria
    // dentro del volumen centrado en 'origin'
    void reset(const Vec3& origin,
               float volumeW, float volumeD, float spawnH);
};