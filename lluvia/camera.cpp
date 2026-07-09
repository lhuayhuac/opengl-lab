#include "include/Camera.h"
#include <GL/freeglut.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Constantes
static const float PITCH_LIMIT = 89.0f;

// ─────────────────────────────────────────────────────────────
Camera::Camera()
    : position(0.0f, 4.0f, 18.0f)
    , front(0.0f, -0.1f, -1.0f)
    , right(1.0f, 0.0f, 0.0f)
    , up(0.0f, 1.0f, 0.0f)
    , worldUp(0.0f, 1.0f, 0.0f)
    , yaw(-90.0f)
    , pitch(-5.0f)
    , moveSpeed(8.0f)
    , mouseSensitivity(0.15f)
    , lastMouseX(0), lastMouseY(0)
    , firstMouse(true)
{
    updateVectors();
}

void Camera::updateVectors() {
    float yawR   = yaw   * (float)M_PI / 180.0f;
    float pitchR = pitch * (float)M_PI / 180.0f;

    front.x = cosf(yawR) * cosf(pitchR);
    front.y = sinf(pitchR);
    front.z = sinf(yawR) * cosf(pitchR);
    front   = front.normalized();

    right   = front.cross(worldUp).normalized();
    up      = right.cross(front).normalized();
}

void Camera::applyLookAt() const {
    Vec3 target = position + front;
    gluLookAt(
        position.x, position.y, position.z,
        target.x,   target.y,   target.z,
        up.x,       up.y,       up.z
    );
}

void Camera::moveForward (float dt) { position += front * (moveSpeed * dt); }
void Camera::moveBackward(float dt) { position -= front * (moveSpeed * dt); }
void Camera::moveLeft    (float dt) { position -= right * (moveSpeed * dt); }
void Camera::moveRight   (float dt) { position += right * (moveSpeed * dt); }
void Camera::moveUp      (float dt) { position += worldUp * (moveSpeed * dt); }
void Camera::moveDown    (float dt) { position -= worldUp * (moveSpeed * dt); }

void Camera::processMouse(int x, int y) {
    if (firstMouse) {
        lastMouseX = x;
        lastMouseY = y;
        firstMouse = false;
        return;
    }

    float dx = (float)(x - lastMouseX) * mouseSensitivity;
    float dy = (float)(lastMouseY - y) * mouseSensitivity; // invertido Y
    lastMouseX = x;
    lastMouseY = y;

    yaw   += dx;
    pitch += dy;

    // Limitar pitch para evitar gimbal lock
    if (pitch >  PITCH_LIMIT) pitch =  PITCH_LIMIT;
    if (pitch < -PITCH_LIMIT) pitch = -PITCH_LIMIT;

    updateVectors();
}

void Camera::reset() {
    position  = Vec3(0.0f, 4.0f, 18.0f);
    yaw       = -90.0f;
    pitch     = -5.0f;
    firstMouse = true;
    updateVectors();
}