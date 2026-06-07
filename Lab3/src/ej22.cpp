#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

void setIdentity(float* mat) {
    for (int i = 0; i < 16; ++i) mat[i] = 0.0f;
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
}
void multiplyMat(const float* A, const float* B, float* C) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            C[c * 4 + r] = A[0 * 4 + r] * B[c * 4 + 0] +
                           A[1 * 4 + r] * B[c * 4 + 1] +
                           A[2 * 4 + r] * B[c * 4 + 2] +
                           A[3 * 4 + r] * B[c * 4 + 3];
        }
    }
}
void getTranslationMatrix(float Tx, float Ty, float* outMat) {
    setIdentity(outMat);
    outMat[12] = Tx; 
    outMat[13] = Ty;
}
void getRotationMatrix(float theta, float* outMat) {
    setIdentity(outMat);
    float c = cosf(theta);
    float s = sinf(theta);
    outMat[0] = c;  outMat[1] = s;
    outMat[4] = -s; outMat[5] = c;
}
void getScaleMatrix(float Sx, float Sy, float* outMat) {
    setIdentity(outMat);
    outMat[0] = Sx;
    outMat[5] = Sy;
}
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 transform;
out vec3 vertexColor;

void main() {
    // Multiplicamos el vértice por la matriz de transformación
    gl_Position = transform * vec4(aPos, 0.0, 1.0);
    vertexColor = aColor;
}
)";
const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 800, "Transformaciones Homogeneas", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    float vertices[] = {
         0.0f,  0.5f,   1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,
         0.5f, -0.5f,   0.0f, 0.0f, 1.0f
    };
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    int transformLoc = glGetUniformLocation(shaderProgram, "transform");
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        float time = glfwGetTime();
        float Tx = cosf(time) * 0.5f;
        float Ty = sinf(time) * 0.5f;
        float theta = time * 2.0f;
        float scaleVal = 0.5f + (sinf(time * 3.0f) * 0.25f);
        float Sx = scaleVal;
        float Sy = scaleVal;
        float T[16], R[16], S[16];
        getTranslationMatrix(Tx, Ty, T);
        getRotationMatrix(theta, R);
        getScaleMatrix(Sx, Sy, S);
        float tempTR[16], finalMatrix[16];
        multiplyMat(T, R, tempTR);
        multiplyMat(tempTR, S, finalMatrix);
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, finalMatrix);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}