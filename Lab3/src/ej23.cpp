#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

float angle = 0.0f;
float posX = 0.0f;
float dir = 1.0f;
float scaleValue = 1.0f;
float scaleDir = 1.0f;

void drawCube() {
    glBegin(GL_QUADS);
        glVertex3f(-1, -1,  1);
        glVertex3f( 1, -1,  1);
        glVertex3f( 1,  1,  1);
        glVertex3f(-1,  1,  1);
        glVertex3f(-1, -1, -1);
        glVertex3f(-1,  1, -1);
        glVertex3f( 1,  1, -1);
        glVertex3f( 1, -1, -1);
        glVertex3f(-1, -1, -1);
        glVertex3f(-1, -1,  1);
        glVertex3f(-1,  1,  1);
        glVertex3f(-1,  1, -1);
        glVertex3f(1, -1, -1);
        glVertex3f(1,  1, -1);
        glVertex3f(1,  1,  1);
        glVertex3f(1, -1,  1);
        glVertex3f(-1, 1, -1);
        glVertex3f(-1, 1,  1);
        glVertex3f( 1, 1,  1);
        glVertex3f( 1, 1, -1);
        glVertex3f(-1, -1, -1);
        glVertex3f( 1, -1, -1);
        glVertex3f( 1, -1,  1);
        glVertex3f(-1, -1,  1);
    glEnd();
    glColor3f(0,0,0);
    glBegin(GL_LINES);
        glVertex3f(-1,-1,1); glVertex3f(1,-1,1);
        glVertex3f(1,-1,1);  glVertex3f(1,1,1);
        glVertex3f(1,1,1);   glVertex3f(-1,1,1);
        glVertex3f(-1,1,1);  glVertex3f(-1,-1,1);
        glVertex3f(-1,-1,1); glVertex3f(-1,-1,-1);
        glVertex3f(1,-1,1);  glVertex3f(1,-1,-1);
        glVertex3f(1,1,1);   glVertex3f(1,1,-1);
        glVertex3f(-1,1,1);  glVertex3f(-1,1,-1);
        glVertex3f(-1,-1,-1); glVertex3f(1,-1,-1);
        glVertex3f(1,-1,-1);  glVertex3f(1,1,-1);
        glVertex3f(1,1,-1);   glVertex3f(-1,1,-1);
        glVertex3f(-1,1,-1);  glVertex3f(-1,-1,-1);
    glEnd();
}
int main() {
    glfwInit();
    GLFWwindow* window =glfwCreateWindow(800, 600, "Cubo con rotacion X", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-15, 15, -10, 10, -50, 50);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        angle -= 0.05f;
        posX += 0.005f * dir;
        if (posX > 8.0f || posX < -8.0f)
            dir *= -1.0f;
        scaleValue += 0.0005f * scaleDir;
        if (scaleValue > 2.0f || scaleValue < 0.5f)
            scaleDir *= -1.0f;
        float rad = angle * 3.1416f / 180.0f;
        float sx = scaleValue;
        float sy = scaleValue;
        float sz = scaleValue;
        float matriz[16] = {
            sx * cosf(rad),  0, sx * sinf(rad),  0,
            0,                sy, 0,             0,
        -sz * sinf(rad),  0, sz * cosf(rad),  0,
            posX, 0, 0, 1
        };
        glMultMatrixf(matriz);
        glColor3f(0.4f, 0.7f, 1.0f);
        drawCube();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}