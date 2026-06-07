#include <glad/glad.h>
#include <GLFW/glfw3.h>

float alpha = 0.0f;
float beta = 0.0f;
float pinzaY = 0.0f;

void drawBase() {
    glColor3f(0.6f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-2, -1);
        glVertex2f( 2, -1);
        glVertex2f( 2,  0);
        glVertex2f(-2,  0);
    glEnd();
}

void drawBrazo() {
    glColor3f(0.8f, 0.8f, 1.0f);
    glBegin(GL_QUADS);
        glVertex2f(-0.5, 0);
        glVertex2f( 0.5, 0);
        glVertex2f( 0.5, 5);
        glVertex2f(-0.5, 5);
    glEnd();
}

void drawAntebrazo() {
    glColor3f(0.8f, 1.0f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(0.4, -0.2);
        glVertex2f(5,   -0.2);
        glVertex2f(5,    0.2);
        glVertex2f(0.4,  0.2);
    glEnd();
}

void drawPinza(float desplazamiento) {
    glColor3f(1.0f, 0.7f, 0.7f);
    glBegin(GL_LINES);
        glVertex2f(0, 0);
        glVertex2f(0, -4);
    glEnd();
    glPushMatrix();
        glTranslatef(0, desplazamiento, 0);
        glBegin(GL_QUADS);
            glVertex2f(-0.5f, -1.2f);
            glVertex2f( 0.5f, -1.2f);
            glVertex2f( 0.5f, -2.0f);
            glVertex2f(-0.5f, -2.0f);
        glEnd();
    glPopMatrix();
}
int main() {
    glfwInit();
    GLFWwindow* window =glfwCreateWindow(800, 600, "Brazo Robotico", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-15, 15, -10, 15, -1, 1);
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            alpha += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            alpha -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            beta += 1.0f;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            beta -= 1.0f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            pinzaY += 0.05f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            pinzaY -= 0.05f;
        if (pinzaY > 2.0f)
            pinzaY = 2.0f;
        if (pinzaY < -2.0f)
            pinzaY = -2.0f;
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushMatrix();
            drawBase();
            glRotatef(alpha, 0, 0, 1);
            drawBrazo();
            glTranslatef(0, 5, 0);
            glRotatef(beta, 0, 0, 1);
            drawAntebrazo();
            glPushMatrix();
                glTranslatef(5, 0, 0);
                glRotatef(-(alpha + beta), 0, 0, 1);
                drawPinza(pinzaY);
            glPopMatrix();
        glPopMatrix();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}