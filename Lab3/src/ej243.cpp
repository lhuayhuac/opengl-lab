#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

float angleTeapot = 0;
float angleRombo = 0;
float angleCube = 0;
float posX = 0.0f;
float speedX = 3.0f;
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

void drawRombo() {
    glColor3f(0,1,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(0.0f, 0.6f);
        glVertex2f(0.6f, 0.0f);
        glVertex2f(0.0f,-0.6f);
        glVertex2f(-0.6f,0.0f);
    glEnd();
}

void drawCuadrado() {
    glColor3f(1,0,0);
    glBegin(GL_LINE_LOOP);
        glVertex2f(-1.0f,-1.0f);
        glVertex2f( 1.0f,-1.0f);
        glVertex2f( 1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
    glEnd();
}

void dibujarCirculo(float radio) {
    glColor3f(1,1,1);
    glBegin(GL_LINE_LOOP);
        for (int i = 0; i < 100; i++) {
            float t = 2.0f * 3.1416f * i / 100;
            glVertex2f(radio * cos(t),
                       radio * sin(t));
        }
    glEnd();
}
int main() {
    glfwInit();
    GLFWwindow* window =glfwCreateWindow(800,600,"Ejercicio 3 Homogeneas",NULL,NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-15,15,-10,10,-1,1);
    float lastTime = glfwGetTime();
    while(!glfwWindowShouldClose(window)) {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        glClear(GL_COLOR_BUFFER_BIT);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        posX += speedX * deltaTime;
        if(posX > 8.0f || posX < -8.0f)
            speedX *= -1;
        angleTeapot += 70.0f * deltaTime;
        angleRombo += 210.0f * deltaTime;
        angleCube += 70.0f * deltaTime;
        float radTea =angleTeapot * 3.1416f / 180.0f;
        float radRombo =angleRombo * 3.1416f / 180.0f;
        float radCube =angleCube * 3.1416f / 180.0f;
        glLoadIdentity();
        float matrizCirculo[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,

            posX,
            0,
            0,
            1
        };
        glMultMatrixf(matrizCirculo);
        dibujarCirculo(0.5f);
        glLoadIdentity();
        float teaX =posX + 4*cosf(radTea);
        float teaY =4*sinf(radTea);
        float matrizTetera[16] = {
             cosf(radTea),  sinf(radTea), 0, 0,
            -sinf(radTea),  cosf(radTea), 0, 0,
             0,             0,            1, 0,

             teaX,
             teaY,
             0,
             1
        };
        glMultMatrixf(matrizTetera);
        drawFakeTeapot();
        glLoadIdentity();
        float romboX =teaX + 3*cosf(radRombo);
        float romboY =teaY + 3*sinf(radRombo);
        float matrizRombo[16] = {
             cosf(radRombo),  sinf(radRombo), 0, 0,
            -sinf(radRombo),  cosf(radRombo), 0, 0,
             0,               0,              1, 0,

             romboX,
             romboY,
             0,
             1
        };
        glMultMatrixf(matrizRombo);
        drawRombo();
        glLoadIdentity();
        float y =5*cosf(radCube);
        float matrizCube[16] = {
            1,0,0,0,
            0,1,0,0,
            0,0,1,0,

            posX,
            y,
            0,
            1
        };
        glMultMatrixf(matrizCube);
        drawCuadrado();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}