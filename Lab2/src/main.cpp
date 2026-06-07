#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

float Vs = 30.0f;
float Vt = 20.0f;

float angleSol = 0;
float angleTierra = 0;
float angleTierraRot = 0;
float angleLuna = 0;
float angleLunaRot = 0;
float angleMarte = 0;
float angleMarteRot = 0;

void drawCircle(float r) {
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 60; i++) {
        float t = 2 * 3.1416f * i / 60;
        glVertex2f(r * cos(t), r * sin(t));
    }
    glEnd();
}

int main() {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "Sistema Solar", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glDisable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-30, 30, -20, 20, -1, 1);

    float lastTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {

        float currentTime = glfwGetTime();
        float dt = currentTime - lastTime;
        lastTime = currentTime;

        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        angleSol += Vs * dt;

        angleTierra += Vt * dt;
        angleTierraRot += 3 * Vs * dt;

        angleLuna += 2 * Vt * dt;
        angleLunaRot += 1.5f * Vs * dt;

        angleMarte += Vt * dt;
        angleMarteRot += Vs * dt;

        glPushMatrix();

            glRotatef(angleSol, 0, 0, 1);

            glColor3f(1.0f, 1.0f, 0.0f);
            drawCircle(4);

            glBegin(GL_LINES);
                glVertex2f(0, 0);
                glVertex2f(4, 0);
            glEnd();

            glPushMatrix();

                glRotatef(angleTierra, 0, 0, 1);
                glTranslatef(10, 0, 0);

                glRotatef(angleTierraRot, 0, 0, 1);

                glColor3f(0.0f, 0.4f, 1.0f);
                drawCircle(2);

                glPushMatrix();

                    glRotatef(angleLuna, 0, 0, 1);
                    glTranslatef(4, 0, 0);

                    glRotatef(angleLunaRot, 0, 0, 1);

                    glColor3f(0.7f, 0.7f, 0.7f);
                    drawCircle(0.8f);

                glPopMatrix();

            glPopMatrix();

            glPushMatrix();

                glRotatef(angleMarte, 0, 0, 1);
                glTranslatef(18, 0, 0);

                glRotatef(angleMarteRot, 0, 0, 1);

                glColor3f(1.0f, 0.2f, 0.2f);
                drawCircle(1.5f);

            glPopMatrix();

        glPopMatrix();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}