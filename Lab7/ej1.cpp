#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//ESTRUCTURAS BASE
enum ObjectType { OBJ_CUBE, OBJ_SPHERE, OBJ_TORUS, OBJ_CYLINDER, OBJ_TEAPOT, OBJ_CONE };
struct Color3 {
    float r, g, b;
    Color3(float r_=1, float g_=1, float b_=1): r(r_), g(g_), b(b_) {}
};

struct Vec3 {
    float x, y, z;
    Vec3(float x_=0, float y_=0, float z_=0): x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)       const { return {x*s,   y*s,   z*s  }; }
    float dot(const Vec3& o)      const { return x*o.x + y*o.y + z*o.z; }
    Vec3 cross(const Vec3& o)     const {
        return {y*o.z - z*o.y, z*o.x - x*o.z, x*o.y - y*o.x};
    }
    float length() const { return sqrtf(x*x + y*y + z*z); }
    Vec3 normalized() const {
        float l = length();
        return l > 0 ? (*this * (1.0f/l)) : Vec3(0,0,0);
    }
};


//OBJETO 3D
static int g_nextId = 1;
struct Object3D {
    int id;
    ObjectType type;
    Vec3 position;
    Vec3 rotation;
    Vec3 scale;
    Color3 color;
    bool selected;
    bool wireframe;
    std::string name;
    Object3D(ObjectType t, Vec3 pos, Color3 col)
        : id(g_nextId++), type(t), position(pos),
          rotation(0,0,0), scale(1,1,1), color(col),
          selected(false), wireframe(false)
    {
        const char* names[] = {"Cubo","Esfera","Toro","Cilindro","Tetera","Cono"};
        name = std::string(names[t]) + " " + std::to_string(id);
    }
};

// ESTADO GLOBAL
int winW = 1000, winH = 700;
std::vector<Object3D> scene;
int selectedId = -1;

// CAMARA ORBITAL
float camYaw   =  30.0f;
float camPitch = -20.0f;
float camDist  =  12.0f;
Vec3  camTarget(0, 0, 0);

// CAMARA LIBRE
Vec3  freeCamPos(0.0f, 3.0f, 10.0f);  // posicion inicial
Vec3  freeCamFront(0.0f, -0.2f, -1.0f); // direccion de vista (normalizada)
Vec3  freeCamUp(0.0f, 1.0f, 0.0f);      // vector arriba del mundo
float freeCamYaw   = -90.0f;  // angulo horizontal (grados)
float freeCamPitch =  -10.0f; // angulo vertical (grados)
float freeCamSpeed = 0.3f;    // velocidad de desplazamiento

// Recalcula freeCamFront desde freeCamYaw y freeCamPitch
void updateFreeCamFront() {
    float yR = freeCamYaw   * M_PI / 180.0f;
    float pR = freeCamPitch * M_PI / 180.0f;
    freeCamFront.x = cosf(pR) * cosf(yR);
    freeCamFront.y = sinf(pR);
    freeCamFront.z = cosf(pR) * sinf(yR);
    freeCamFront = freeCamFront.normalized();
}

// MODO DE CAMARA ACTIVO
enum CameraMode { CAM_ORBITAL, CAM_FREE };
CameraMode cameraMode = CAM_ORBITAL;


// PROYECCION
float camFov  = 60.0f;
float camNear =  0.1f;
float camFar  = 200.0f;

// MOUSE
bool mouseDown  = false;
bool rightDown  = false;
int  lastMouseX = 0, lastMouseY = 0;

// Mouse libre para free cam (giro de vista)
bool freeCamMouseLook = false; // activado con clic izquierdo en modo FREE

// VISTA
bool globalWireframe = false;

// HELPERS
Object3D* findById(int id) {
    for (auto& o : scene) if (o.id == id) return &o;
    return nullptr;
}
int selectedIndex() {
    for (int i = 0; i < (int)scene.size(); i++)
        if (scene[i].id == selectedId) return i;
    return -1;
}


// POSICION DE CAMARA ORBITAL
Vec3 orbitalCameraPosition() {
    float yR = camYaw   * M_PI / 180.0f;
    float pR = camPitch * M_PI / 180.0f;
    float cx = camDist * cosf(pR) * sinf(yR);
    float cy = camDist * sinf(pR);
    float cz = camDist * cosf(pR) * cosf(yR);
    return camTarget + Vec3(cx, cy, cz);
}

// DIBUJO DE PRIMITIVAS
void drawPrimitive(const Object3D& o) {
    switch (o.type) {
        case OBJ_CUBE:     glutSolidCube(1.0); break;
        case OBJ_SPHERE:   glutSolidSphere(0.6, 32, 32); break;
        case OBJ_TORUS:    glutSolidTorus(0.2, 0.5, 24, 48); break;
        case OBJ_CYLINDER: glutSolidCylinder(0.5, 1.0, 24, 8); break;
        case OBJ_TEAPOT:   glutSolidTeapot(0.6); break;
        case OBJ_CONE:     glutSolidCone(0.5, 1.0, 24, 8); break;
    }
}
void drawPrimitiveWire(const Object3D& o) {
    switch (o.type) {
        case OBJ_CUBE:     glutWireCube(1.0); break;
        case OBJ_SPHERE:   glutWireSphere(0.6, 16, 16); break;
        case OBJ_TORUS:    glutWireTorus(0.2, 0.5, 12, 24); break;
        case OBJ_CYLINDER: glutWireCylinder(0.5, 1.0, 12, 4); break;
        case OBJ_TEAPOT:   glutWireTeapot(0.6); break;
        case OBJ_CONE:     glutWireCone(0.5, 1.0, 16, 4); break;
    }
}
void drawObject(const Object3D& o) {
    glPushMatrix();
    glTranslatef(o.position.x, o.position.y, o.position.z);
    glRotatef(o.rotation.z, 0, 0, 1);
    glRotatef(o.rotation.y, 0, 1, 0);
    glRotatef(o.rotation.x, 1, 0, 0);
    glScalef(o.scale.x, o.scale.y, o.scale.z);
    bool wire = globalWireframe || o.wireframe;
    if (wire) {
        glColor3f(o.color.r, o.color.g, o.color.b);
        drawPrimitiveWire(o);
    } else {
        glColor3f(o.color.r, o.color.g, o.color.b);
        drawPrimitive(o);
        glLineWidth(1.0f);
        glColor3f(0, 0, 0);
        drawPrimitiveWire(o);
    }
    if (o.selected) {
        glDisable(GL_LIGHTING);
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(2.5f);
        drawPrimitiveWire(o);
        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }
    glPopMatrix();
}

// GIZMO Y CUADRICULA
void drawGizmo() {
    glDisable(GL_LIGHTING);
    glLineWidth(2.5f);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0); glVertex3f(0,0,0); glVertex3f(2,0,0);
        glColor3f(0, 1, 0); glVertex3f(0,0,0); glVertex3f(0,2,0);
        glColor3f(0, 0, 1); glVertex3f(0,0,0); glVertex3f(0,0,2);
    glEnd();
    glColor3f(1, 0.3f, 0.3f); glRasterPos3f(2.1f,0,0); glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,'X');
    glColor3f(0.3f, 1, 0.3f); glRasterPos3f(0,2.1f,0); glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,'Y');
    glColor3f(0.3f,0.3f, 1);  glRasterPos3f(0,0,2.1f); glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12,'Z');
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}
void drawGrid() {
    glDisable(GL_LIGHTING);
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(1.0f);
    glBegin(GL_LINES);
    for (int i = -10; i <= 10; i++) {
        glVertex3f((float)i, 0, -10); glVertex3f((float)i, 0, 10);
        glVertex3f(-10, 0, (float)i); glVertex3f(10,  0, (float)i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
}


//HUD
void drawText2D(float x, float y, const std::string& text,void* font = GLUT_BITMAP_HELVETICA_12) {
    glRasterPos2f(x, y);
    for (char c : text) glutBitmapCharacter(font, c);
}
void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0.05f, 0.05f, 0.1f, 0.80f);
    glBegin(GL_QUADS);
        glVertex2f(0,0); glVertex2f(215,0);
        glVertex2f(215,winH); glVertex2f(0,winH);
    glEnd();
    glColor3f(1, 1, 0.4f);
    drawText2D(8, winH-20, "EDITOR 3D - Lab7", GLUT_BITMAP_HELVETICA_18);
    if (cameraMode == CAM_ORBITAL) {
        glColor3f(0.3f, 1.0f, 0.5f);
        drawText2D(8, winH-40, "CAM: ORBITAL  [M: cambiar]");
    } else {
        glColor3f(1.0f, 0.6f, 0.2f);
        drawText2D(8, winH-40, "CAM: LIBRE    [M: cambiar]");
    }
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-62, "--- CREAR ---");
    glColor3f(1,1,1);
    drawText2D(8, winH-78,  "C : Cubo");
    drawText2D(8, winH-94,  "E : Esfera");
    drawText2D(8, winH-110, "T : Toro");
    drawText2D(8, winH-126, "Y : Cilindro");
    drawText2D(8, winH-142, "P : Tetera");
    drawText2D(8, winH-158, "N : Cono");
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-178, "--- SELECCION ---");
    glColor3f(1,1,1);
    drawText2D(8, winH-194, "Tab : sig. objeto");
    drawText2D(8, winH-210, "Del : eliminar");
    drawText2D(8, winH-226, "k/K : duplicar");
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-246, "--- EDITAR OBJETO ---");
    glColor3f(1,1,1);
    drawText2D(8, winH-262, "Flechas : mover X/Z");
    if (cameraMode == CAM_ORBITAL) {
        drawText2D(8, winH-278, "Q/A  : mover Y +/-");
        drawText2D(8, winH-294, "R/r  : rotar Y");
        drawText2D(8, winH-310, "F/f  : rotar X");
        drawText2D(8, winH-326, "S/s  : escala +/-");
        drawText2D(8, winH-342, "W    : wireframe");
    } else {
        drawText2D(8, winH-278, "Q/q  : mover Y +/-");
        drawText2D(8, winH-294, "r  : rotar Y");
        drawText2D(8, winH-310, "f  : rotar X");
        drawText2D(8, winH-326, "*/div  : escala +/-");
        drawText2D(8, winH-342, "v/V    : wireframe");
    }
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-362, "--- CAMARA ORBITAL ---");
    glColor3f(cameraMode==CAM_ORBITAL ? 1.0f : 0.5f, 1,cameraMode==CAM_ORBITAL ? 1.0f : 0.5f);
    drawText2D(8, winH-378, "Drag izq: orbita");
    drawText2D(8, winH-394, "Drag der: pan");
    drawText2D(8, winH-410, "Rueda: zoom");
    drawText2D(8, winH-426, "H : reset camara");
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-446, "--- CAM LIBRE ---");
    glColor3f(cameraMode==CAM_FREE ? 1.0f : 0.5f, 1,cameraMode==CAM_FREE ? 1.0f : 0.5f);
    drawText2D(8, winH-462, "W/S: adelante/atras");
    drawText2D(8, winH-478, "A/D: izq/der");
    drawText2D(8, winH-494, "R/F: subir/bajar");
    drawText2D(8, winH-510, "Drag izq: girar vista");
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-530, "--- PERSPECTIVA ---");
    glColor3f(1,1,1);
    drawText2D(8, winH-546, "+/-  : FOV");
    drawText2D(8, winH-562, "z/x  : near -/+");
    drawText2D(8, winH-578, "[/]  : far  -/+");
    glColor3f(0.7f, 0.9f, 1.0f);
    drawText2D(8, winH-598, "--- ARCHIVO ---");
    glColor3f(1,1,1);
    drawText2D(8, winH-614, "g/l  : guardar/cargar");
    int panelX = winW - 200;
    glColor4f(0.05f, 0.05f, 0.1f, 0.75f);
    glBegin(GL_QUADS);
        glVertex2f(panelX,0); glVertex2f(winW,0);
        glVertex2f(winW,winH); glVertex2f(panelX,winH);
    glEnd();
    glColor3f(1,1,0.4f);
    drawText2D(panelX+8, winH-20, "ESCENA", GLUT_BITMAP_HELVETICA_18);
    glColor3f(0.6f,0.6f,0.6f);
    drawText2D(panelX+8, winH-38, std::to_string(scene.size())+" objeto(s)");
    int yy = winH - 58;
    for (int i = (int)scene.size()-1; i >= 0 && yy > 20; i--, yy -= 16) {
        glColor3f(scene[i].selected ? 1.0f : 0.85f,
                  scene[i].selected ? 1.0f : 0.85f,
                  scene[i].selected ? 0.0f : 0.85f);
        drawText2D(panelX+8, yy, scene[i].name);
    }
    Object3D* sel = findById(selectedId);
    if (sel) {
        glColor4f(0.05f, 0.1f, 0.05f, 0.82f);
        glBegin(GL_QUADS);
            glVertex2f(215,0); glVertex2f(panelX,0);
            glVertex2f(panelX,90); glVertex2f(215,90);
        glEnd();
        glColor3f(0.4f,1,0.4f);
        drawText2D(225, 72, "Sel: " + sel->name, GLUT_BITMAP_HELVETICA_18);
        char buf[160];
        glColor3f(1,1,1);
        sprintf(buf,"Pos:(%.2f, %.2f, %.2f)",sel->position.x,sel->position.y,sel->position.z);
        drawText2D(225,52,buf);
        sprintf(buf,"Rot:(%.1f, %.1f, %.1f)",sel->rotation.x,sel->rotation.y,sel->rotation.z);
        drawText2D(225,36,buf);
        sprintf(buf,"Esc:(%.2f, %.2f, %.2f)",sel->scale.x,sel->scale.y,sel->scale.z);
        drawText2D(225,20,buf);
    }
    glColor3f(0.8f,0.8f,0.8f);
    char camBuf[160];
    if (cameraMode == CAM_ORBITAL) {
        sprintf(camBuf,"ORBITAL | FOV:%.0f  Near:%.2f  Far:%.0f  Dist:%.1f  Yaw:%.0f  Pitch:%.0f",
                camFov, camNear, camFar, camDist, camYaw, camPitch);
    } else {
        sprintf(camBuf,"FREE | FOV:%.0f  Near:%.2f  Far:%.0f  Pos:(%.1f,%.1f,%.1f)  Yaw:%.0f  Pitch:%.0f",
                camFov, camNear, camFar,
                freeCamPos.x, freeCamPos.y, freeCamPos.z,
                freeCamYaw, freeCamPitch);
    }
    drawText2D(220, winH-20, camBuf);
    if (globalWireframe) {
        glColor3f(1,0.5f,0);
        drawText2D(220, winH-38, "[WIREFRAME GLOBAL]");
    }
    if (cameraMode == CAM_FREE && freeCamMouseLook) {
        glColor3f(1,0.8f,0.2f);
        drawText2D(220, winH-54, "[MOUSE LOOK activo - clic izq para soltar]");
    }
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

// LUZ
void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_NORMALIZE);
    GLfloat amb[]  = {0.25f,0.25f,0.25f,1};
    GLfloat diff[] = {0.85f,0.85f,0.85f,1};
    GLfloat spec[] = {0.5f, 0.5f, 0.5f, 1};
    GLfloat pos[]  = {5,8,5,1};
    glLightfv(GL_LIGHT0,GL_AMBIENT,  amb);
    glLightfv(GL_LIGHT0,GL_DIFFUSE,  diff);
    glLightfv(GL_LIGHT0,GL_SPECULAR, spec);
    glLightfv(GL_LIGHT0,GL_POSITION, pos);
}

// DISPLAY PRINCIPAL
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(camFov, (double)winW/winH, camNear, camFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if (cameraMode == CAM_ORBITAL) {
        Vec3 eye = orbitalCameraPosition();
        gluLookAt(eye.x, eye.y, eye.z,
                  camTarget.x, camTarget.y, camTarget.z,
                  0, 1, 0);
    } else {
        Vec3 target = freeCamPos + freeCamFront;
        gluLookAt(freeCamPos.x, freeCamPos.y, freeCamPos.z,
                  target.x,     target.y,     target.z,
                  freeCamUp.x,  freeCamUp.y,  freeCamUp.z);
    }
    setupLighting();
    drawGrid();
    drawGizmo();
    for (auto& o : scene) drawObject(o);
    drawHUD();
    glutSwapBuffers();
}
void reshape(int w, int h) {
    winW = w; winH = h;
    glViewport(0,0,w,h);
    glutPostRedisplay();
}

// CREACION DE OBJETOS
Color3 nextColor() {
    static int idx = 0;
    static Color3 pal[] = {
        {0.9f,0.3f,0.3f},{0.3f,0.7f,0.9f},{0.4f,0.85f,0.4f},
        {0.9f,0.7f,0.2f},{0.7f,0.3f,0.9f},{0.9f,0.5f,0.2f},
        {0.3f,0.9f,0.7f},{0.9f,0.3f,0.7f}
    };
    return pal[idx++ % 8];
}
Vec3 nextPosition() {
    static float off = 0;
    off += 0.5f;
    if (off > 4.0f) off = 0.0f;
    return Vec3(off-2.0f, 0.5f, off-2.0f);
}
void addObject(ObjectType t) {
    scene.emplace_back(t, nextPosition(), nextColor());
    if (selectedId != -1) { Object3D* p=findById(selectedId); if(p) p->selected=false; }
    selectedId = scene.back().id;
    scene.back().selected = true;
    glutPostRedisplay();
}

// SELECCION
void selectNext() {
    if (scene.empty()) return;
    int idx = selectedIndex();
    if (selectedId != -1 && idx >= 0) scene[idx].selected = false;
    idx = (idx+1) % (int)scene.size();
    selectedId = scene[idx].id;
    scene[idx].selected = true;
    glutPostRedisplay();
}
void selectByIndex(int idx) {
    if (idx < 0 || idx >= (int)scene.size()) return;
    if (selectedId != -1) { Object3D* p=findById(selectedId); if(p) p->selected=false; }
    selectedId = scene[idx].id;
    scene[idx].selected = true;
}

// OPERACIONES SOBRE OBJETOS
void deleteSelected() {
    int idx = selectedIndex();
    if (idx < 0) return;
    scene.erase(scene.begin()+idx);
    selectedId = -1;
    if (!scene.empty()) selectByIndex(std::min(idx,(int)scene.size()-1));
    glutPostRedisplay();
}
void duplicateSelected() {
    Object3D* o = findById(selectedId);
    if (!o) return;
    Object3D copy = *o;
    copy.id = g_nextId++;
    copy.name = o->name + "_dup";
    copy.position.x += 1.0f;
    copy.selected = false;
    scene.push_back(copy);
    o->selected = false;
    selectedId = scene.back().id;
    scene.back().selected = true;
    glutPostRedisplay();
}

// GUARDAR / CARGAR
void saveScene(const std::string& fn) {
    std::ofstream f(fn);
    if (!f) { std::cerr<<"No se pudo abrir "<<fn<<"\n"; return; }
    for (auto& o : scene)
        f<<o.id<<" "<<(int)o.type<<" "
         <<o.position.x<<" "<<o.position.y<<" "<<o.position.z<<" "
         <<o.rotation.x<<" "<<o.rotation.y<<" "<<o.rotation.z<<" "
         <<o.scale.x<<" "<<o.scale.y<<" "<<o.scale.z<<" "
         <<o.color.r<<" "<<o.color.g<<" "<<o.color.b<<"\n";
}
void loadScene(const std::string& fn) {
    std::ifstream f(fn);
    if (!f) { std::cerr<<"No se pudo abrir "<<fn<<"\n"; return; }
    scene.clear(); selectedId = -1;
    std::string line;
    while (std::getline(f,line)) {
        if (line.empty()) continue;
        std::istringstream ss(line);
        int id, type;
        Object3D o((ObjectType)0,Vec3(),Color3());
        ss>>id>>type
          >>o.position.x>>o.position.y>>o.position.z
          >>o.rotation.x>>o.rotation.y>>o.rotation.z
          >>o.scale.x>>o.scale.y>>o.scale.z
          >>o.color.r>>o.color.g>>o.color.b;
        o.id=id; o.type=(ObjectType)type;
        if(id>=g_nextId) g_nextId=id+1;
        const char* names[]={"Cubo","Esfera","Toro","Cilindro","Tetera","Cono"};
        o.name=std::string(names[type])+" "+std::to_string(id);
        scene.push_back(o);
    }
    glutPostRedisplay();
}

// ESCENA INICIAL
void createInitialScene() {
    scene.clear(); selectedId = -1;
    auto add = [&](ObjectType t, Vec3 p, Color3 c){ scene.emplace_back(t,p,c); };
    add(OBJ_CUBE,     {-3, 0.5f,  0}, {0.9f,0.3f,0.3f});
    add(OBJ_SPHERE,   { 0, 0.6f,  0}, {0.3f,0.6f,0.9f});
    add(OBJ_TORUS,    { 3, 0.5f,  0}, {0.4f,0.9f,0.4f});
    add(OBJ_CYLINDER, { 0, 0.0f,  3}, {0.2f,0.9f,0.8f});
    add(OBJ_TEAPOT,   { 0, 0.0f, -3}, {0.9f,0.8f,0.2f});
    add(OBJ_CONE,     {-3, 0.0f, -3}, {0.7f,0.3f,0.9f});
    scene[0].rotation.y = 20.0f;
    scene[2].scale      = {1.2f,1.2f,1.2f};
    scene[4].scale      = {0.8f,1.4f,0.8f};
    selectByIndex(0);
}

// TECLADO
void keyboard(unsigned char key, int x, int y) {
    Object3D* sel = findById(selectedId);
    float tStep=0.2f, rStep=10.0f, sStep=0.1f;
    if (key == 'm' || key == 'M') {
        if (cameraMode == CAM_ORBITAL) {
            cameraMode = CAM_FREE;
            Vec3 eye = orbitalCameraPosition();
            freeCamPos = eye;
            Vec3 dir = (camTarget - eye).normalized();
            freeCamFront = dir;
            freeCamPitch = asinf(dir.y) * 180.0f / M_PI;
            freeCamYaw   = atan2f(dir.z, dir.x) * 180.0f / M_PI;
            freeCamMouseLook = false;
        } else {
            cameraMode = CAM_ORBITAL;
            freeCamMouseLook = false;
        }
        glutPostRedisplay();
        return;
    }
    switch(key) {
        case '+': camFov  = std::max(10.0f,  camFov-2.0f);   glutPostRedisplay(); return;
        case '-': camFov  = std::min(150.0f, camFov+2.0f);   glutPostRedisplay(); return;
        case 'z': camNear = std::max(0.01f,  camNear-0.05f); glutPostRedisplay(); return;
        case 'x': camNear = std::min(camFar-0.1f, camNear+0.05f); glutPostRedisplay(); return;
        case '[': camFar  = std::max(camNear+1.0f, camFar-10.0f); glutPostRedisplay(); return;
        case ']': camFar  = std::min(2000.0f, camFar+10.0f); glutPostRedisplay(); return;
    }
    switch(key) {
        case 'c': case 'C': addObject(OBJ_CUBE);     return;
        case 'e': case 'E': addObject(OBJ_SPHERE);   return;
        case 't': case 'T': addObject(OBJ_TORUS);    return;
        case 'y': case 'Y': addObject(OBJ_CYLINDER); return;
        case 'p': case 'P': addObject(OBJ_TEAPOT);   return;
        case 'n': case 'N': addObject(OBJ_CONE);     return;
    }
    switch(key) {
        case '\t': selectNext(); return;
        case 127:  deleteSelected(); return;
        case 'k': case 'K': duplicateSelected(); return;
        case 'g':  saveScene("scene.txt"); return;
        case 'l':  loadScene("scene.txt"); return;
        case 'h': case 'H':
            camYaw=30; camPitch=-20; camDist=12; camTarget=Vec3(0,0,0);
            glutPostRedisplay(); return;
        case 27:
            if(cameraMode==CAM_FREE && freeCamMouseLook){
                freeCamMouseLook=false; glutPostRedisplay();
            } else if(selectedId!=-1){
                Object3D* p=findById(selectedId);
                if(p) p->selected=false;
                selectedId=-1; glutPostRedisplay();
            }
            return;
    }
    //subir
    if (key=='Q') { if(sel){sel->position.y+=tStep; glutPostRedisplay();} return; }
    //bajar
    if (key=='q') { if(sel){sel->position.y-=tStep; glutPostRedisplay();} return; }
    if (key=='a' || key=='A') {
        if (cameraMode == CAM_FREE) goto freecam_keys;
        if(sel){sel->position.y-=tStep; glutPostRedisplay();} return;
    }
    if (cameraMode == CAM_ORBITAL) {
        switch(key) {
            case 'R': if(sel){sel->rotation.y+=rStep; glutPostRedisplay();} break;
            case 'r': if(sel){sel->rotation.y-=rStep; glutPostRedisplay();} break;
            case 'F': if(sel){sel->rotation.x+=rStep; glutPostRedisplay();} break;
            case 'f': if(sel){sel->rotation.x-=rStep; glutPostRedisplay();} break;
            case 'S': if(sel){sel->scale.x+=sStep;sel->scale.y+=sStep;sel->scale.z+=sStep; glutPostRedisplay();} break;
            case 's': if(sel){
                          sel->scale.x=std::max(0.05f,sel->scale.x-sStep);
                          sel->scale.y=std::max(0.05f,sel->scale.y-sStep);
                          sel->scale.z=std::max(0.05f,sel->scale.z-sStep);
                          glutPostRedisplay();} break;
            case 'w': case 'W': globalWireframe=!globalWireframe; glutPostRedisplay(); break;
        }
    } else {
        freecam_keys:
        Vec3 right = freeCamFront.cross(freeCamUp).normalized();
        switch(key) {
            case 'w': case 'W':
                freeCamPos = freeCamPos + freeCamFront * freeCamSpeed;
                glutPostRedisplay(); break;
            case 's': case 'S':
                freeCamPos = freeCamPos - freeCamFront * freeCamSpeed;
                glutPostRedisplay(); break;
            case 'a': case 'A':
                freeCamPos = freeCamPos - right * freeCamSpeed;
                glutPostRedisplay(); break;
            case 'd': case 'D':
                freeCamPos = freeCamPos + right * freeCamSpeed;
                glutPostRedisplay(); break;
            case 'R':
                freeCamPos.y += freeCamSpeed;
                glutPostRedisplay(); break;
            case 'F':
                freeCamPos.y -= freeCamSpeed;
                glutPostRedisplay(); break;
            case 'r': if(sel){sel->rotation.y-=rStep; glutPostRedisplay();} break;
            case 'f': if(sel){sel->rotation.x-=rStep; glutPostRedisplay();} break;
            case '*': if(sel){sel->scale.x+=sStep;sel->scale.y+=sStep;sel->scale.z+=sStep; glutPostRedisplay();} break;
            case '/': if(sel){//falta
                          sel->scale.x=std::max(0.05f,sel->scale.x-sStep);
                          sel->scale.y=std::max(0.05f,sel->scale.y-sStep);
                          sel->scale.z=std::max(0.05f,sel->scale.z-sStep);
                          glutPostRedisplay();} break;
            case 'v': case 'V': globalWireframe=!globalWireframe; glutPostRedisplay(); break;
        }
    }
}
void specialKeys(int key, int x, int y) {
    Object3D* sel = findById(selectedId);
    float tStep = 0.2f;
    switch(key) {
        case GLUT_KEY_LEFT:  if(sel){sel->position.x-=tStep; glutPostRedisplay();} break;
        case GLUT_KEY_RIGHT: if(sel){sel->position.x+=tStep; glutPostRedisplay();} break;
        case GLUT_KEY_UP:    if(sel){sel->position.z-=tStep; glutPostRedisplay();} break;
        case GLUT_KEY_DOWN:  if(sel){sel->position.z+=tStep; glutPostRedisplay();} break;
        case GLUT_KEY_HOME:
            camYaw=30; camPitch=-20; camDist=12; camTarget=Vec3(0,0,0);
            glutPostRedisplay(); break;
    }
}

// MOUSE
void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (cameraMode == CAM_FREE) {
            if (state == GLUT_DOWN) {
                freeCamMouseLook = !freeCamMouseLook;
                lastMouseX = x; lastMouseY = y;
            }
        } else {
            mouseDown  = (state == GLUT_DOWN);
            lastMouseX = x; lastMouseY = y;
        }
    }
    if (button == GLUT_RIGHT_BUTTON) {
        rightDown  = (state == GLUT_DOWN);
        lastMouseX = x; lastMouseY = y;
    }
    if (button == 3) {
        if (cameraMode == CAM_ORBITAL)
            camDist = std::max(1.0f, camDist-0.5f);
        else
            freeCamPos = freeCamPos + freeCamFront * freeCamSpeed * 2.0f;
        glutPostRedisplay();
    }
    if (button == 4) {
        if (cameraMode == CAM_ORBITAL)
            camDist = std::min(80.0f, camDist+0.5f);
        else
            freeCamPos = freeCamPos - freeCamFront * freeCamSpeed * 2.0f;
        glutPostRedisplay();
    }
}
void mouseMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;
    lastMouseX = x; lastMouseY = y;
    if (cameraMode == CAM_ORBITAL) {
        if (mouseDown) {
            camYaw   += dx * 0.4f;
            camPitch += dy * 0.4f;
            camPitch  = std::max(-89.0f, std::min(89.0f, camPitch));
            glutPostRedisplay();
        }
        if (rightDown) {
            float yR  = camYaw * M_PI / 180.0f;
            Vec3 right = {cosf(yR), 0, -sinf(yR)};
            float spd  = camDist * 0.002f;
            camTarget  = camTarget + right * (-dx*spd);
            camTarget.y += dy * spd;
            glutPostRedisplay();
        }
    } else {
        if (freeCamMouseLook) {
            float sensitivity = 0.25f;
            freeCamYaw   += dx * sensitivity;
            freeCamPitch -= dy * sensitivity;
            freeCamPitch  = std::max(-89.0f, std::min(89.0f, freeCamPitch));
            updateFreeCamFront();
            glutPostRedisplay();
        }
    }
}

// MAIN
int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Editor Grafico 3D - OpenGL | Lab7");
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.12f, 0.12f, 0.18f, 1.0f);
    glShadeModel(GL_SMOOTH);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    updateFreeCamFront();
    createInitialScene();
    glutMainLoop();
    return 0;
}