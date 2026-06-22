#include <GL/freeglut.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// TEXTURAS
static GLuint texGrass, texBrick, texRoof, texWood, texLeaves, texDoor;
static GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 3);
    
    if (!data) {
        printf("Error: No se pudo cargar la textura '%s'\n", filename);
        return 0;
    }
    
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    
    // Configurar parámetros de textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    // Cargar la imagen
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
    
    stbi_image_free(data);
    return id;
}

static void generateAllTextures() {
    texGrass  = loadTexture("texturas/cesped.jpg");
    texBrick  = loadTexture("texturas/ladrillo.jpg");
    texRoof   = loadTexture("texturas/tejas.jpg");
    texWood   = loadTexture("texturas/madera.jpg");
    texLeaves = loadTexture("texturas/hojas.jpg");
    texDoor   = loadTexture("texturas/puerta.jpg");
}

// ESTADO GLOBAL
int winW = 900, winH = 650;
float camYaw    =  25.0f;
float camPitch  = -20.0f;
float camDist   =  30.0f;
float camTarget[3] = {0.0f, 1.0f, 0.0f};

bool useDirLight   = true;
bool texturesOn    = true;
bool wireframeMode = false;

// MATERIALES
enum MatID { MAT_TERRAIN, MAT_WALL, MAT_ROOF, MAT_TRUNK, MAT_LEAVES, MAT_DOOR };

static void applyMaterial(MatID m) {
    GLfloat amb[4], dif[4], spec[4];    
    GLfloat shin;
    switch(m) {
        case MAT_TERRAIN:
            amb[0]=0.10f; amb[1]=0.25f; amb[2]=0.05f; amb[3]=1;
            dif[0]=0.20f; dif[1]=0.55f; dif[2]=0.10f; dif[3]=1;
            spec[0]=0.05f; spec[1]=0.10f; spec[2]=0.05f; spec[3]=1;
            shin = 8.0f; break;
        case MAT_WALL:
            amb[0]=0.50f; amb[1]=0.15f; amb[2]=0.10f; amb[3]=1;
            dif[0]=0.75f; dif[1]=0.18f; dif[2]=0.10f; dif[3]=1;
            spec[0]=0.15f; spec[1]=0.05f; spec[2]=0.05f; spec[3]=1;
            shin = 12.0f; break;
        case MAT_ROOF:
            amb[0]=0.20f; amb[1]=0.20f; amb[2]=0.22f; amb[3]=1;
            dif[0]=0.45f; dif[1]=0.45f; dif[2]=0.50f; dif[3]=1;
            spec[0]=0.30f; spec[1]=0.30f; spec[2]=0.35f; spec[3]=1;
            shin = 32.0f; break;
        case MAT_TRUNK:
            amb[0]=0.20f; amb[1]=0.10f; amb[2]=0.03f; amb[3]=1;
            dif[0]=0.45f; dif[1]=0.22f; dif[2]=0.06f; dif[3]=1;
            spec[0]=0.05f; spec[1]=0.03f; spec[2]=0.01f; spec[3]=1;
            shin = 4.0f; break;
        case MAT_LEAVES:
            amb[0]=0.05f; amb[1]=0.20f; amb[2]=0.03f; amb[3]=1;
            dif[0]=0.15f; dif[1]=0.55f; dif[2]=0.10f; dif[3]=1;
            spec[0]=0.10f; spec[1]=0.30f; spec[2]=0.10f; spec[3]=1;
            shin = 16.0f; break;
        case MAT_DOOR:
            amb[0]=0.25f; amb[1]=0.12f; amb[2]=0.05f; amb[3]=1;
            dif[0]=0.55f; dif[1]=0.27f; dif[2]=0.07f; dif[3]=1;
            spec[0]=0.10f; spec[1]=0.05f; spec[2]=0.02f; spec[3]=1;
            shin = 8.0f;break;
        default: return;
    }
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,  amb);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,  dif);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec);
    glMaterialf (GL_FRONT_AND_BACK, GL_SHININESS, shin);
}

// ILUMINACIÓN
static void setupLighting() {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    GLfloat ambLight[]  = {0.30f, 0.30f, 0.28f, 1.0f};
    GLfloat diffLight[] = {0.90f, 0.88f, 0.80f, 1.0f};
    GLfloat specLight[] = {0.80f, 0.80f, 0.75f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT,  ambLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diffLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specLight);

    if (useDirLight) {
        GLfloat dirPos[] = {0.5f, 1.0f, 0.7f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, dirPos);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.0f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
    } else {
        GLfloat ptPos[] = {5.0f, 8.0f, 5.0f, 1.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, ptPos);
        glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  1.0f);
        glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.02f);
        glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.005f);
    }
    GLfloat globalAmb[] = {0.15f, 0.15f, 0.15f, 1.0f};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
}

// FUNCIONES DE DIBUJO CON TEXTURA
static void quadTex(float x0,float y0,float z0, float u0,float v0,
                    float x1,float y1,float z1, float u1,float v1,
                    float x2,float y2,float z2, float u2,float v2,
                    float x3,float y3,float z3, float u3,float v3) {
    float ax=x2-x0, ay=y2-y0, az=z2-z0;
    float bx=x1-x3, by=y1-y3, bz=z1-z3;
    float nx=ay*bz-az*by, ny=az*bx-ax*bz, nz=ax*by-ay*bx;
    float len=sqrtf(nx*nx+ny*ny+nz*nz);
    if(len>0){ nx/=len; ny/=len; nz/=len; }
    glNormal3f(nx,ny,nz);
    glTexCoord2f(u0,v0); glVertex3f(x0,y0,z0);
    glTexCoord2f(u1,v1); glVertex3f(x1,y1,z1);
    glTexCoord2f(u2,v2); glVertex3f(x2,y2,z2);
    glTexCoord2f(u3,v3); glVertex3f(x3,y3,z3);
}

static void drawTexturedCylinder(float radius, float height, int slices, int stacks) {
    float stepTheta = 2.0f * M_PI / slices;
    float stepV = 1.0f / stacks;
    glBegin(GL_QUADS);
    for (int i = 0; i < slices; i++) {
        float t0 = i * stepTheta;
        float t1 = (i+1) * stepTheta;
        float x0 = radius * cosf(t0), z0 = radius * sinf(t0);
        float x1 = radius * cosf(t1), z1 = radius * sinf(t1);
        for (int j = 0; j < stacks; j++) {
            float y0 = j * height / stacks;
            float y1 = (j+1) * height / stacks;
            float v0 = (float)j / stacks;
            float v1 = (float)(j+1) / stacks;
            glNormal3f(cosf(t0), 0.0f, sinf(t0));
            glTexCoord2f((float)i/slices, v0); glVertex3f(x0, y0, z0);
            glTexCoord2f((float)(i+1)/slices, v0); glVertex3f(x1, y0, z1);
            glTexCoord2f((float)(i+1)/slices, v1); glVertex3f(x1, y1, z1);
            glTexCoord2f((float)i/slices, v1); glVertex3f(x0, y1, z0);
        }
    }
    glEnd();
}

static void drawTexturedSphere(float radius, int slices, int stacks) {
    for (int i = 0; i < slices; i++) {
        float theta0 = 2.0f * M_PI * i / slices;
        float theta1 = 2.0f * M_PI * (i+1) / slices;
        for (int j = 0; j < stacks; j++) {
            float phi0 = M_PI * j / stacks;
            float phi1 = M_PI * (j+1) / stacks;
            float x0 = radius * sinf(phi0) * cosf(theta0);
            float y0 = radius * cosf(phi0);
            float z0 = radius * sinf(phi0) * sinf(theta0);
            float x1 = radius * sinf(phi0) * cosf(theta1);
            float z1 = radius * sinf(phi0) * sinf(theta1);
            float x2 = radius * sinf(phi1) * cosf(theta1);
            float y2 = radius * cosf(phi1);
            float z2 = radius * sinf(phi1) * sinf(theta1);
            float x3 = radius * sinf(phi1) * cosf(theta0);
            float z3 = radius * sinf(phi1) * sinf(theta0);
            glBegin(GL_QUADS);
                glNormal3f(x0/radius, y0/radius, z0/radius);
                glTexCoord2f((float)i/slices, (float)j/stacks);
                glVertex3f(x0, y0, z0);
                glNormal3f(x1/radius, y0/radius, z1/radius);
                glTexCoord2f((float)(i+1)/slices, (float)j/stacks);
                glVertex3f(x1, y0, z1);
                glNormal3f(x2/radius, y2/radius, z2/radius);
                glTexCoord2f((float)(i+1)/slices, (float)(j+1)/stacks);
                glVertex3f(x2, y2, z2);
                glNormal3f(x3/radius, y2/radius, z3/radius);
                glTexCoord2f((float)i/slices, (float)(j+1)/stacks);
                glVertex3f(x3, y2, z3);
            glEnd();
        }
    }
}

static void drawTexturedCube(float size) {
    float s = size/2.0f;
    glBegin(GL_QUADS);
        quadTex(-s,-s, s, 0,0,   s,-s, s, 1,0,   s, s, s, 1,1,  -s, s, s, 0,1);
        quadTex( s,-s,-s, 0,0,  -s,-s,-s, 1,0,  -s, s,-s, 1,1,   s, s,-s, 0,1);
        quadTex( s,-s,-s, 0,0,   s,-s, s, 1,0,   s, s, s, 1,1,   s, s,-s, 0,1);
        quadTex(-s,-s, s, 0,0,  -s,-s,-s, 1,0,  -s, s,-s, 1,1,  -s, s, s, 0,1);
        quadTex(-s, s,-s, 0,0,   s, s,-s, 1,0,   s, s, s, 1,1,  -s, s, s, 0,1);
        quadTex(-s,-s, s, 0,0,   s,-s, s, 1,0,   s,-s,-s, 1,1,  -s,-s,-s, 0,1);
    glEnd();
}

// DIBUJO DE LA ESCENA
static void drawTerrain() {
    applyMaterial(MAT_TERRAIN);
    if (texturesOn && texGrass) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texGrass);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    // Desactivar culling temporalmente para ver el terreno por ambos lados
    glDisable(GL_CULL_FACE);

    float H = 20.0f, rep = 8.0f;
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0, rep);   glVertex3f(-H, 0.0f,  H);
        glTexCoord2f(rep, rep); glVertex3f( H, 0.0f,  H);
        glTexCoord2f(rep, 0);   glVertex3f( H, 0.0f, -H);
        glTexCoord2f(0, 0);     glVertex3f(-H, 0.0f, -H);
    glEnd();

    // Reactivar culling para el resto de objetos
    glEnable(GL_CULL_FACE);

    if (texturesOn) glDisable(GL_TEXTURE_2D);
}

static void drawDoor() {
    float doorW = 0.8f;
    float doorH = 1.8f;
    float doorY = 0.0f;
    float doorZ = -2.51f;

    applyMaterial(MAT_DOOR);

    if (texturesOn && texDoor) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texDoor);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);

        glTexCoord2f(0.0f, 0.0f); glVertex3f(-doorW/2, doorY,       doorZ);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-doorW/2, doorY+doorH, doorZ);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( doorW/2, doorY+doorH, doorZ);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( doorW/2, doorY,       doorZ);
    glEnd();

    if (texturesOn) glDisable(GL_TEXTURE_2D);
}

static void drawHouseWalls() {
    applyMaterial(MAT_WALL);
    if (texturesOn && texBrick) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBrick);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    float W=3.0f, D=2.5f, H=3.0f;
    float repW=3.0f, repH=2.0f;
    float doorW = 0.8f;
    float doorH = 1.8f;

    glBegin(GL_QUADS);
        // PARED FRONTAL (+Z) - COMPLETA
        quadTex(-W, 0, D, 0, 0,  W, 0, D, repW, 0,  W, H, D, repW, repH,  -W, H, D, 0, repH);

        // PARED TRASERA (-Z)
        // Panel izquierdo - ORDEN ANTIHORARIO visto desde -Z
        quadTex(-doorW/2, 0, -D, (W-doorW/2)/(2*W)*repW, 0,
                -W, 0, -D, 0, 0,
                -W, H, -D, 0, repH,
                -doorW/2, H, -D, (W-doorW/2)/(2*W)*repW, repH);

        // Panel derecho - ORDEN ANTIHORARIO visto desde -Z
        quadTex(W, 0, -D, repW, 0,
                doorW/2, 0, -D, (W+doorW/2)/(2*W)*repW, 0,
                doorW/2, H, -D, (W+doorW/2)/(2*W)*repW, repH,
                W, H, -D, repW, repH);

        // Panel superior - ORDEN ANTIHORARIO visto desde -Z
        quadTex(-doorW/2, doorH, -D, (W-doorW/2)/(2*W)*repW, (H-doorH)/H*repH,
                -doorW/2, H, -D, (W-doorW/2)/(2*W)*repW, repH,
                doorW/2, H, -D, (W+doorW/2)/(2*W)*repW, repH,
                doorW/2, doorH, -D, (W+doorW/2)/(2*W)*repW, (H-doorH)/H*repH);

        // PARED IZQUIERDA Y DERECHA
        // Pared izquierda (-X)
        quadTex(-W,0,-D, 0,0, -W,0,D, repW,0,  -W,H,D, repW,repH, -W,H,-D, 0,repH);
        // Pared derecha (+X)
        quadTex( W,0,D, 0,0,  W,0,-D, repW,0,   W,H,-D, repW,repH,  W,H,D, 0,repH);
    glEnd();

    if (texturesOn) glDisable(GL_TEXTURE_2D);
    drawDoor();
}


static void drawHouseRoof() {
    float W=3.2f, D=2.7f, H=3.0f, RH=5.0f, cx=0.0f;

    // Triángulos frontales (con textura de ladrillo)
    applyMaterial(MAT_WALL);
    if (texturesOn && texBrick) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texBrick);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    glBegin(GL_TRIANGLES);
        // Triángulo frontal (+Z)
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(0.5f, 1.0f); glVertex3f(cx, RH,  D);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-W, H,   D);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( W, H,   D);

        // Triángulo trasero (-Z)
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.5f, 1.0f); glVertex3f(cx, RH, -D);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( W, H,  -D);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-W, H,  -D);
    glEnd();

    if (texturesOn) glDisable(GL_TEXTURE_2D);

    // Superficies inclinadas (con textura de tejas)
    applyMaterial(MAT_ROOF);
    if (texturesOn && texRoof) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texRoof);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }

    glBegin(GL_QUADS);
        quadTex(-W, H, -D, 0,0,  -W, H,  D, 2,0,   cx, RH,  D, 2,1.5f,  cx, RH, -D, 0,1.5f);
        quadTex( W, H,  D, 0,0,   W, H, -D, 2,0,   cx, RH, -D, 2,1.5f,  cx, RH,  D, 0,1.5f);
    glEnd();

    if (texturesOn) glDisable(GL_TEXTURE_2D);
}

static void drawHouse() {
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);
    drawHouseWalls();
    drawHouseRoof();
    
    // Chimenea
    applyMaterial(MAT_ROOF);
    if (texturesOn && texRoof) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texRoof);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    glPushMatrix();
    glTranslatef(1.5f, 4.2f, -1.0f);
    glScalef(0.5f, 1.2f, 0.5f);
    drawTexturedCube(1.0f);
    glPopMatrix();
    if (texturesOn) glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

static void drawTree(float tx, float tz) {
    glPushMatrix();
    glTranslatef(tx, 0.0f, tz);

    // Tronco
    applyMaterial(MAT_TRUNK);
    if (texturesOn && texWood) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texWood);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    drawTexturedCylinder(0.35f, 2.5f, 16, 8);
    if (texturesOn) glDisable(GL_TEXTURE_2D);

    // Copa
    applyMaterial(MAT_LEAVES);
    if (texturesOn && texLeaves) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texLeaves);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    }
    glPushMatrix();
    glTranslatef(0.0f, 2.5f, 0.0f);
    drawTexturedSphere(1.4f, 20, 20);
    glPopMatrix();
    if (texturesOn) glDisable(GL_TEXTURE_2D);

    glPopMatrix();
}

// GIZMO, HUD, CÁMARA, DISPLAY Y CONTROLES
static void drawGizmo() {
    glDisable(GL_LIGHTING);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(3,0,0);
        glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,3,0);
        glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,3);
    glEnd();
    glLineWidth(1.0f);
    glEnable(GL_LIGHTING);
}

static void drawHUD() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity();
    gluOrtho2D(0, winW, 0, winH);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix(); glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_TEXTURE_2D);

    glColor3f(1,1,1);
    auto txt = [&](float x, float y, const char* s) {
        glRasterPos2f(x, y);
        for (const char* c = s; *c; c++) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    };
    txt(10, winH-18, "Lab9 - Escena 3D con Texturas Importadas");
    glColor3f(0.8f,0.8f,0);
    char buf[128];
    sprintf(buf, "Luz: %s  | Texturas: %s | Wire: %s",
            useDirLight ? "DIRECCIONAL" : "PUNTUAL",
            texturesOn  ? "ON" : "OFF",
            wireframeMode ? "ON" : "OFF");
    txt(10, winH-34, buf);
    glColor3f(0.7f,0.9f,1.0f);
    txt(10, winH-52, "Flechas: orbita | Q/E: zoom | L: luz | T: tex | W: wire | H: reset | ESC: salir");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW);  glPopMatrix();
}

static void applyCamera() {
    float yR = camYaw   * M_PI / 180.0f;
    float pR = camPitch * M_PI / 180.0f;
    float ex = camTarget[0] + camDist * cosf(pR) * sinf(yR);
    float ey = camTarget[1] + camDist * sinf(pR);
    float ez = camTarget[2] + camDist * cosf(pR) * cosf(yR);
    gluLookAt(ex, ey, ez,
              camTarget[0], camTarget[1], camTarget[2],
              0, 1, 0);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(55.0, (double)winW/winH, 0.5, 300.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    applyCamera();
    setupLighting();

    glShadeModel(GL_SMOOTH);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    if (wireframeMode) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else               glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    drawTerrain();
    drawHouse();
    drawTree(-6.0f, 4.0f);
    drawTree( 7.5f, 3.0f);
    drawTree(-8.0f,-3.0f);
    drawGizmo();

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawHUD();
    glutSwapBuffers();
}

void reshape(int w, int h) {
    winW=w; winH=h;
    glViewport(0,0,w,h);
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'l': case 'L': useDirLight = !useDirLight; break;
        case 't': case 'T': texturesOn = !texturesOn; break;
        case 'w': case 'W': wireframeMode = !wireframeMode; break;
        case 'q': case 'Q': camDist = fmaxf(5.0f, camDist - 1.5f); break;
        case 'e': case 'E': camDist = fminf(60.0f, camDist + 1.5f); break;
        case 'h': case 'H': camYaw=25; camPitch=-20; camDist=30;
                            camTarget[0]=0; camTarget[1]=1; camTarget[2]=0; break;
        case 27: exit(0);
    }
    glutPostRedisplay();
}

void specialKeys(int key, int x, int y) {
    switch(key) {
        case GLUT_KEY_LEFT:  camYaw -= 4.0f; break;
        case GLUT_KEY_RIGHT: camYaw += 4.0f; break;
        case GLUT_KEY_UP:    camPitch += 3.0f; if(camPitch>88) camPitch=88; break;
        case GLUT_KEY_DOWN:  camPitch -= 3.0f; if(camPitch<-88) camPitch=-88; break;
    }
    glutPostRedisplay();
}

static bool mbDown=false;
static int  mbLastX=0, mbLastY=0;
void mouseBtn(int btn, int state, int x, int y) {
    if(btn==GLUT_LEFT_BUTTON){ mbDown=(state==GLUT_DOWN); mbLastX=x; mbLastY=y; }
    if(btn==3){ camDist=fmaxf(5.0f,camDist-0.8f); glutPostRedisplay(); }
    if(btn==4){ camDist=fminf(60.0f,camDist+0.8f); glutPostRedisplay(); }
}
void mouseMove(int x, int y) {
    if(!mbDown) return;
    camYaw   += (x-mbLastX)*0.5f;
    camPitch += (y-mbLastY)*0.5f;
    if(camPitch>88) camPitch=88;
    if(camPitch<-88) camPitch=-88;
    mbLastX=x; mbLastY=y;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Lab9 - Escena 3D con Texturas Importadas");
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    // Cargar texturas desde archivos
    generateAllTextures();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseBtn);
    glutMotionFunc(mouseMove);

    glutMainLoop();
    return 0;
}