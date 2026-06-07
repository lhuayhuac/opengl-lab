#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
void setIdentity(float* mat) {
    for (int i = 0; i < 16; ++i) mat[i] = 0.0f;
    mat[0] = mat[5] = mat[10] = mat[15] = 1.0f;
}
void copyMat(float* dest, const float* src) {
    for (int i = 0; i < 16; ++i) dest[i] = src[i];
}
void multiplyMat(const float* A, const float* B, float* C) {
    for (int c = 0; c < 4; ++c) {
        for (int r = 0; r < 4; ++r) {
            C[c * 4 + r] =
                A[0 * 4 + r] * B[c * 4 + 0] +
                A[1 * 4 + r] * B[c * 4 + 1] +
                A[2 * 4 + r] * B[c * 4 + 2] +
                A[3 * 4 + r] * B[c * 4 + 3];
        }
    }
}
void translate(float* mat, float x, float y, float z) {
    float T[16];
    setIdentity(T);
    T[12] = x;
    T[13] = y;
    T[14] = z;
    float temp[16];
    multiplyMat(mat, T, temp);
    copyMat(mat, temp);
}
void scaleMat(float* mat, float x, float y, float z) {
    float S[16];
    setIdentity(S);
    S[0] = x;
    S[5] = y;
    S[10] = z;
    float temp[16];
    multiplyMat(mat, S, temp);
    copyMat(mat, temp);
}
void rotateZ(float* mat, float angle) {
    float R[16];
    setIdentity(R);
    float c = cosf(angle);
    float s = sinf(angle);
    R[0] = c;
    R[1] = s;
    R[4] = -s;
    R[5] = c;
    float temp[16];
    multiplyMat(mat, R, temp);
    copyMat(mat, temp);
}
void ortho(float left, float right,
           float bottom, float top,
           float nearVal, float farVal,
           float* out) {
    setIdentity(out);
    out[0] = 2.0f / (right - left);
    out[5] = 2.0f / (top - bottom);
    out[10] = -2.0f / (farVal - nearVal);
    out[12] = -(right + left) / (right - left);
    out[13] = -(top + bottom) / (top - bottom);
    out[14] = -(farVal + nearVal) / (farVal - nearVal);
}
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform mat4 model;
void main() {
    gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

bool facingRight = true;
float robotX = 0.0f;
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        facingRight = true;
        robotX += 0.003f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        facingRight = false;
        robotX -= 0.003f;
    }
}
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    return shader;
}
void DrawRect(unsigned int shaderProgram,unsigned int VAO,float* model,float r, float g, float b) {
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"),1,GL_FALSE,model);
    glBindVertexArray(VAO);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"),r, g, b);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"),0.0f, 0.0f, 0.0f);
    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
}
int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window =glfwCreateWindow(800, 600, "Robot Minecraft", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    unsigned int vertexShader =compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader =compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    float vertices[] = {0.5f,  0.5f,0.5f, -0.5f,-0.5f, -0.5f,-0.5f,  0.5f};
    unsigned int indices[] = {0, 1, 3,1, 2, 3};
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2 * sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect = (float)width / (float)height;
        float proj[16];
        ortho(-aspect, aspect,-1.0f, 1.0f,-1.0f, 1.0f,proj);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"),1,GL_FALSE,proj);
        float swingAngle = 0.0f;
        bool walking = false;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            walking = true;
        }
        if (walking) {
            float time = glfwGetTime();
            swingAngle = sinf(time * 5.0f) * 0.7f;
        }
        float root[16];
        setIdentity(root);
        translate(root, robotX, 0.0f, 0.0f);
        float dir = facingRight ? 1.0f : -1.0f;
        scaleMat(root, dir, 1.0f, 1.0f);
        float limbW = 0.2f;
        float limbH = 0.6f;
        float rR = 0.70f;
        float rG = 0.82f;
        float rB = 0.95f;
        float sR = 0.55f;
        float sG = 0.68f;
        float sB = 0.85f;
        float backArm[16];
        copyMat(backArm, root);
        translate(backArm, 0.0f, 0.3f, 0.0f);
        rotateZ(backArm, -swingAngle);
        translate(backArm, 0.0f, -0.25f, 0.0f);
        scaleMat(backArm, limbW, limbH, 1.0f);
        DrawRect(shaderProgram, VAO, backArm,sR, sG, sB);
        float backLeg[16];
        copyMat(backLeg, root);
        translate(backLeg, 0.0f, -0.35f, 0.0f);
        rotateZ(backLeg, swingAngle);
        translate(backLeg, 0.0f, -0.3f, 0.0f);
        scaleMat(backLeg, limbW, limbH, 1.0f);
        DrawRect(shaderProgram, VAO, backLeg,sR, sG, sB);
        float torso[16];
        copyMat(torso, root);
        scaleMat(torso, 0.4f, 0.7f, 1.0f);
        DrawRect(shaderProgram, VAO, torso,rR, rG, rB);
        float head[16];
        copyMat(head, root);
        translate(head, 0.0f, 0.525f, 0.0f);
        scaleMat(head, 0.35f, 0.35f, 1.0f);
        DrawRect(shaderProgram, VAO, head,rR, rG, rB);
        float frontLeg[16];
        copyMat(frontLeg, root);
        translate(frontLeg, 0.0f, -0.35f, 0.0f);
        rotateZ(frontLeg, -swingAngle);
        translate(frontLeg, 0.0f, -0.3f, 0.0f);
        scaleMat(frontLeg, limbW, limbH, 1.0f);
        DrawRect(shaderProgram, VAO, frontLeg,rR, rG, rB);
        float frontArm[16];
        copyMat(frontArm, root);
        translate(frontArm, 0.0f, 0.3f, 0.0f);
        rotateZ(frontArm, swingAngle);
        translate(frontArm, 0.0f, -0.25f, 0.0f);
        scaleMat(frontArm, limbW, limbH, 1.0f);
        DrawRect(shaderProgram, VAO, frontArm,rR, rG, rB);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}