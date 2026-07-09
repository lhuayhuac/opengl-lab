#include <GL/freeglut.h>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>

#include "include/Vec3.h"
#include "include/Camera.h"
#include "include/RainSystem.h"

static Camera     camera;
static RainSystem rain;

// Ventana
static int winW = 1280, winH = 720;

// Tiempo
static float lastTime   = 0.0f;
static float deltaTime  = 0.0f;
static float fps        = 0.0f;
static float fpsTimer   = 0.0f;
static int   fpsFrames  = 0;

// Estado de teclas (para movimiento suave)
static bool keys[256] = {};

// Mouse capturado
static bool mouseCaptured = true;

static void drawGround() {
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    // Suelo verde oscuro (pasto mojado)
    glColor3f(0.18f, 0.28f, 0.14f);
    glBegin(GL_QUADS);
        glVertex3f(-60, 0, -60);
        glVertex3f( 60, 0, -60);
        glVertex3f( 60, 0,  60);
        glVertex3f(-60, 0,  60);
    glEnd();

    // Líneas de cuadrícula para sensación de profundidad
    glColor3f(0.14f, 0.22f, 0.10f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -60; i <= 60; i += 5) {
        glVertex3f((float)i, 0.01f, -60);
        glVertex3f((float)i, 0.01f,  60);
        glVertex3f(-60, 0.01f, (float)i);
        glVertex3f( 60, 0.01f, (float)i);
    }
    glEnd();
}

static void drawCube(float x, float y, float z,
                     float sx, float sy, float sz,
                     float r, float g, float b)
{
    glPushMatrix();
    glTranslatef(x, y + sy*0.5f, z);
    glScalef(sx, sy, sz);
    glColor3f(r, g, b);

    // 6 caras del cubo unitario centrado en origen
    glBegin(GL_QUADS);
        // +Y (techo)
        glVertex3f(-0.5f, 0.5f,-0.5f); glVertex3f( 0.5f, 0.5f,-0.5f);
        glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
        // -Y (suelo del cubo)
        glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,-0.5f, 0.5f);
        glVertex3f( 0.5f,-0.5f, 0.5f); glVertex3f( 0.5f,-0.5f,-0.5f);
        // +Z (frente)
        glVertex3f(-0.5f,-0.5f, 0.5f); glVertex3f( 0.5f,-0.5f, 0.5f);
        glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f, 0.5f);
        // -Z (fondo)
        glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f, 0.5f,-0.5f);
        glVertex3f( 0.5f, 0.5f,-0.5f); glVertex3f( 0.5f,-0.5f,-0.5f);
        // +X (derecha)
        glVertex3f( 0.5f,-0.5f,-0.5f); glVertex3f( 0.5f, 0.5f,-0.5f);
        glVertex3f( 0.5f, 0.5f, 0.5f); glVertex3f( 0.5f,-0.5f, 0.5f);
        // -X (izquierda)
        glVertex3f(-0.5f,-0.5f,-0.5f); glVertex3f(-0.5f,-0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f); glVertex3f(-0.5f, 0.5f,-0.5f);
    glEnd();
    glPopMatrix();
}

static void drawTree(float x, float z) {
    // Tronco: cubo marrón alargado
    drawCube(x, 0, z, 0.4f, 3.0f, 0.4f, 0.35f, 0.22f, 0.10f);
    // Copa: cubo verde
    drawCube(x, 3.0f, z, 1.8f, 2.0f, 1.8f, 0.15f, 0.38f, 0.12f);
    // Copa alta: más pequeña
    drawCube(x, 4.5f, z, 1.1f, 1.5f, 1.1f, 0.12f, 0.32f, 0.10f);
}

static void drawBuilding(float x, float z,
                         float w, float h, float d,
                         float r, float g, float b) {
    drawCube(x, 0, z, w, h, d, r, g, b);
    // Borde oscuro encima (techo)
    drawCube(x, h, z, w+0.2f, 0.3f, d+0.2f,
             r*0.7f, g*0.7f, b*0.7f);
}

static void drawScene() {
    drawGround();

    // Edificios
    drawBuilding(-15, -10,  4, 8, 3, 0.50f, 0.48f, 0.44f);
    drawBuilding( 12, -8,   5, 6, 4, 0.44f, 0.42f, 0.40f);
    drawBuilding( -8,  15,  3, 10, 3, 0.46f, 0.44f, 0.42f);
    drawBuilding(  5,  12,  6, 5, 5, 0.48f, 0.46f, 0.43f);

    // Árboles
    drawTree(-5,  -5);
    drawTree( 8,  -3);
    drawTree(-10,  8);
    drawTree( 3,   7);
    drawTree(-18,  2);
    drawTree( 15,  5);

    // Columnas/postes de luz
    drawCube(-6, 0, 6, 0.2f, 5.0f, 0.2f, 0.3f, 0.3f, 0.3f);
    drawCube( 6, 0, 6, 0.2f, 5.0f, 0.2f, 0.3f, 0.3f, 0.3f);
    drawCube(-6, 0,-6, 0.2f, 5.0f, 0.2f, 0.3f, 0.3f, 0.3f);
    drawCube( 6, 0,-6, 0.2f, 5.0f, 0.2f, 0.3f, 0.3f, 0.3f);
}

static void drawText(float x, float y, const char* text,
                     float r=1,float g=1,float b=1) {
    glColor3f(r, g, b);
    glRasterPos2f(x, y);
    for (const char* c = text; *c; c++)
        glutBitmapCharacter(GLUT_BITMAP_8_BY_13, *c);
}

static void drawHUD() {
    // Cambiar a proyección ortográfica 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Desactivar depth test para que el HUD siempre se vea
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    char buf[128];
    float lh = 16.0f; // line height
    float x0 = 10.0f;
    float y0 = winH - 20.0f;

    // Fondo semi-transparente del panel
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.0f, 0.0f, 0.55f);
    glBegin(GL_QUADS);
        glVertex2f(5, winH-5);
        glVertex2f(310, winH-5);
        glVertex2f(310, winH - lh*10 - 10);
        glVertex2f(5,   winH - lh*10 - 10);
    glEnd();
    glDisable(GL_BLEND);

    // ── Líneas del HUD ──────────────────────────────────────
    snprintf(buf, sizeof(buf), "FPS: %.1f  (%.2f ms)", fps, 1000.0f/fmaxf(fps,1));
    drawText(x0, y0, buf, 0.2f, 1.0f, 0.2f);

    snprintf(buf, sizeof(buf), "Particulas activas: %d / %d",
             rain.activeDrops, rain.maxDrops);
    drawText(x0, y0-lh, buf, 1.0f, 1.0f, 0.2f);

    snprintf(buf, sizeof(buf), "Camara pos: (%.1f, %.1f, %.1f)",
             camera.position.x, camera.position.y, camera.position.z);
    drawText(x0, y0-lh*2, buf, 0.8f, 0.9f, 1.0f);

    snprintf(buf, sizeof(buf), "Camara dir: (%.2f, %.2f, %.2f)",
             camera.front.x, camera.front.y, camera.front.z);
    drawText(x0, y0-lh*3, buf, 0.8f, 0.9f, 1.0f);

    snprintf(buf, sizeof(buf), "Lluvia: %s",
             rain.paused ? "PAUSADA" : "ACTIVA");
    drawText(x0, y0-lh*4, buf,
             rain.paused ? 1.0f : 0.3f,
             rain.paused ? 0.3f : 1.0f,
             0.3f);

    snprintf(buf, sizeof(buf), "Billboards: %s | Textura: %s",
             rain.billboardsEnabled ? "ON" : "OFF",
             rain.textureEnabled    ? "ON" : "OFF");
    drawText(x0, y0-lh*5, buf, 1.0f, 0.8f, 0.5f);

    snprintf(buf, sizeof(buf), "Alpha Blending: ACTIVO (SRC_ALPHA)");
    drawText(x0, y0-lh*6, buf, 0.7f, 0.7f, 1.0f);

    // Controles
    drawText(x0, y0-lh*7,
             "WASD=mover  QE=subir/bajar  Mouse=rotar",
             0.7f, 0.7f, 0.7f);
    drawText(x0, y0-lh*8,
             "R=reset  +/-=particulas  B=billboard  T=tex  P=pausa",
             0.7f, 0.7f, 0.7f);
    drawText(x0, y0-lh*9,
             "ESC=salir  [click izq = capturar/soltar mouse]",
             0.7f, 0.7f, 0.7f);

    // Restaurar
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

static void display() {
    // Delta time
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime  = now - lastTime;
    lastTime   = now;
    if (deltaTime > 0.1f) deltaTime = 0.1f; // clamp

    // FPS counter
    fpsTimer  += deltaTime;
    fpsFrames++;
    if (fpsTimer >= 0.5f) {
        fps       = fpsFrames / fpsTimer;
        fpsTimer  = 0;
        fpsFrames = 0;
    }

    // ── Movimiento suave desde teclas retenidas ──────────────
    if (keys['w'] || keys['W']) camera.moveForward (deltaTime);
    if (keys['s'] || keys['S']) camera.moveBackward(deltaTime);
    if (keys['a'] || keys['A']) camera.moveLeft    (deltaTime);
    if (keys['d'] || keys['D']) camera.moveRight   (deltaTime);
    if (keys['q'] || keys['Q']) camera.moveUp      (deltaTime);
    if (keys['e'] || keys['E']) camera.moveDown    (deltaTime);

    // ── Actualizar sistema de lluvia ─────────────────────────
    rain.update(deltaTime, camera);

    // ── Render ───────────────────────────────────────────────
    // Cielo nublado (gris azulado oscuro)
    glClearColor(0.30f, 0.32f, 0.36f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Matrices
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (double)winW / winH, 0.1, 250.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyLookAt();

    glEnable(GL_DEPTH_TEST);

    // 1. Escena opaca (sin blending)
    drawScene();

    // 2. Lluvia (con blending, después de opacos)
    rain.render(camera);

    // 3. HUD 2D
    drawHUD();

    glutSwapBuffers();
    glutPostRedisplay(); // loop continuo
}

static void reshape(int w, int h) {
    winW = w;
    winH = h > 0 ? h : 1;
    glViewport(0, 0, winW, winH);
}

static void keyDown(unsigned char key, int /*x*/, int /*y*/) {
    keys[key] = true;

    switch (key) {
    case 27: // ESC
        rain.cleanup();
        exit(0);
        break;

    case 'r': case 'R':
        rain.resetAll(camera.position);
        break;

    case '+': case '=':
        rain.increaseDrops(100, camera.position);
        break;

    case '-': case '_':
        rain.decreaseDrops(100);
        break;

    case 'b': case 'B':
        rain.billboardsEnabled = !rain.billboardsEnabled;
        break;

    case 't': case 'T':
        rain.textureEnabled = !rain.textureEnabled;
        break;

    case 'p': case 'P':
        rain.paused = !rain.paused;
        break;
    }
}

static void keyUp(unsigned char key, int /*x*/, int /*y*/) {
    keys[key] = false;
}

static void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // Toggle captura de mouse
        mouseCaptured = !mouseCaptured;
        if (mouseCaptured) {
            glutSetCursor(GLUT_CURSOR_NONE);
            camera.firstMouse = true;
        } else {
            glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
        }
    }
}

static void mouseMotion(int x, int y) {
    if (!mouseCaptured) return;
    camera.processMouse(x, y);
}

static void mousePassive(int x, int y) {
    if (!mouseCaptured) return;
    camera.processMouse(x, y);

    // Recentrar cursor para movimiento ilimitado
    int cx = winW / 2, cy = winH / 2;
    if (x != cx || y != cy) {
        glutWarpPointer(cx, cy);
        camera.lastMouseX = cx;
        camera.lastMouseY = cy;
    }
}

int main(int argc, char** argv) {
    srand((unsigned int)time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Rain System - FreeGLUT OpenGL Clasico");

    // Callbacks
    glutDisplayFunc   (display);
    glutReshapeFunc   (reshape);
    glutKeyboardFunc  (keyDown);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc     (mouseButton);
    glutMotionFunc    (mouseMotion);
    glutPassiveMotionFunc(mousePassive);

    // Estado inicial de OpenGL
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);

    // Capturar mouse al inicio
    glutSetCursor(GLUT_CURSOR_NONE);

    // Inicializar sistema de lluvia
    // 1200 gotas iniciales dentro del volumen 40x40 sobre la cámara
    rain.initialize(1200, camera.position);

    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glutMainLoop();
    return 0;
}