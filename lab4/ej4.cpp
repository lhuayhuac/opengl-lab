#include <windows.h>
#include <GL/freeglut.h>
#include <cmath>

float angle = 0.0f;
void drawPlanet(float distance, float size, float speed,
                float r, float g, float b) {
    float x = distance * cos(angle * speed);
    float z = distance * sin(angle * speed);
    glPushMatrix();
    glTranslatef(x, 0, z);
    glColor3f(r, g, b);
    glutSolidSphere(size, 30, 30);
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    float earthX = 6 * cos(angle * 0.7f);
    float earthZ = 6 * sin(angle * 0.7f);
    gluLookAt(
        earthX + 10, 6, earthZ + 10,
        earthX, 0, earthZ,
        0, 1, 0
    );
    glPushMatrix();
    glColor3f(1, 1, 0);
    glutSolidSphere(2.0, 40, 40);
    glPopMatrix();
    drawPlanet(3, 0.3, 1.5, 0.7, 0.7, 0.7);
    drawPlanet(4.5, 0.5, 1.0, 1.0, 0.5, 0.0);
    drawPlanet(6, 0.7, 0.7, 0.0, 0.0, 1.0);
    drawPlanet(8, 0.6, 0.5, 1.0, 0.0, 0.0);
    glutSwapBuffers();
}

void update(int value) {
    angle += 0.01f;
    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, 1.33, 1, 200);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sistema Solar con gluLookAt");
    init();
    glutDisplayFunc(display);
    glutTimerFunc(16, update, 0);
    glutMainLoop();
    return 0;
}