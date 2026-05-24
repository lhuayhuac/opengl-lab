#include <windows.h>
#include <GL/freeglut.h>

float camX = 0.0f;
float camY = 0.0f;
float camZ = 8.0f;
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(
        camX, camY, camZ,
        0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    );
    glPushMatrix();
    glTranslatef(-2, 0, 0);
    glColor3f(0.0f, 0.7f, 1.0f);
    glutSolidCube(1.5);
    glColor3f(0.0f, 0.0f, 0.0f);
    glutWireCube(1.51);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(2, 0, 0);
    glColor3f(1.0f, 0.3f, 0.3f);
    glutSolidSphere(1, 30, 30);
    glColor3f(0.0f, 0.0f, 0.0f);
    glutWireSphere(1.01, 20, 20);
    glPopMatrix();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'a':
            camX -= 0.5f;
            break;
        case 'd':
            camX += 0.5f;
            break;
        case 'q':
            camY += 0.5f;
            break;
        case 'e':
            camY -= 0.5f;
            break;
        case 'w':
            camZ -= 0.5f;
            break;
        case 's':
            camZ += 0.5f;
            break;
    }
    glutPostRedisplay();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1.33, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Camara con gluLookAt");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}