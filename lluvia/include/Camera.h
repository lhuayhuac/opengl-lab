#pragma once
#include "Vec3.h"

class Camera {
public:
    // ── Posición y orientación ──────────────────────────────
    Vec3  position;     // posición en world-space
    Vec3  front;        // dirección de visión (normalizado)
    Vec3  right;        // eje derecho (normalizado) ← usado por Billboard
    Vec3  up;           // eje arriba real de la cámara (normalizado)
    Vec3  worldUp;      // up global (0,1,0)

    // ── Ángulos de Euler ────────────────────────────────────
    float yaw;          // rotación horizontal (grados)
    float pitch;        // rotación vertical   (grados)

    // ── Parámetros de movimiento ────────────────────────────
    float moveSpeed;    // unidades / segundo
    float mouseSensitivity;

    // ── Mouse ───────────────────────────────────────────────
    int   lastMouseX, lastMouseY;
    bool  firstMouse;

    // ── Constructor ─────────────────────────────────────────
    Camera();

    // Aplica gluLookAt con los vectores actuales
    void applyLookAt() const;

    // Actualiza front, right, up desde yaw y pitch
    void updateVectors();

    // Movimiento con teclado (llamar desde keyboardCallback)
    void moveForward (float dt);
    void moveBackward(float dt);
    void moveLeft    (float dt);
    void moveRight   (float dt);
    void moveUp      (float dt);
    void moveDown    (float dt);

    // Rotación con mouse (llamar desde motionCallback)
    void processMouse(int x, int y);

    // Reiniciar posición
    void reset();
};