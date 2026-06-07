#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

void drawCircle(float r) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(0,0);
        for(int i = 0; i <= 50; i++) {
            float angle = 2.0f * M_PI * i / 50;
            glVertex2f(cos(angle)*r,sin(angle)*r);
        }
    glEnd();
}

int main() {
    glfwInit();
    GLFWwindow* window =glfwCreateWindow(800,600,"Sin Push Pop",NULL,NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-15,15,-15,15,-1,1);
    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        float t = glfwGetTime();
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        // SOL
        glColor3f(1,1,0);
        drawCircle(1);
        // TIERRA
        glRotatef(t*30,0,0,1);
        glTranslatef(6,0,0);
        glColor3f(0,0,1);
        drawCircle(0.5);
        // LUNA
        glRotatef(t*100,0,0,1);
        glTranslatef(2,0,0);
        glColor3f(1,1,1);
        drawCircle(0.2);
        // PLANETA
        glRotatef(t*15,0,0,1);
        glTranslatef(-10,0,0);
        glColor3f(1,0,0);
        drawCircle(0.7);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}