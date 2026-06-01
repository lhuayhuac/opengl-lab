#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <memory>
#include <string>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Point2D {
    float x, y;
    Point2D(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}
};

struct Color {
    float r, g, b;
    Color(float r_ = 1, float g_ = 1, float b_ = 1) : r(r_), g(g_), b(b_) {}
};

enum ShapeType { SHAPE_POINT, SHAPE_LINE, SHAPE_POLYLINE, SHAPE_POLYGON };

class Mat3 {
public:
    float m[3][3];
    Mat3() {
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++)
                m[i][j] = (i == j) ? 1.0f : 0.0f; // Identidad
    }

    Mat3 operator*(const Mat3& other) const {
        Mat3 res;
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 3; j++) {
                res.m[i][j] = 0;
                for (int k = 0; k < 3; k++)
                    res.m[i][j] += m[i][k] * other.m[k][j];
            }
        return res;
    }

    Point2D apply(const Point2D& p) const {
        float xr = m[0][0] * p.x + m[0][1] * p.y + m[0][2];
        float yr = m[1][0] * p.x + m[1][1] * p.y + m[1][2];
        float w  = m[2][0] * p.x + m[2][1] * p.y + m[2][2];
        if (w != 0) { xr /= w; yr /= w; }
        return Point2D(xr, yr);
    }

    static Mat3 translation(float dx, float dy) {
        Mat3 res;
        res.m[0][2] = dx;
        res.m[1][2] = dy;
        return res;
    }

    static Mat3 rotation(float angle_deg) {
        float rad = angle_deg * M_PI / 180.0f;
        float c = cos(rad), s = sin(rad);
        Mat3 res;
        res.m[0][0] =  c; res.m[0][1] = -s;
        res.m[1][0] =  s; res.m[1][1] =  c;
        return res;
    }

    static Mat3 scaling(float sx, float sy) {
        Mat3 res;
        res.m[0][0] = sx;
        res.m[1][1] = sy;
        return res;
    }
};

class Shape {
public:
    ShapeType shapeType;
    Color     color;
    Color     fillColor;
    bool      filled;
    bool      selected;

    Shape(ShapeType t)
        : shapeType(t), color(0,0,0), fillColor(0.8f,0.8f,0.8f),
          filled(false), selected(false) {}

    virtual ~Shape() {}

    virtual void    draw() = 0;
    virtual bool    hitTest(float x, float y, float tol = 5.0f) = 0;
    virtual void    translate(float dx, float dy)= 0;
    virtual void    rotate(float angle_deg) = 0;
    virtual void    scale(float sx, float sy)= 0;
    virtual Point2D getCentroid() const = 0;

    virtual void setColor(const Color& c)     { color = c; }
    virtual void setFillColor(const Color& c) { fillColor = c; }
    virtual void setFilled(bool f)            { filled = f; }

    Mat3 buildTransformAroundCentroid(const Mat3& M) const {
        Point2D c = getCentroid();
        return Mat3::translation(c.x, c.y) * M * Mat3::translation(-c.x,-c.y);
    }
};

class PointShape : public Shape {
public:
    Point2D pos;

    PointShape(float x, float y) : Shape(SHAPE_POINT), pos(x, y) {}

    void draw() override {
        if (selected) glColor3f(1, 1, 0); // Amarillo para seleccionado
        else glColor3f(color.r, color.g, color.b);
        glPointSize(7);
        glBegin(GL_POINTS);
        glVertex2f(pos.x, pos.y);
        glEnd();
        glPointSize(1);
    }

    bool hitTest(float x, float y, float tol) override {
        float dx = pos.x - x, dy = pos.y - y;// Distancia al punto
        return sqrtf(dx*dx + dy*dy) <= tol;// dentro de radio de selección
    }

    void translate(float dx, float dy) override {
        Mat3 M = Mat3::translation(dx, dy);// Matriz de traslación
        pos = M.apply(pos);// Aplicar traslación al punto
    }

    void rotate(float angle_deg) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::rotation(angle_deg));
        pos = M.apply(pos);
    }

    void scale(float sx, float sy) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::scaling(sx, sy));
        pos = M.apply(pos);
    }

    Point2D getCentroid() const override { return pos; }
};

class LineShape : public Shape {
public:
    Point2D p1, p2;

    LineShape(float x1, float y1, float x2, float y2)
        : Shape(SHAPE_LINE), p1(x1, y1), p2(x2, y2) {}

    void draw() override {
        if (selected) glColor3f(1, 1, 0);
        else glColor3f(color.r, color.g, color.b);
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2f(p1.x, p1.y);
        glVertex2f(p2.x, p2.y);
        glEnd();
        glLineWidth(1);
    }
    bool hitTest(float x, float y, float tol) override {
        float ax = p2.x - p1.x, ay = p2.y - p1.y;
        float len2 = ax*ax + ay*ay;
        if (len2 == 0) return sqrtf((x-p1.x)*(x-p1.x)+(y-p1.y)*(y-p1.y)) <= tol;
        float t = ((x-p1.x)*ax + (y-p1.y)*ay) / len2;
        t = std::max(0.0f, std::min(1.0f, t));
        float projx = p1.x + t*ax, projy = p1.y + t*ay;
        float dx = x - projx, dy = y - projy;
        return sqrtf(dx*dx + dy*dy) <= tol;
    }
    void translate(float dx, float dy) override {
        Mat3 M = Mat3::translation(dx, dy);
        p1 = M.apply(p1);
        p2 = M.apply(p2);
    }
    void rotate(float angle_deg) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::rotation(angle_deg));
        p1 = M.apply(p1);
        p2 = M.apply(p2);
    }
    void scale(float sx, float sy) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::scaling(sx, sy));
        p1 = M.apply(p1);
        p2 = M.apply(p2);
    }

    Point2D getCentroid() const override {
        return Point2D((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);
    }
};

class PolylineShape : public Shape {
public:
    std::vector<Point2D> points;

    PolylineShape(const std::vector<Point2D>& pts)
        : Shape(SHAPE_POLYLINE), points(pts) {}

    void draw() override {
        if (selected) glColor3f(1, 1, 0);
        else          glColor3f(color.r, color.g, color.b);
        glLineWidth(2);
        glBegin(GL_LINE_STRIP);
        for (auto& p : points) glVertex2f(p.x, p.y);
        glEnd();
        glLineWidth(1);
    }

    bool hitTest(float x, float y, float tol) override {
        for (size_t i = 0; i + 1 < points.size(); i++) {
            float ax = points[i+1].x - points[i].x;
            float ay = points[i+1].y - points[i].y;
            float len2 = ax*ax + ay*ay;
            if (len2 == 0) continue;
            float t = ((x-points[i].x)*ax + (y-points[i].y)*ay) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            float projx = points[i].x + t*ax, projy = points[i].y + t*ay;
            float dx = x - projx, dy = y - projy;
            if (sqrtf(dx*dx + dy*dy) <= tol) return true;
        }
        return false;
    }

    void translate(float dx, float dy) override {
        Mat3 M = Mat3::translation(dx, dy);
        for (auto& p : points) p = M.apply(p);
    }

    void rotate(float angle_deg) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::rotation(angle_deg));
        for (auto& p : points) p = M.apply(p);
    }

    void scale(float sx, float sy) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::scaling(sx, sy));
        for (auto& p : points) p = M.apply(p);
    }

    Point2D getCentroid() const override {
        if (points.empty()) return Point2D(0, 0);
        float cx = 0, cy = 0;
        for (auto& p : points) { cx += p.x; cy += p.y; }
        return Point2D(cx / points.size(), cy / points.size());
    }
};

class PolygonShape : public Shape {
public:
    std::vector<Point2D> points;

    PolygonShape(const std::vector<Point2D>& pts)
        : Shape(SHAPE_POLYGON), points(pts) {
        filled    = false;
        fillColor = Color(0.5f, 0.7f, 0.9f);
        color     = Color(0, 0, 0);
    }

    void draw() override {
        if (points.size() < 3) return;
        if (filled) {
            glColor3f(fillColor.r, fillColor.g, fillColor.b);
            glBegin(GL_POLYGON);
            for (auto& p : points) glVertex2f(p.x, p.y);
            glEnd();
        }
        if (selected) glColor3f(1, 1, 0);
        else          glColor3f(color.r, color.g, color.b);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        for (auto& p : points) glVertex2f(p.x, p.y);
        glEnd();
        glLineWidth(1);
    }
    
    bool hitTest(float x, float y, float tol) override {
        for (size_t i = 0; i < points.size(); i++) {
            size_t j = (i + 1) % points.size();
            float ax = points[j].x - points[i].x;
            float ay = points[j].y - points[i].y;
            float len2 = ax*ax + ay*ay;
            if (len2 == 0) continue;
            float t = ((x-points[i].x)*ax + (y-points[i].y)*ay) / len2;
            t = std::max(0.0f, std::min(1.0f, t));
            float projx = points[i].x + t*ax, projy = points[i].y + t*ay;
            float dx = x - projx, dy = y - projy;
            if (sqrtf(dx*dx + dy*dy) <= tol) return true;
        }
        bool inside = false;
        for (size_t i = 0, j = points.size()-1; i < points.size(); j = i++) {
            if (((points[i].y > y) != (points[j].y > y)) &&
                (x < (points[j].x - points[i].x) * (y - points[i].y) /
                     (points[j].y - points[i].y) + points[i].x))
                inside = !inside;
        }
        return inside;
    }

    void translate(float dx, float dy) override {
        Mat3 M = Mat3::translation(dx, dy);
        for (auto& p : points) p = M.apply(p);
    }

    void rotate(float angle_deg) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::rotation(angle_deg));
        for (auto& p : points) p = M.apply(p);
    }

    void scale(float sx, float sy) override {
        Mat3 M = buildTransformAroundCentroid(Mat3::scaling(sx, sy));
        for (auto& p : points) p = M.apply(p);
    }

    Point2D getCentroid() const override {
        if (points.empty()) return Point2D(0, 0);
        float cx = 0, cy = 0;
        for (auto& p : points) { cx += p.x; cy += p.y; }
        return Point2D(cx / points.size(), cy / points.size());
    }

    void setFilled(bool f)            override { filled = f; }
    void setFillColor(const Color& c) override { fillColor = c; }
};

int windowWidth = 800, windowHeight = 600;

std::vector<std::unique_ptr<Shape>> shapes;
int selectedIndex = -1;

enum ToolMode { TOOL_POINT, TOOL_LINE, TOOL_POLYLINE, TOOL_POLYGON, TOOL_SELECT };
ToolMode currentMode = TOOL_POINT;

bool lineFirstPoint = false;
Point2D lineStart;
std::vector<Point2D> tempPolyPoints;
int currentMouseX = 0, currentMouseY = 0;

int colorIdx = 0;
std::vector<Color> presetColors = {
    Color(1,0,0), Color(0,1,0), Color(0,0,1),
    Color(1,0.5f,0), Color(0.5f,0,0.5f), Color(0,0.5f,0.5f)
};

void nextColor(Shape* sh) {
    if (!sh) return;
    colorIdx = (colorIdx + 1) % (int)presetColors.size();
    sh->setColor(presetColors[colorIdx]);
}

void nextFillColor(PolygonShape* poly) {
    if (!poly) return;
    static int fillIdx = 0;
    fillIdx = (fillIdx + 1) % (int)presetColors.size();
    poly->setFillColor(presetColors[fillIdx]);
}

void nextOutlineColor(PolygonShape* poly) {
    if (!poly) return;
    static int outIdx = 0;
    outIdx = (outIdx + 1) % (int)presetColors.size();
    poly->setColor(presetColors[outIdx]);
}

Point2D screenToWorld(int sx, int sy) {
    return Point2D((float)sx, (float)(windowHeight - sy));
}

void translateSelected(float dx, float dy) {
    if (selectedIndex >= 0 && selectedIndex < (int)shapes.size()) {
        shapes[selectedIndex]->translate(dx, dy);
        glutPostRedisplay();
    }
}

void rotateSelected(float angle) {
    if (selectedIndex >= 0 && selectedIndex < (int)shapes.size()) {
        shapes[selectedIndex]->rotate(angle);
        glutPostRedisplay();
    }
}

void scaleSelected(float sx, float sy) {
    if (selectedIndex >= 0 && selectedIndex < (int)shapes.size()) {
        shapes[selectedIndex]->scale(sx, sy);
        glutPostRedisplay();
    }
}

void pickObject(int x, int y) {
    Point2D world = screenToWorld(x, y);
    int best = -1;
    for (int i = (int)shapes.size() - 1; i >= 0; i--) {
        if (shapes[i]->hitTest(world.x, world.y, 6.0f)) {
            best = i;
            break;
        }
    }
    if (selectedIndex != -1) shapes[selectedIndex]->selected = false;
    selectedIndex = best;
    if (selectedIndex != -1) shapes[selectedIndex]->selected = true;
    glutPostRedisplay();
}

void createInitialScene() {
    shapes.clear();

    shapes.push_back(std::make_unique<PointShape>(100, 100));
    shapes.back()->setColor(Color(1, 0, 0));

    shapes.push_back(std::make_unique<PointShape>(700, 500));
    shapes.back()->setColor(Color(0, 0, 1));

    shapes.push_back(std::make_unique<PointShape>(400, 300));
    shapes.back()->setColor(Color(0, 0.5f, 0.5f));

    shapes.push_back(std::make_unique<LineShape>(150, 150, 250, 200));
    shapes.back()->setColor(Color(0, 1, 0));

    shapes.push_back(std::make_unique<LineShape>(600, 100, 700, 150));
    shapes.back()->setColor(Color(1, 0.5f, 0));

    shapes.push_back(std::make_unique<LineShape>(50, 550, 120, 530));
    shapes.back()->setColor(Color(0.2f, 0.6f, 0.8f));

    std::vector<Point2D> pl1 = {{300,400},{350,350},{400,380},{450,350},{500,400}};
    shapes.push_back(std::make_unique<PolylineShape>(pl1));
    shapes.back()->setColor(Color(0.5f, 0, 0.5f));

    std::vector<Point2D> pl2 = {{550,550},{600,520},{650,550}};
    shapes.push_back(std::make_unique<PolylineShape>(pl2));
    shapes.back()->setColor(Color(0.9f, 0.2f, 0.8f));

    std::vector<Point2D> poly1 = {{200,400},{250,450},{200,500},{150,450}};
    auto p1 = std::make_unique<PolygonShape>(poly1);
    p1->setColor(Color(0, 0, 0));
    p1->setFillColor(Color(1, 0.8f, 0.5f));
    p1->setFilled(true);
    shapes.push_back(std::move(p1));

    std::vector<Point2D> poly2 = {{600,400},{680,420},{650,480},{580,460}};
    auto p2 = std::make_unique<PolygonShape>(poly2);
    p2->setColor(Color(0, 0, 1));
    p2->setFillColor(Color(0.5f, 1, 0.5f));
    p2->setFilled(false);
    shapes.push_back(std::move(p2));

    std::vector<Point2D> poly3 = {{400,100},{450,150},{400,200},{350,150}};
    auto p3 = std::make_unique<PolygonShape>(poly3);
    p3->setColor(Color(0.8f, 0, 0));
    p3->setFillColor(Color(1, 0.5f, 0.5f));
    p3->setFilled(true);
    shapes.push_back(std::move(p3));

    std::vector<Point2D> poly4 = {{50,50},{100,30},{120,80},{70,100}};
    auto p4 = std::make_unique<PolygonShape>(poly4);
    p4->setColor(Color(0.2f, 0.5f, 0.9f));
    p4->setFillColor(Color(0.8f, 0.9f, 0.6f));
    p4->setFilled(true);
    shapes.push_back(std::move(p4));

    selectedIndex = -1;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    for (auto& sh : shapes) sh->draw();

    glColor3f(0, 0, 0);
    glRasterPos2f(10, 20);
    std::string modeStr;
    switch (currentMode) {
        case TOOL_POINT:    modeStr = "MODO: PUNTO (click izq)"; break;
        case TOOL_LINE:     modeStr = "MODO: LINEA (2 clics)"; break;
        case TOOL_POLYLINE: modeStr = "MODO: POLILINEA (clics, F para terminar)"; break;
        case TOOL_POLYGON:  modeStr = "MODO: POLIGONO (clics, F para cerrar)"; break;
        case TOOL_SELECT:   modeStr = "MODO: SELECCION (click objeto)"; break;
    }
    for (char c : modeStr) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(10, 40);
    std::string info = "1-Punto  2-Linea  3-Polilinea  4-Poligono  5-Seleccion | Flechas:mover | R/r:rotar | S/s:escalar | C:color | F:relleno | I:color relleno | O:color borde";
    for (char c : info) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);

    if (selectedIndex != -1) {
        glRasterPos2f(10, 58);
        std::string sel = "Objeto seleccionado ID: " + std::to_string(selectedIndex);
        // Mostrar tipo
        const char* typeNames[] = {"PUNTO","LINEA","POLILINEA","POLIGONO"};
        sel += "  Tipo: ";
        sel += typeNames[shapes[selectedIndex]->shapeType];
        for (char c : sel) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }

    if (currentMode == TOOL_LINE && lineFirstPoint) {
        Point2D mouseWorld = screenToWorld(currentMouseX, currentMouseY);
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        glVertex2f(lineStart.x, lineStart.y);
        glVertex2f(mouseWorld.x, mouseWorld.y);
        glEnd();
    }

    if ((currentMode == TOOL_POLYLINE || currentMode == TOOL_POLYGON) && !tempPolyPoints.empty()) {
        Point2D mouseWorld = screenToWorld(currentMouseX, currentMouseY);
        glColor3f(0.6f, 0.6f, 0.6f);
        glBegin(GL_LINE_STRIP);
        for (auto& p : tempPolyPoints) glVertex2f(p.x, p.y);
        glVertex2f(mouseWorld.x, mouseWorld.y);
        glEnd();
        if (currentMode == TOOL_POLYGON && tempPolyPoints.size() >= 2) {
            glColor3f(0.3f, 0.3f, 0.3f);
            glBegin(GL_LINES);
            glVertex2f(tempPolyPoints.back().x, tempPolyPoints.back().y);
            glVertex2f(tempPolyPoints[0].x, tempPolyPoints[0].y);
            glEnd();
        }
    }

    glutSwapBuffers();
}

void reshape(int w, int h) {
    windowWidth = w; windowHeight = h;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        Point2D world = screenToWorld(x, y);

        if (currentMode == TOOL_POINT) {
            auto newPoint = std::make_unique<PointShape>(world.x, world.y);
            newPoint->setColor(presetColors[colorIdx]);
            shapes.push_back(std::move(newPoint));
            glutPostRedisplay();
        }
        else if (currentMode == TOOL_LINE) {
            if (!lineFirstPoint) {
                lineStart = world;
                lineFirstPoint = true;
            } else {
                auto newLine = std::make_unique<LineShape>(lineStart.x, lineStart.y, world.x, world.y);
                newLine->setColor(presetColors[colorIdx]);
                shapes.push_back(std::move(newLine));
                lineFirstPoint = false;
                glutPostRedisplay();
            }
        }
        else if (currentMode == TOOL_POLYLINE || currentMode == TOOL_POLYGON) {
            tempPolyPoints.push_back(world);
            glutPostRedisplay();
        }
        else if (currentMode == TOOL_SELECT) {
            pickObject(x, y);
        }
    }
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '1': currentMode = TOOL_POINT;    lineFirstPoint = false; tempPolyPoints.clear(); break;
        case '2': currentMode = TOOL_LINE;     lineFirstPoint = false; tempPolyPoints.clear(); break;
        case '3': currentMode = TOOL_POLYLINE; tempPolyPoints.clear(); break;
        case '4': currentMode = TOOL_POLYGON;  tempPolyPoints.clear(); break;
        case '5': currentMode = TOOL_SELECT; break;

        case 'f': case 'F':
            if (currentMode == TOOL_POLYLINE && tempPolyPoints.size() >= 2) {
                auto nl = std::make_unique<PolylineShape>(tempPolyPoints);
                nl->setColor(presetColors[colorIdx]);
                shapes.push_back(std::move(nl));
                tempPolyPoints.clear();
                glutPostRedisplay();
            }
            else if (currentMode == TOOL_POLYGON && tempPolyPoints.size() >= 3) {
                auto np = std::make_unique<PolygonShape>(tempPolyPoints);
                np->setColor(presetColors[colorIdx]);
                np->setFillColor(Color(0.7f, 0.7f, 0.9f));
                np->setFilled(false);
                shapes.push_back(std::move(np));
                tempPolyPoints.clear();
                glutPostRedisplay();
            }

            if (currentMode == TOOL_SELECT && selectedIndex >= 0) {
                auto* poly = dynamic_cast<PolygonShape*>(shapes[selectedIndex].get());
                if (poly) { poly->setFilled(!poly->filled); glutPostRedisplay(); }
            }
            break;

        case 'c': case 'C':
            if (selectedIndex >= 0) { nextColor(shapes[selectedIndex].get()); glutPostRedisplay(); }
            break;

        case 'i': case 'I':
            if (selectedIndex >= 0) {
                auto* poly = dynamic_cast<PolygonShape*>(shapes[selectedIndex].get());
                if (poly) { nextFillColor(poly); glutPostRedisplay(); }
            }
            break;

        case 'o': case 'O':
            if (selectedIndex >= 0) {
                auto* poly = dynamic_cast<PolygonShape*>(shapes[selectedIndex].get());
                if (poly) { nextOutlineColor(poly); glutPostRedisplay(); }
            }
            break;

        case 'r': rotateSelected(-15.0f); break;
        case 'R': rotateSelected( 15.0f); break;
        case 's': scaleSelected(0.9f, 0.9f); break;
        case 'S': scaleSelected(1.1f, 1.1f); break;

        case 27:
            lineFirstPoint = false;
            tempPolyPoints.clear();
            glutPostRedisplay();
            break;
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_LEFT:  translateSelected(-10,  0); break;
        case GLUT_KEY_RIGHT: translateSelected( 10,  0); break;
        case GLUT_KEY_UP:    translateSelected(  0, 10); break;
        case GLUT_KEY_DOWN:  translateSelected(  0,-10); break;
    }
}

void passiveMotion(int x, int y) {
    currentMouseX = x;
    currentMouseY = y;
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Editor Grafico 2D - OpenGL");
    glClearColor(1, 1, 1, 1);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutPassiveMotionFunc(passiveMotion);
    createInitialScene();
    glutMainLoop();
    return 0;
}