#include "include/Billboard.h"
#include <GL/freeglut.h>
void Billboard::render(const Vec3& center,
                       float width, float height,
                       const Vec3& camRight, const Vec3& camUp,
                       float alpha,
                       bool  useTexture,
                       float r, float g, float b)
{
    // Mitad del tamaño en cada eje
    Vec3 halfR = camRight * (width  * 0.5f);
    Vec3 halfU = camUp    * (height * 0.5f);

    // Cuatro vértices del quad
    Vec3 v0 = center - halfR + halfU;  // top-left
    Vec3 v1 = center - halfR - halfU;  // bottom-left
    Vec3 v2 = center + halfR - halfU;  // bottom-right
    Vec3 v3 = center + halfR + halfU;  // top-right

    glColor4f(r, g, b, alpha);

    glBegin(GL_QUADS);
        if (useTexture) glTexCoord2f(0.0f, 1.0f);
        glVertex3f(v0.x, v0.y, v0.z);

        if (useTexture) glTexCoord2f(0.0f, 0.0f);
        glVertex3f(v1.x, v1.y, v1.z);

        if (useTexture) glTexCoord2f(1.0f, 0.0f);
        glVertex3f(v2.x, v2.y, v2.z);

        if (useTexture) glTexCoord2f(1.0f, 1.0f);
        glVertex3f(v3.x, v3.y, v3.z);
    glEnd();
}