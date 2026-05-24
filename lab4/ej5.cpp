#include <windows.h>
#include <GL/freeglut.h>

float rotX = 20.0f;
float rotY = -30.0f;
float zoom = -15.0f;
int lastX, lastY;
bool mouseLeftDown = false;
void drawAxes() {
    glLineWidth(3);
    glBegin(GL_LINES);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(5,0,0);
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,5,0);
    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,5);
    glEnd();
}
void drawScene() {
    glPushMatrix();
    glTranslatef(-3,0,0);
    glColor3f(1,1,0);
    glutSolidCube(2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(3,0,0);
    glColor3f(0,1,1);
    glutSolidSphere(1.2,30,30);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0,0,-3);
    glColor3f(1,0,1);
    glutSolidTorus(0.4,1,30,30);
    glPopMatrix();
}
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    glTranslatef(0,0,zoom);
    glRotatef(rotX,1,0,0);
    glRotatef(rotY,0,1,0);
    drawAxes();
    drawScene();
    glutSwapBuffers();
}
void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON) {
        if(state == GLUT_DOWN) {
            mouseLeftDown = true;
            lastX = x;
            lastY = y;
        }
        else {
            mouseLeftDown = false;
        }
    }
    if(button == 3) {
        zoom += 0.5f;
    }
    if(button == 4) {
        zoom -= 0.5f;
    }
    glutPostRedisplay();
}

void motion(int x, int y) {
    if(mouseLeftDown) {
        rotY += (x - lastX);
        rotX += (y - lastY);
        lastX = x;
        lastY = y;
        glutPostRedisplay();
    }
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
    glutInitWindowSize(800,600);
    glutCreateWindow("Trackball Camera");
    init();
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutMainLoop();
    return 0;
}