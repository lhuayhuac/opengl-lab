#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

using namespace std;

void dibujarCuadrado(float x, float y, float lado) {
    glBegin(GL_LINE_LOOP);
    glVertex2f(x, y);                 
    glVertex2f(x + lado, y);          
    glVertex2f(x + lado, y + lado);   
    glVertex2f(x, y + lado);          
    glEnd();
}

void dibujarPoligono( float x, float y, float radio, int num_segmentos){
    glBegin(GL_LINE_LOOP);
    for(int i = 0; i < num_segmentos; i++){
        float theta = 2.0f * 3.1415926f * float(i) / float(num_segmentos);
        float dx = radio * cosf(theta);
        float dy = radio * sinf(theta);
        glVertex2f(x + dx, y + dy);
    }
    glEnd();

}

void dibujarSecuenciaCirculos(float x, float y, float radioInicial, int num_circulos, float factor_reduccion) { 
    for (int i = 0; i < num_circulos; i++) {
        float radioActual = radioInicial * (factor_reduccion * i);
        dibujarPoligono(x, y, radioActual, 34);
    }
}

int main(){
    if (!glfwInit()) {
        cerr << "Error al inicializar GLFW" << endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Dibujar Cuadrado y Poligono", NULL, NULL);
    if (!window) {
        cerr << "Error al crear la ventana" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cerr << "Error al inicializar GLAD" << endl;
        return -1;
    }

    glViewport(0, 0, 400, 400);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        //dibujarCuadrado(-0.5f, -0.5f, 1.0f);
        dibujarPoligono(-0.5f, -0.5f, 0.3f, 34);
        dibujarSecuenciaCirculos(-0.5f, -0.5f, 0.3f, 5, 0.8f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}