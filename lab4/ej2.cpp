#include <windows.h>
#include <GL/freeglut.h>

int tipoProyeccion = 0;
void configurarProyeccion() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (tipoProyeccion == 0) {
        glOrtho(-6, 6, -6, 6, 1, 20);
    } else {
        gluPerspective(45, 1.33, 1, 20);
    }
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glPushMatrix();
    glTranslatef(-2.0f, 0.0f, -7.0f);
    glutWireCube(1.5);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, -8.0f);
    glutWireSphere(1.0, 20, 20);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(3.0f, 0.0f, -12.0f);
    glutWireTorus(0.3, 1.0, 20, 20);
    glPopMatrix();
    glutSwapBuffers();
}

void teclado(unsigned char key, int x, int y) {
    switch (key) {
    case 'o':
    case 'O':
        tipoProyeccion = 0;
        configurarProyeccion();
        glutPostRedisplay();
        break;
    case 'p':
    case 'P':
        tipoProyeccion = 1;
        configurarProyeccion();
        glutPostRedisplay();
        break;
    }
}

void init() {
    glEnable(GL_DEPTH_TEST);
    configurarProyeccion();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("glOrtho y gluPerspective");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(teclado);
    glutMainLoop();
    return 0;
}