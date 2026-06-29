#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// VENTANA
static int WIN_W = 1000;
static int WIN_H = 700;


// VECTOR 2D
struct Vec2 {
    float x, y;
    Vec2(float x_=0, float y_=0): x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const { return {x+o.x, y+o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x-o.x, y-o.y}; }
    Vec2 operator*(float s)       const { return {x*s,   y*s  }; }
    Vec2 operator/(float s)       const { return {x/s,   y/s  }; }
    Vec2& operator+=(const Vec2& o){ x+=o.x; y+=o.y; return *this; }
    Vec2& operator-=(const Vec2& o){ x-=o.x; y-=o.y; return *this; }

    float length()  const { return sqrtf(x*x + y*y); }
    float length2() const { return x*x + y*y; }

    Vec2 normalized() const {
        float l = length();
        return l > 1e-6f ? (*this / l) : Vec2(0,0);
    }

    // Limita la magnitud del vector
    Vec2 clamped(float maxLen) const {
        float l = length();
        return l > maxLen ? (*this * (maxLen/l)) : *this;
    }

    // Devuelve el angulo en radianes respecto al eje +X
    float angle() const { return atan2f(y, x); }
};


// ESTRUCTURA BOID
struct Boid {
    Vec2  pos;      // Posicion actual
    Vec2  vel;      // Velocidad actual (magnitud * direccion)
    Vec2  dir;      // Direccion unitaria de movimiento (normalizada)
    Vec2  acc;      // Aceleracion acumulada del frame

    // Historial de posiciones para el rastro
    static const int TRAIL_LEN = 20;
    Vec2  trail[TRAIL_LEN];
    int   trailHead;

    // Color individual (fijo, para distinguir individuos)
    float r, g, b;

    Boid() : trailHead(0), r(1), g(1), b(1) {
        memset(trail, 0, sizeof(trail));
    }

    // Actualiza el rastro con la posicion actual
    void pushTrail() {
        trail[trailHead % TRAIL_LEN] = pos;
        trailHead++;
    }
};

// PARAMETROS DE LA SIMULACION
struct SimParams {
    int numBoids = 80;

    float neighborRadius = 60.0f;   // Radio de vecindad
    float separateDist = 25.0f;   // Distancia minima de separacion

    float wSeparate = 1.8f;      // Peso separacion
    float wAlign = 1.0f;      // Peso alineamiento
    float wCohesion = 1.0f;      // Peso cohesion

    float maxSpeed = 200.0f;    // Velocidad maxima (px/s)
    float minSpeed =  60.0f;    // Velocidad minima (px/s)
    float maxForce = 400.0f;    // Fuerza de steering maxima

    bool  toroidal = false;     // true = mundo toroidal, false = rebote
    bool  paused = false;
    bool  showTrail = false;
    bool  showDebug= false;
};

static SimParams params;
static std::vector<Boid> boids;

// INICIALIZACION DE BOIDS
static float frand(float lo, float hi) {
    return lo + (hi - lo) * (rand() / (float)RAND_MAX);
}

static void initBoid(Boid& b) {
    b.pos.x = frand(50, WIN_W - 50);
    b.pos.y = frand(50, WIN_H - 50);
    float angle = frand(0, 2.0f * M_PI);
    float speed = frand(params.minSpeed, params.maxSpeed);
    b.vel.x = cosf(angle) * speed;
    b.vel.y = sinf(angle) * speed;
    b.acc   = Vec2(0, 0);
    // Color vistoso aleatorio (tonos calidos/frios mezclados)
    b.r = frand(0.3f, 1.0f);
    b.g = frand(0.3f, 1.0f);
    b.b = frand(0.3f, 1.0f);
    // Inicializar rastro en la posicion actual
    for (int i = 0; i < Boid::TRAIL_LEN; i++) b.trail[i] = b.pos;
    b.trailHead = 0;
}

static void resetBoids() {
    boids.resize(params.numBoids);
    for (auto& b : boids) initBoid(b);
}

static void setBoidCount(int n) {
    n = std::max(5, std::min(500, n));
    int prev = (int)boids.size();
    params.numBoids = n;
    boids.resize(n);
    for (int i = prev; i < n; i++) initBoid(boids[i]);
}

// REGLAS DE STEERING - Distancia entre dos boids (con soporte toroidal)
static Vec2 delta(const Vec2& a, const Vec2& b) {
    Vec2 d = b - a;
    if (params.toroidal) {
        // Ajuste de toro: elegir el camino mas corto
        if (d.x >  WIN_W * 0.5f) d.x -= WIN_W;
        if (d.x < -WIN_W * 0.5f) d.x += WIN_W;
        if (d.y >  WIN_H * 0.5f) d.y -= WIN_H;
        if (d.y < -WIN_H * 0.5f) d.y += WIN_H;
    }
    return d;
}

// Calcula las tres fuerzas de steering para un boid dado
static void computeSteering(int idx) {
    Boid& self = boids[idx];

    Vec2 sepForce(0,0); // Separacion
    Vec2 aliForce(0,0); // Alineamiento
    Vec2 cohForce(0,0); // Cohesion

    int  sepCount = 0;
    int  nbrCount = 0;

    float R2 = params.neighborRadius * params.neighborRadius;
    float S2 = params.separateDist   * params.separateDist;

    for (int j = 0; j < (int)boids.size(); j++) {
        if (j == idx) continue;
        Vec2 d = delta(self.pos, boids[j].pos);
        float dist2 = d.length2();

        //Separacion: vecinos MUY cercanos
        if (dist2 < S2 && dist2 > 1e-6f) {
            // Fuerza inversamente proporcional a la distancia
            float dist = sqrtf(dist2);
            Vec2 repulse = (self.pos - boids[j].pos).normalized() * (params.separateDist / dist);
            sepForce += repulse;
            sepCount++;
        }

        //Alineamiento y Cohesion: vecinos en el radio
        if (dist2 < R2) {
            aliForce += boids[j].vel;          // acumular velocidades
            cohForce += boids[j].pos + delta(boids[j].pos, self.pos); // posicion absoluta vecino
            nbrCount++;
        }
    }

    //Aplicar separacion
    if (sepCount > 0) {
        Vec2 desired = sepForce.normalized() * params.maxSpeed;
        Vec2 steer   = (desired - self.vel).clamped(params.maxForce);
        self.acc += steer * params.wSeparate;
    }

    //Aplicar alineamiento
    if (nbrCount > 0) {
        Vec2 avgVel  = aliForce / (float)nbrCount;
        Vec2 desired = avgVel.normalized() * params.maxSpeed;
        Vec2 steer   = (desired - self.vel).clamped(params.maxForce);
        self.acc += steer * params.wAlign;
    }

    //Aplicar cohesion
    if (nbrCount > 0) {
        Vec2 center  = cohForce / (float)nbrCount;
        Vec2 toward  = delta(self.pos, center);
        Vec2 desired = toward.normalized() * params.maxSpeed;
        Vec2 steer   = (desired - self.vel).clamped(params.maxForce);
        self.acc += steer * params.wCohesion;
    }
}

// ACTUALIZACION DE POSICION Y VELOCIDAD
static void updateBoids(float dt) {
    // Fase 1: calcular aceleraciones (basadas en el estado anterior)
    for (int i = 0; i < (int)boids.size(); i++) {
        boids[i].acc = Vec2(0,0);
        computeSteering(i);
    }

    // Fase 2: integrar posicion y velocidad
    for (auto& b : boids) {
        b.pushTrail();

        // Integrar velocidad
        b.vel += b.acc * dt;

        // Limitar velocidad
        float spd = b.vel.length();
        if (spd > params.maxSpeed) b.vel = b.vel * (params.maxSpeed / spd);
        if (spd < params.minSpeed && spd > 1e-6f) b.vel = b.vel * (params.minSpeed / spd);

        // Integrar posicion
        b.pos += b.vel * dt;

        //Comportamiento en bordes
        if (params.toroidal) {
            // Mundo toroidal: salir por un borde y aparecer por el opuesto
            if (b.pos.x < 0)      b.pos.x += WIN_W;
            if (b.pos.x > WIN_W)  b.pos.x -= WIN_W;
            if (b.pos.y < 0)      b.pos.y += WIN_H;
            if (b.pos.y > WIN_H)  b.pos.y -= WIN_H;
        } else {
            // Rebote suave: invertir componente de velocidad y reposicionar
            float margin = 5.0f;
            if (b.pos.x < margin)       { b.pos.x = margin;       b.vel.x = fabsf(b.vel.x); }
            if (b.pos.x > WIN_W-margin) { b.pos.x = WIN_W-margin; b.vel.x = -fabsf(b.vel.x); }
            if (b.pos.y < margin)       { b.pos.y = margin;       b.vel.y = fabsf(b.vel.y); }
            if (b.pos.y > WIN_H-margin) { b.pos.y = WIN_H-margin; b.vel.y = -fabsf(b.vel.y); }
        }
    }
}

// DIBUJO DE UN BOID (triangulo orientado segun velocidad)
static void drawBoid(const Boid& b, bool debug) {
    float angle = b.vel.angle(); // angulo de movimiento en radianes
    float size  = 7.0f;

    // Radio de vecindad (debug)
    if (debug) {
        glColor4f(b.r, b.g, b.b, 0.06f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(b.pos.x, b.pos.y);
        int seg = 32;
        for (int i = 0; i <= seg; i++) {
            float a = 2.0f * M_PI * i / seg;
            glVertex2f(b.pos.x + params.neighborRadius * cosf(a),
                       b.pos.y + params.neighborRadius * sinf(a));
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    // Rastro de trayectoria
    if (params.showTrail) {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i < Boid::TRAIL_LEN; i++) {
            int idx = (b.trailHead + i) % Boid::TRAIL_LEN;
            float alpha = (float)i / Boid::TRAIL_LEN;
            glColor4f(b.r, b.g, b.b, alpha * 0.5f);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glVertex2f(b.trail[idx].x, b.trail[idx].y);
        }
        glEnd();
        glDisable(GL_BLEND);
    }

    // Triangulo apuntando en la direccion de movimiento
    // Vertices en espacio local: frente en +X, base en -X
    glPushMatrix();
    glTranslatef(b.pos.x, b.pos.y, 0.0f);
    glRotatef(angle * 180.0f / M_PI, 0, 0, 1);

    // Sombra suave
    glColor4f(0, 0, 0, 0.25f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLES);
    glVertex2f( size*1.8f + 1.5f, -1.5f);
    glVertex2f(-size*0.7f + 1.5f,  size*0.6f - 1.5f);
    glVertex2f(-size*0.7f + 1.5f, -size*0.6f - 1.5f);
    glEnd();
    glDisable(GL_BLEND);

    // Cuerpo del boid (degradado de punta a base)
    glBegin(GL_TRIANGLES);
    // Punta (frente, mas brillante)
    glColor3f(fminf(1.0f, b.r * 1.3f), fminf(1.0f, b.g * 1.3f), fminf(1.0f, b.b * 1.3f));
    glVertex2f( size * 1.8f, 0.0f);
    // Base izquierda y derecha (mas oscuro)
    glColor3f(b.r * 0.55f, b.g * 0.55f, b.b * 0.55f);
    glVertex2f(-size * 0.7f,  size * 0.6f);
    glVertex2f(-size * 0.7f, -size * 0.6f);
    glEnd();

    // Contorno
    glColor3f(b.r * 0.3f, b.g * 0.3f, b.b * 0.3f);
    glBegin(GL_LINE_LOOP);
    glVertex2f( size * 1.8f, 0.0f);
    glVertex2f(-size * 0.7f,  size * 0.6f);
    glVertex2f(-size * 0.7f, -size * 0.6f);
    glEnd();

    glPopMatrix();
}

// DIBUJO DE LA CUADRICULA DE FONDO
static void drawGrid() {
    glColor3f(0.08f, 0.10f, 0.13f);
    int step = 50;
    glBegin(GL_LINES);
    for (int x = 0; x < WIN_W; x += step) {
        glVertex2f((float)x, 0); glVertex2f((float)x, (float)WIN_H);
    }
    for (int y = 0; y < WIN_H; y += step) {
        glVertex2f(0, (float)y); glVertex2f((float)WIN_W, (float)y);
    }
    glEnd();
}

// HUD
static void txt(float x, float y, const char* s, void* font=GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (const char* c = s; *c; c++) glutBitmapCharacter(font, *c);
}

static void drawHUD() {
    // Panel izquierdo semitransparente
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.0f, 0.02f, 0.07f, 0.82f);
    glBegin(GL_QUADS);
    glVertex2f(0,0); glVertex2f(230,0); glVertex2f(230,WIN_H); glVertex2f(0,WIN_H);
    glEnd();
    glDisable(GL_BLEND);

    float y = WIN_H - 18;
    auto line = [&](const char* s, float r=1, float g=1, float b=1) {
        glColor3f(r,g,b); txt(8, y, s); y -= 16;
    };
    auto lineF = [&](const char* label, float val, const char* unit="") {
        char buf[64]; sprintf(buf, "%s %.2f %s", label, val, unit);
        glColor3f(1,1,1); txt(8,y,buf); y-=16;
    };
    auto lineI = [&](const char* label, int val) {
        char buf[64]; sprintf(buf, "%s %d", label, val);
        glColor3f(1,1,1); txt(8,y,buf); y-=16;
    };

    glColor3f(1,1,0.4f); txt(8,y,"BOIDS - Lab10", GLUT_BITMAP_HELVETICA_18); y-=24;

    line("--- ESTADO ---", 0.7f,0.9f,1.0f);
    lineI("Boids:         ", (int)boids.size());
    lineF("Radio vecindad:", params.neighborRadius, "px");
    lineF("Radio sep:     ", params.separateDist,   "px");
    y -= 4;
    line("--- PESOS ---", 0.7f,0.9f,1.0f);
    lineF("Separacion [S/s]:", params.wSeparate);
    lineF("Alineamiento[A/a]:", params.wAlign);
    lineF("Cohesion   [C/c]:", params.wCohesion);
    y -= 4;
    line("--- VELOCIDAD ---", 0.7f,0.9f,1.0f);
    lineF("Min:", params.minSpeed, "px/s");
    lineF("Max:", params.maxSpeed, "px/s");
    y -= 4;
    line("--- CONTROLES ---", 0.7f,0.9f,1.0f);
    line("+/- : boids +/-10");
    line("R/r : radio vecindad");
    line("S/s : sep +/-");
    line("A/a : align +/-");
    line("C/c : cohesion +/-");
    line("B   : bordes (rebote/toro)");
    line("T   : rastro on/off");
    line("D   : debug on/off");
    line("Space: pausar");
    line("1-5 : presets");
    line("ESC : salir");
    y -= 4;
    line("--- MODO ---", 0.7f,0.9f,1.0f);
    glColor3f(params.toroidal   ? 0.3f:0.6f, params.toroidal   ? 1.0f:0.6f, 0.3f);
    txt(8,y, params.toroidal ? "Bordes: TOROIDAL" : "Bordes: REBOTE"); y-=16;
    glColor3f(params.paused     ? 1.0f:0.6f, params.paused     ? 0.4f:0.6f, 0.3f);
    txt(8,y, params.paused ? ">>> PAUSADO <<<" : "Simulando..."); y-=16;

    // Info FPS y conteo en la esquina superior derecha
    static int   frames=0, fps=0;
    static float lastTime=0;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    frames++;
    if (now - lastTime >= 1.0f) { fps=frames; frames=0; lastTime=now; }
    char fpsBuf[64];
    sprintf(fpsBuf, "FPS: %d", fps);
    glColor3f(0.6f,0.6f,0.6f);
    txt(WIN_W - 75, WIN_H - 16, fpsBuf);
}

// PRESETS DE COMPORTAMIENTO
static void applyPreset(int n) {
    switch(n) {
    case 1: // Bandada clasica
        params.neighborRadius=70; params.separateDist=25;
        params.wSeparate=1.8f; params.wAlign=1.0f; params.wCohesion=1.0f;
        params.maxSpeed=200; params.minSpeed=60;
        break;
    case 2: // Enjambre compacto
        params.neighborRadius=50; params.separateDist=18;
        params.wSeparate=1.2f; params.wAlign=0.5f; params.wCohesion=2.5f;
        params.maxSpeed=180; params.minSpeed=50;
        break;
    case 3: // Cardumen muy alineado
        params.neighborRadius=90; params.separateDist=20;
        params.wSeparate=1.5f; params.wAlign=3.0f; params.wCohesion=0.5f;
        params.maxSpeed=220; params.minSpeed=80;
        break;
    case 4: // Chaos (baja cohesion/alineamiento)
        params.neighborRadius=40; params.separateDist=30;
        params.wSeparate=3.0f; params.wAlign=0.2f; params.wCohesion=0.2f;
        params.maxSpeed=250; params.minSpeed=40;
        break;
    case 5: // Mundo toroidal rapido
        params.neighborRadius=80; params.separateDist=22;
        params.wSeparate=1.6f; params.wAlign=1.5f; params.wCohesion=1.2f;
        params.maxSpeed=280; params.minSpeed=100;
        params.toroidal = true;
        break;
    }
    printf("Preset %d aplicado\n", n);
}

// CALLBACKS GLUT
static float lastTime = 0.0f;

void display() {
    glClearColor(0.04f, 0.06f, 0.10f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIN_W, 0, WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawGrid();

    // Borde del mundo (toroidal = punteado, rebote = solido)
    if (params.toroidal) {
        glColor3f(0.2f,0.4f,0.7f);
        glEnable(GL_LINE_STIPPLE);
        glLineStipple(2, 0xAAAA);
    } else {
        glColor3f(0.3f,0.5f,0.8f);
    }
    glBegin(GL_LINE_LOOP);
    glVertex2f(1,1); glVertex2f(WIN_W-1,1);
    glVertex2f(WIN_W-1,WIN_H-1); glVertex2f(1,WIN_H-1);
    glEnd();
    if (params.toroidal) glDisable(GL_LINE_STIPPLE);

    // Dibujar boids (debug solo para el primer boid)
    for (int i = 0; i < (int)boids.size(); i++)
        drawBoid(boids[i], params.showDebug && i == 0);

    drawHUD();

    glutSwapBuffers();
}

void reshape(int w, int h) {
    WIN_W = w; WIN_H = h;
    glViewport(0, 0, w, h);
    glutPostRedisplay();
}

void timer(int) {
    if (!params.paused) {
        float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        float dt  = now - lastTime;
        // Limitar dt para evitar saltos en frames lentos
        if (dt > 0.05f) dt = 0.05f;
        lastTime = now;
        updateBoids(dt);
    }
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 fps
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
    // Numero de boids
    case '+': case '=': setBoidCount(params.numBoids + 10); break;
    case '-': case '_': setBoidCount(params.numBoids - 10); break;

    // Radio de vecindad
    case 'r': params.neighborRadius = fmaxf(20, params.neighborRadius - 5); break;
    case 'R': params.neighborRadius = fminf(300,params.neighborRadius + 5); break;

    // Pesos
    case 's': params.wSeparate   = fmaxf(0, params.wSeparate   - 0.1f); break;
    case 'S': params.wSeparate   = fminf(5, params.wSeparate   + 0.1f); break;
    case 'a': params.wAlign      = fmaxf(0, params.wAlign      - 0.1f); break;
    case 'A': params.wAlign      = fminf(5, params.wAlign      + 0.1f); break;
    case 'c': params.wCohesion   = fmaxf(0, params.wCohesion   - 0.1f); break;
    case 'C': params.wCohesion   = fminf(5, params.wCohesion   + 0.1f); break;

    // Modos
    case 'b': case 'B': params.toroidal   = !params.toroidal;   break;
    case ' ':           params.paused     = !params.paused;     break;
    case 't': case 'T': params.showTrail  = !params.showTrail;  break;
    case 'd': case 'D': params.showDebug  = !params.showDebug;  break;

    // Presets
    case '1': applyPreset(1); break;
    case '2': applyPreset(2); break;
    case '3': applyPreset(3); break;
    case '4': applyPreset(4); break;
    case '5': applyPreset(5); break;

    case 27: exit(0);
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WIN_W, WIN_H);
    glutCreateWindow("Lab10 - Comportamiento Colectivo (Boids) - Craig Reynolds 1986");

    resetBoids();
    lastTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(16, timer, 0);
    glutMainLoop();
    return 0;
}