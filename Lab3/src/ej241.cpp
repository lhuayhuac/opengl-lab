#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

float angleValue = 0;
void drawFakeTeapot() {
    glColor3f(0.55f, 0.38f, 0.09f);
    glBegin(GL_POLYGON);
        for(int i = 0; i < 40; i++) {
            float a = 2.0f * 3.1416f * i / 40;
            glVertex2f(cos(a) * 0.6f,
                       sin(a) * 0.4f);
        }
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(0.6f, 0.1f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(0.6f,-0.1f);
    glEnd();
    glBegin(GL_LINE_LOOP);
        for(int i = 0; i < 40; i++) {
            float a = 2.0f * 3.1416f * i / 40;
            glVertex2f(-0.8f + cos(a) * 0.2f,
                        sin(a) * 0.25f);
        }
    glEnd();
    glBegin(GL_TRIANGLES);
        glVertex2f(-0.2f,0.35f);
        glVertex2f( 0.2f,0.35f);
        glVertex2f( 0.0f,0.6f);
    glEnd();
}
void dibujarCirculo(float radio) {
    glColor3f(1,1,1);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 100; i++) {
            float theta = 2.0f * 3.1416f * i / 100;
            float x = radio * cos(theta);
            float y = radio * sin(theta);
            glVertex2f(x, y);
        }
    glEnd();
}

int main() {
    glfwInit();
    GLFWwindow* window =
        glfwCreateWindow(800,600,"Transformaciones Homogeneas",NULL,NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10,10,-10,10,-1,1);
    float lastTime = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        dibujarCirculo(0.5f);
        angleValue += 70.0f * deltaTime;
        float rad = angleValue * 3.1416f / 180.0f;
        float matriz[16] = {
             cosf(rad),  sinf(rad), 0, 0,
            -sinf(rad),  cosf(rad), 0, 0,
             0,          0,         1, 0,
             4*cosf(rad),
             4*sinf(rad),
             0,
             1
        };
        glMultMatrixf(matriz);
        drawFakeTeapot();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}