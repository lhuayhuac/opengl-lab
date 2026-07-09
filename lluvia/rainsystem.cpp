#include "include/RainSystem.h"
#include "include/Billboard.h"
#include <GL/freeglut.h>
#include <cstdlib>
#include <cstring>
#include <algorithm>

RainSystem::RainSystem()
    : volumeWidth(40.0f)
    , volumeDepth(40.0f)
    , spawnHeight(20.0f)
    , groundLevel(0.0f)
    , maxDrops(0)
    , activeDrops(0)
    , paused(false)
    , billboardsEnabled(true)
    , textureEnabled(true)
    , textureID(0)
{}

void RainSystem::initialize(int count, const Vec3& origin) {
    maxDrops    = count;
    activeDrops = count;
    drops.resize(count);

    for (auto& d : drops)
        d.reset(origin, volumeWidth, volumeDepth, spawnHeight);

    // Distribuir en alturas aleatorias para no verlas aparecer todas juntas
    for (auto& d : drops)
        d.position.y = (float)(rand() % (int)spawnHeight);

    createTexture();
}

void RainSystem::update(float dt, const Camera& cam) {
    if (paused) return;

    activeDrops = 0;
    for (int i = 0; i < maxDrops; i++) {
        RainDrop& d = drops[i];
        if (!d.active) continue;

        d.update(dt);

        // ¿Tocó el suelo? → reciclar
        if (d.position.y < groundLevel) {
            d.reset(cam.position, volumeWidth, volumeDepth, spawnHeight);
        }

        // ¿Salió del volumen horizontal? → reciclar
        // (cuando la cámara se mueve rápido algunas gotas quedan atrás)
        float dx = d.position.x - cam.position.x;
        float dz = d.position.z - cam.position.z;
        if (fabsf(dx) > volumeWidth || fabsf(dz) > volumeDepth)
            d.reset(cam.position, volumeWidth, volumeDepth, spawnHeight);

        activeDrops++;
    }
}

void RainSystem::render(const Camera& cam) const {
    if (!billboardsEnabled) return;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // No escribir en depth buffer (sí leer)
    glDepthMask(GL_FALSE);

    // Textura
    if (textureEnabled && textureID != 0) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, textureID);
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    bool useTex = textureEnabled && textureID != 0;

    for (int i = 0; i < maxDrops; i++) {
        const RainDrop& d = drops[i];
        if (!d.active) continue;

        // Alpha fijo semi-transparente
        // (se puede hacer fade-in/out según posición Y)
        float alpha = 0.55f;

        Billboard::render(
            d.position,
            d.size,          // ancho de la gota
            d.size * 3.5f,   // alto de la gota (más alargada)
            cam.right,
            cam.up,
            alpha,
            useTex,
            0.72f, 0.88f, 1.0f  // color azul-lluvia
        );
    }

    // ── Restaurar estado ─────────────────────────────────────
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
}

void RainSystem::resetAll(const Vec3& origin) {
    for (auto& d : drops)
        d.reset(origin, volumeWidth, volumeDepth, spawnHeight);
}

void RainSystem::increaseDrops(int amount, const Vec3& origin) {
    int newMax = maxDrops + amount;
    if (newMax > 5000) newMax = 5000;

    // Expandir el vector si es necesario
    if (newMax > (int)drops.size()) {
        drops.resize(newMax);
        for (int i = maxDrops; i < newMax; i++)
            drops[i].reset(origin, volumeWidth, volumeDepth, spawnHeight);
    } else {
        // Reactivar gotas que estaban desactivadas
        for (int i = maxDrops; i < newMax; i++) {
            drops[i].active = true;
            drops[i].reset(origin, volumeWidth, volumeDepth, spawnHeight);
        }
    }
    maxDrops = newMax;
}

void RainSystem::decreaseDrops(int amount) {
    int newMax = maxDrops - amount;
    if (newMax < 50) newMax = 50;
    // Desactivar las últimas 'amount' gotas
    for (int i = newMax; i < maxDrops; i++)
        drops[i].active = false;
    maxDrops = newMax;
}

void RainSystem::createTexture() {
    const int W = 4, H = 16;
    unsigned char data[W * H * 4];

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            int idx = (y * W + x) * 4;
            // Posición centrada en X: -0.5..+0.5
            float cx   = (float)x / W - 0.5f;
            // Fade vertical: máximo en centro, cero en extremos
            float fadeY = sinf((float)y / H * 3.14159f);
            // Máscara de ancho: cae rápido desde el centro
            float mask  = fmaxf(0.0f, 1.0f - fabsf(cx) * 6.0f);
            // Alpha resultante
            unsigned char a = (unsigned char)(mask * fadeY * 210.0f);
            data[idx+0] = 200; // R
            data[idx+1] = 220; // G
            data[idx+2] = 255; // B
            data[idx+3] = a;   // A
        }
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // GL_CLAMP disponible en todos los drivers OpenGL 1.1+
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void RainSystem::cleanup() {
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}