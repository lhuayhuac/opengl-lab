#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

void drawCircle(float cx, float cy, float r, int num_segments) {
    glBegin(GL_TRIANGLE_FAN);
        glVertex2f(cx, cy);
        for (int i = 0; i <= num_segments; i++) {
            float theta = 2.0f * M_PI * float(i) / float(num_segments);
            float x = r * cosf(theta);
            float y = r * sinf(theta);
            glVertex2f(cx + x, cy + y);
        }
    glEnd();
}

void drawFakeTeapot() {
    glColor3f(0.55f, 0.38f, 0.09f);
    glBegin(GL_POLYGON);
        for(int i = 0; i < 40; i++) {
            float angle = 2.0f * M_PI * i / 40;
            glVertex2f(cos(angle) * 0.6f,sin(angle) * 0.4f);
        }
    glEnd();

    glBegin(GL_TRIANGLES);
        glVertex2f(0.6f, 0.1f);
        glVertex2f(1.0f, 0.0f);
        glVertex2f(0.6f, -0.1f);
    glEnd();

    glBegin(GL_LINE_LOOP);
        for(int i = 0; i < 40; i++) {
            float angle = 2.0f * M_PI * i / 40;
            glVertex2f(-0.8f + cos(angle) * 0.2f,sin(angle) * 0.25f);
        }
    glEnd();

    glBegin(GL_TRIANGLES);
        glVertex2f(-0.2f, 0.35f);
        glVertex2f(0.2f, 0.35f);
        glVertex2f(0.0f, 0.6f);
    glEnd();
}

int main() {
    glfwInit();
    GLFWwindow* window =glfwCreateWindow(800, 600, "Tetera Orbitando", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10, 10, -10, 10, -1, 1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        float angle = glfwGetTime() * 50.0f;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glColor3f(0.27f, 0.7f, 0.79f);
        drawCircle(0, 0, 0.5f, 60);

        glPushMatrix();
            glRotatef(angle, 0, 0, 1);
            glTranslatef(4, 0, 0);
            glRotatef(-angle, 0, 0, 1);
            drawFakeTeapot();
        glPopMatrix();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}