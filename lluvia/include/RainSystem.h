#pragma once
#include "RainDrop.h"
#include "Camera.h"
#include <vector>

class RainSystem {
public:
    // ── Parámetros del volumen de generación ────────────────
    float volumeWidth;      // ancho  del volumen XZ
    float volumeDepth;      // profundidad del volumen XZ
    float spawnHeight;      // altura desde donde caen las gotas
    float groundLevel;      // Y del suelo (muerte de partícula)

    // ── Pool de partículas ───────────────────────────────────
    std::vector<RainDrop> drops;
    int maxDrops;           // capacidad máxima del pool
    int activeDrops;        // cuántas están activas este frame

    // ── Estado ──────────────────────────────────────────────
    bool paused;
    bool billboardsEnabled;
    bool textureEnabled;

    // ── Textura ─────────────────────────────────────────────
    unsigned int textureID; // GL texture handle

    // ── Constructor ─────────────────────────────────────────
    RainSystem();

    // Crear el pool, generar textura procedural
    // count = número inicial de gotas
    void initialize(int count, const Vec3& origin);

    // Actualizar física de todas las gotas
    void update(float dt, const Camera& cam);

    // Renderizar todas las gotas activas
    void render(const Camera& cam) const;

    // Reiniciar todas las gotas
    void resetAll(const Vec3& origin);

    // Aumentar / disminuir cantidad de partículas activas
    void increaseDrops(int amount, const Vec3& origin);
    void decreaseDrops(int amount);

    // Crear textura de gota proceduralmente
    void createTexture();

    // Liberar textura
    void cleanup();
};