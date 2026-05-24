//EJERCICIO1 - ORTOGONAL
#include <windows.h>
#include <GL/freeglut.h>
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, -5);
    glutWireCube(2);
    glutSwapBuffers();
}
void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-5,5,-5,5,1,100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Ortografica");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}

//EJERCICIO1 - PERSPECTIVE
#include <windows.h>
#include <GL/freeglut.h>
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0, 0, -5);
    glutWireCube(2);
    glutSwapBuffers();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45, 1, 1, 100);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800,600);
    glutCreateWindow("Perspectiva");
    init();
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}