// Laboratorio 9: Escena 3D con Terreno, Casa, Árbol, Materiales, Iluminación y Texturas
// Compilar: g++ lab9.cpp -o lab9 -lfreeglut -lGL -lGLU -lm

#include <GL/freeglut.h>
#include <cmath>
#include <cstdlib>
#include <cstdio>

// Ángulo de cámara (para órbita simple)
static float angleX = 25.0f;
static float angleY = -45.0f;
static float distance = 30.0f;
static int lastMouseX, lastMouseY;
static bool mouseLeftDown = false;

// IDs de texturas
GLuint texGrass, texBrick, texWood, texLeaf;

// ==================== Funciones para crear texturas ====================

// Genera una textura de pasto (verde con motas)
void makeGrassTexture() {
    const int size = 64;
    unsigned char data[size][size][3];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int r = 80 + (rand() % 40);
            int g = 150 + (rand() % 60);
            int b = 60 + (rand() % 30);
            if ((i/8 + j/8) % 2 == 0) { // pequeños patrones
                r = r * 0.8; g = g * 0.9;
            }
            data[i][j][0] = r;
            data[i][j][1] = g;
            data[i][j][2] = b;
        }
    }
    glGenTextures(1, &texGrass);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// Textura de ladrillos
void makeBrickTexture() {
    const int w = 64, h = 64;
    unsigned char data[w][h][3];
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            int brickRow = j / 16;      // cada ladrillo de 16 píxeles de alto
            int offset = (brickRow % 2) * 16; // desplazamiento para efecto de trabazón
            int brickCol = (i + offset) / 32;
            bool isMortar = ((i + offset) % 32 < 2) || (j % 16 < 2);
            if (isMortar) {
                data[i][j][0] = 200; data[i][j][1] = 200; data[i][j][2] = 200;
            } else {
                data[i][j][0] = 180; data[i][j][1] = 60; data[i][j][2] = 50;
                // añadir variación
                if ((i+j) % 3 == 0) { data[i][j][0] *= 0.9; data[i][j][2] *= 0.9; }
            }
        }
    }
    glGenTextures(1, &texBrick);
    glBindTexture(GL_TEXTURE_2D, texBrick);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

// Textura de madera (anillos concéntricos)
void makeWoodTexture() {
    const int size = 64;
    unsigned char data[size][size][3];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            float dx = (i - size/2.0f) / (size/2.0f);
            float dy = (j - size/2.0f) / (size/2.0f);
            float r = sqrt(dx*dx + dy*dy);
            int ring = (int)(r * 12) % 2;
            if (ring == 0) {
                data[i][j][0] = 160; data[i][j][1] = 100; data[i][j][2] = 50;
            } else {
                data[i][j][0] = 100; data[i][j][1] = 60; data[i][j][2] = 30;
            }
        }
    }
    glGenTextures(1, &texWood);
    glBindTexture(GL_TEXTURE_2D, texWood);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// Textura de hojas (verde con puntos claros)
void makeLeafTexture() {
    const int size = 64;
    unsigned char data[size][size][3];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int r = 20 + rand() % 40;
            int g = 100 + rand() % 80;
            int b = 20 + rand() % 40;
            if ((i+j) % 10 < 2) { r = 200; g = 220; b = 100; } // motas claras
            data[i][j][0] = r;
            data[i][j][1] = g;
            data[i][j][2] = b;
        }
    }
    glGenTextures(1, &texLeaf);
    glBindTexture(GL_TEXTURE_2D, texLeaf);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

// ==================== Configuración de iluminación y materiales ====================

void initLighting() {
    // Luz ambiental tenue
    GLfloat ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);
    
    // Luz direccional (como sol)
    GLfloat diffuseLight[] = { 0.8f, 0.8f, 0.7f, 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    GLfloat lightPos[] = { 10.0f, 20.0f, 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glEnable(GL_LIGHT0);
    
    // Habilitar iluminación y cálculos de material
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);    // Para objetos sin textura asignar color por glColor3f
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);
}

// Función para establecer material (para objetos sin textura)
void setMaterial(GLfloat r, GLfloat g, GLfloat b, GLfloat shininess = 32.0f) {
    GLfloat mat_ambient[] = { r*0.3f, g*0.3f, b*0.3f, 1.0f };
    GLfloat mat_diffuse[] = { r, g, b, 1.0f };
    GLfloat mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
}

// ==================== Dibujo de objetos ====================

// Terreno (40x40, centrado en origen, con textura de pasto)
void drawTerrain() {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texGrass);
    glColor3f(1,1,1);
    
    // Se dibuja como un solo quad grande con muchas repeticiones de textura
    // pero para mejor efecto de luz, podemos usar varios quads o uno grande.
    // Usamos un quad con subdivisiones para dar sensación de terreno (plano).
    glBegin(GL_QUADS);
    glNormal3f(0,1,0); // normal hacia arriba
    glTexCoord2f(0, 0); glVertex3f(-20, 0, -20);
    glTexCoord2f(8, 0); glVertex3f( 20, 0, -20);
    glTexCoord2f(8, 8); glVertex3f( 20, 0,  20);
    glTexCoord2f(0, 8); glVertex3f(-20, 0,  20);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

// Casa: cubo con textura de ladrillos + techo piramidal rojo
void drawHouse() {
    // Paredes (cubo de 4x4x4, base en y=0)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texBrick);
    glColor3f(1,1,1);
    
    // Cara delantera (z = -2)
    glBegin(GL_QUADS);
    glNormal3f(0,0,-1);
    glTexCoord2f(0,0); glVertex3f(-2, 0, -2);
    glTexCoord2f(2,0); glVertex3f( 2, 0, -2);
    glTexCoord2f(2,2); glVertex3f( 2, 4, -2);
    glTexCoord2f(0,2); glVertex3f(-2, 4, -2);
    glEnd();
    
    // Cara trasera (z = 2)
    glBegin(GL_QUADS);
    glNormal3f(0,0,1);
    glTexCoord2f(0,0); glVertex3f(-2, 0, 2);
    glTexCoord2f(2,0); glVertex3f( 2, 0, 2);
    glTexCoord2f(2,2); glVertex3f( 2, 4, 2);
    glTexCoord2f(0,2); glVertex3f(-2, 4, 2);
    glEnd();
    
    // Cara izquierda (x = -2)
    glBegin(GL_QUADS);
    glNormal3f(-1,0,0);
    glTexCoord2f(0,0); glVertex3f(-2, 0, -2);
    glTexCoord2f(2,0); glVertex3f(-2, 0,  2);
    glTexCoord2f(2,2); glVertex3f(-2, 4,  2);
    glTexCoord2f(0,2); glVertex3f(-2, 4, -2);
    glEnd();
    
    // Cara derecha (x = 2)
    glBegin(GL_QUADS);
    glNormal3f(1,0,0);
    glTexCoord2f(0,0); glVertex3f(2, 0, -2);
    glTexCoord2f(2,0); glVertex3f(2, 0,  2);
    glTexCoord2f(2,2); glVertex3f(2, 4,  2);
    glTexCoord2f(0,2); glVertex3f(2, 4, -2);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    
    // Techo (pirámide de 4 caras, color rojo, material)
    setMaterial(0.8f, 0.2f, 0.1f, 60.0f);
    glBegin(GL_TRIANGLES);
    // Cara frontal (z = -2.2, pico en y=5)
    glNormal3f(0, 0.6f, -0.8f);
    glVertex3f(0, 5, 0);
    glVertex3f(-2.5f, 4, -2.2f);
    glVertex3f( 2.5f, 4, -2.2f);
    // Cara trasera
    glNormal3f(0, 0.6f, 0.8f);
    glVertex3f(0, 5, 0);
    glVertex3f( 2.5f, 4,  2.2f);
    glVertex3f(-2.5f, 4,  2.2f);
    // Cara izquierda
    glNormal3f(-0.8f, 0.6f, 0);
    glVertex3f(0, 5, 0);
    glVertex3f(-2.5f, 4, -2.2f);
    glVertex3f(-2.5f, 4,  2.2f);
    // Cara derecha
    glNormal3f(0.8f, 0.6f, 0);
    glVertex3f(0, 5, 0);
    glVertex3f( 2.5f, 4,  2.2f);
    glVertex3f( 2.5f, 4, -2.2f);
    glEnd();
    
    // Puerta (un rectángulo marrón)
    setMaterial(0.5f, 0.3f, 0.1f, 40);
    glBegin(GL_QUADS);
    glNormal3f(0,0,-1);
    glVertex3f(-0.8f, 0, -2.01f);
    glVertex3f( 0.8f, 0, -2.01f);
    glVertex3f( 0.8f, 2, -2.01f);
    glVertex3f(-0.8f, 2, -2.01f);
    glEnd();
}

// Árbol: tronco cilíndrico (prisma) con textura de madera, copa esférica con textura de hojas
void drawTree(float x, float z) {
    glPushMatrix();
    glTranslatef(x, 0, z);
    
    // Tronco (altura 2, radio 0.5)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texWood);
    glColor3f(1,1,1);
    // Usamos un cilindro aproximado con un prisma de 8 lados
    const int segments = 12;
    float radius = 0.5f;
    float height = 2.0f;
    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float xc = cos(angle) * radius;
        float zc = sin(angle) * radius;
        float tex_u = i / (float)segments;
        glNormal3f(xc, 0, zc);
        glTexCoord2f(tex_u, 0); glVertex3f(xc, 0, zc);
        glTexCoord2f(tex_u, 1); glVertex3f(xc, height, zc);
    }
    glEnd();
    // Tapas (opcional)
    glDisable(GL_TEXTURE_2D);
    setMaterial(0.6f, 0.4f, 0.2f);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -1, 0);
    glVertex3f(0, 0, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex3f(cos(angle)*radius, 0, sin(angle)*radius);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 1, 0);
    glVertex3f(0, height, 0);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        glVertex3f(cos(angle)*radius, height, sin(angle)*radius);
    }
    glEnd();
    
    // Copa (esfera achatada con textura de hojas)
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texLeaf);
    glColor3f(1,1,1);
    glTranslatef(0, height + 0.2f, 0);
    // Esfera con textura (glutSolidSphere no aplica textura fácil, usamos un quadric)
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 0.9f, 20, 20);
    gluDeleteQuadric(quad);
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

// ==================== Control de cámara ====================

void updateCamera() {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    float radX = angleX * M_PI / 180.0f;
    float radY = angleY * M_PI / 180.0f;
    float cx = distance * cos(radX) * cos(radY);
    float cy = distance * sin(radX);
    float cz = distance * cos(radX) * sin(radY);
    gluLookAt(cx, cy, cz, 0, 2, 0, 0, 1, 0);
}

void mouseMotion(int x, int y) {
    if (mouseLeftDown) {
        angleY += (x - lastMouseX) * 0.5f;
        angleX += (y - lastMouseY) * 0.5f;
        if (angleX > 80) angleX = 80;
        if (angleX < 5) angleX = 5;
        lastMouseX = x;
        lastMouseY = y;
        glutPostRedisplay();
    }
}

void mouseButton(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastMouseX = x;
            lastMouseY = y;
        } else {
            mouseLeftDown = false;
        }
    }
}

void mouseWheel(int wheel, int direction, int x, int y) {
    distance -= direction * 1.5f;
    if (distance < 8) distance = 8;
    if (distance > 50) distance = 50;
    glutPostRedisplay();
}

// ==================== Display principal ====================

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    updateCamera();
    
    // Terreno
    drawTerrain();
    
    // Casa (ubicada en (-5, 0, -3))
    glPushMatrix();
    glTranslatef(-5, 0, -3);
    drawHouse();
    glPopMatrix();
    
    // Árbol cerca de la casa (posiciones relativas)
    drawTree(-2, -1);
    drawTree(-8, -2);
    drawTree(-4, 3);   // otro árbol
    
    // Opcional: un par de árboles más para decorar
    drawTree(5, 4);
    drawTree(3, -6);
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)w / (double)h, 0.5, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void init() {
    glClearColor(0.6f, 0.8f, 1.0f, 1.0f); // cielo azul claro
    initLighting();
    
    // Generar texturas
    makeGrassTexture();
    makeBrickTexture();
    makeWoodTexture();
    makeLeafTexture();
    
    // Configurar materiales por defecto para objetos no texturizados
    glEnable(GL_NORMALIZE);
}

// ==================== Main ====================

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Escena 3D - Casa, Árboles, Terreno con Texturas");
    
    init();
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMouseWheelFunc(mouseWheel);
    
    glutMainLoop();
    return 0;
}