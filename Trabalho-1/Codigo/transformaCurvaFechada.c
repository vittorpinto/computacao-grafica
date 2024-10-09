#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

#define MAX_POINTS 100
#define HERMITE 1
#define BEZIER 2
#define BSPLINE 3   
#define CATMULLR 4
#define TRANSLACAO 1
#define ROTACAO 2
#define SCALA 3
#define CISALHA 4
#define ESPELHO 5

#ifndef M_PI
    #define M_PI 3.14159265358979323846
#endif

typedef struct {
    float x, y;
} Point;

Point controlPoints[MAX_POINTS];
int numPoints = 0;
int curveType = 0;
int transformType = 0;
int selectedPoint = -1;
int polygonClosed = 0;
int windowWidth = 500, windowHeight = 500;
int ignoreClicks = 0;

void applyMatrix(float matrix[3][3], Point* p) {
    float tempX = matrix[0][0] * p->x + matrix[0][1] * p->y + matrix[0][2];
    float tempY = matrix[1][0] * p->x + matrix[1][1] * p->y + matrix[1][2];
    p->x = tempX;
    p->y = tempY;
}

Point calculateCentroid() {
    Point centroid = {0, 0};
    for (int i = 0; i < numPoints; i++) {
        centroid.x += controlPoints[i].x;
        centroid.y += controlPoints[i].y;
    }
    centroid.x /= numPoints;
    centroid.y /= numPoints;
    return centroid;
}

void translate(float dx, float dy) {
    float matrix[3][3] = {
        {1, 0, dx},
        {0, 1, dy},
        {0, 0, 1}
    };
    
    for (int i = 0; i < numPoints; i++) {
        applyMatrix(matrix, &controlPoints[i]);
    }
}

void rotate(float angle) {
    Point centroid = calculateCentroid();
    translate(-centroid.x, -centroid.y);

    float rad = angle * M_PI / 180.0;
    float matrix[3][3] = {
        {cos(rad), -sin(rad), 0},
        {sin(rad), cos(rad), 0},
        {0, 0, 1}
    };
    
    for (int i = 0; i < numPoints; i++) {
        applyMatrix(matrix, &controlPoints[i]);
    }

    translate(centroid.x, centroid.y);
}

void scale(float sx, float sy) {
    Point centroid = calculateCentroid();
    translate(-centroid.x, -centroid.y);

    float matrix[3][3] = {
        {sx, 0, 0},
        {0, sy, 0},
        {0, 0, 1}
    };
    
    for (int i = 0; i < numPoints; i++) {
        applyMatrix(matrix, &controlPoints[i]);
    }

    translate(centroid.x, centroid.y);
}

void shear(float shx, float shy) {
    float matrix[3][3] = {
        {1, shx, 0},
        {shy, 1, 0},
        {0, 0, 1}
    };

    for (int i = 0; i < numPoints; i++) {
        applyMatrix(matrix, &controlPoints[i]);
    }
}

void mirror() {
    float matrix[3][3] = {
        {-1, 0, 0},
        {0, -1, 0},
        {0, 0, 1}
    };

    for (int i = 0; i < numPoints; i++) {
        applyMatrix(matrix, &controlPoints[i]);
    }
}

void drawPoints() {
    glColor3f(1.0, 0.0, 0.0);
    glPointSize(5.0);
    glBegin(GL_POINTS);
    for (int i = 0; i < numPoints; i++) {
        glVertex2f(controlPoints[i].x, controlPoints[i].y);
    }
    glEnd();
}

void drawPolygon() {
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < numPoints; i++) {
        glVertex2f(controlPoints[i].x, controlPoints[i].y);
    }
    if (polygonClosed && numPoints > 2) {
        glVertex2f(controlPoints[0].x, controlPoints[0].y);
    }
    glEnd();
}

void drawBezier() {
    if (numPoints < 4) return;
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (float t = 0.0; t <= 1.0; t += 0.01) {
        float x = 0, y = 0;
        int n = numPoints - 1;
        for (int i = 0; i <= n; i++) {
            float binomial = tgamma(n + 1) / (tgamma(i + 1) * tgamma(n - i + 1));
            float coeff = binomial * pow(t, i) * pow(1 - t, n - i);
            x += coeff * controlPoints[i].x;
            y += coeff * controlPoints[i].y;
        }
        glVertex2f(x, y);
    }
    glEnd();
}

void drawHermite() {
    if (numPoints < 2) return;
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < numPoints - 1; i += 2) {
        Point p0 = controlPoints[i];
        Point p1 = controlPoints[i + 1];
        Point t0 = {100.0, 0.0};
        Point t1 = {100.0, 0.0};

        for (float t = 0; t <= 1.0; t += 0.01) {
            float h1 = 2 * t * t * t - 3 * t * t + 1;
            float h2 = -2 * t * t * t + 3 * t * t;
            float h3 = t * t * t - 2 * t * t + t;
            float h4 = t * t * t - t * t;

            float x = h1 * p0.x + h2 * p1.x + h3 * t0.x + h4 * t1.x;
            float y = h1 * p0.y + h2 * p1.y + h3 * t0.y + h4 * t1.y;
            glVertex2f(x, y);
        }
    }
    glEnd();
}

void drawBSpline() {
    if (numPoints < 4) return;
    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_LINE_STRIP);

    for (int i = 1; i < numPoints - 2; i++) {
        for (float t = 0.0; t <= 1.0; t += 0.01) {
            float t2 = t * t;
            float t3 = t2 * t;

            float b0 = (-t3 + 3 * t2 - 3 * t + 1) / 6.0;
            float b1 = (3 * t3 - 6 * t2 + 4) / 6.0;
            float b2 = (-3 * t3 + 3 * t2 + 3 * t + 1) / 6.0;
            float b3 = t3 / 6.0;

            float x = b0 * controlPoints[i - 1].x +
                      b1 * controlPoints[i].x +
                      b2 * controlPoints[i + 1].x +
                      b3 * controlPoints[i + 2].x;

            float y = b0 * controlPoints[i - 1].y +
                      b1 * controlPoints[i].y +
                      b2 * controlPoints[i + 1].y +
                      b3 * controlPoints[i + 2].y;

            glVertex2f(x, y);
        }
    }

    glEnd();
}

void drawCatmullRom() {
    if (numPoints < 4) return;
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for (int i = 1; i < numPoints - 2; i++) {
        for (float t = 0.0; t <= 1.0; t += 0.01) {
            float t2 = t * t;
            float t3 = t2 * t;

            float x = 0.5 * ((2 * controlPoints[i].x) +
                             (-controlPoints[i - 1].x + controlPoints[i + 1].x) * t +
                             (2 * controlPoints[i - 1].x - 5 * controlPoints[i].x + 4 * controlPoints[i + 1].x - controlPoints[i + 2].x) * t2 +
                             (-controlPoints[i - 1].x + 3 * controlPoints[i].x - 3 * controlPoints[i + 1].x + controlPoints[i + 2].x) * t3);

            float y = 0.5 * ((2 * controlPoints[i].y) +
                             (-controlPoints[i - 1].y + controlPoints[i + 1].y) * t +
                             (2 * controlPoints[i - 1].y - 5 * controlPoints[i].y + 4 * controlPoints[i + 1].y - controlPoints[i + 2].y) * t2 +
                             (-controlPoints[i - 1].y + 3 * controlPoints[i].y - 3 * controlPoints[i + 1].y + controlPoints[i + 2].y) * t3);

            glVertex2f(x, y);
        }
    }
    glEnd();
}

void drawAxes() {
    glColor3f(0.0, 0.0, 0.0);
    glLineWidth(1.5);

    glBegin(GL_LINES);
    glVertex2f(-windowWidth / 2, 0);
    glVertex2f(windowWidth / 2, 0);
    glEnd();

    glBegin(GL_LINES);
    glVertex2f(0, -windowHeight / 2);
    glVertex2f(0, windowHeight / 2);
    glEnd();
}

Point convertMouseToOpenGL(int x, int y) {
    Point p;
    p.x = (float)(x - windowWidth / 2);
    p.y = (float)(windowHeight / 2 - y);
    return p;
}

void applyTransformation() {
    if (selectedPoint == -1 && !polygonClosed) return;

    switch (transformType) {
        case TRANSLACAO:
            translate(10.0, 10.0);
            break;
        case ROTACAO:
            rotate(45.0);
            break;
        case SCALA:
            scale(1.1, 1.1);
            break;
        case CISALHA:
            shear(0.1, 0);
            break;
        case ESPELHO:
            mirror();
            break;
    }

    glutPostRedisplay();
}

void clearAll() {
    numPoints = 0;
    curveType = 0;
    transformType = 0;
    selectedPoint = -1;
    polygonClosed = 0;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawAxes();
    drawPolygon();
    drawPoints();

    switch (curveType) {
        case BEZIER:
            drawBezier();
            break;
        case HERMITE:
            drawHermite();
            break;
        case BSPLINE:
            drawBSpline();
            break;
        case CATMULLR:
            drawCatmullRom();
            break;
    }

    glutSwapBuffers();
}

void mouse(int button, int state, int x, int y) {
    if (ignoreClicks) {
        ignoreClicks = 0;
        return;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        selectedPoint = -1;

        Point worldCoords = convertMouseToOpenGL(x, y);

        for (int i = 0; i < numPoints; i++) {
            float dx = worldCoords.x - controlPoints[i].x;
            float dy = worldCoords.y - controlPoints[i].y;
            if (sqrt(dx * dx + dy * dy) < 10.0) {
                selectedPoint = i;
                break;
            }
        }

        if (selectedPoint == -1 && numPoints < MAX_POINTS) {
            controlPoints[numPoints] = worldCoords;
            numPoints++;
        }

        applyTransformation();
        glutPostRedisplay();
    }
}

void motion(int x, int y) {
    if (selectedPoint >= 0 && selectedPoint < numPoints) {
        Point worldCoords = convertMouseToOpenGL(x, y);
        controlPoints[selectedPoint] = worldCoords;
        glutPostRedisplay();
    }
}

void keyboard(unsigned char key, int x, int y) {
    if (key == 27) {
        exit(0);
    }
}

void curveMenu(int option) {
    ignoreClicks = 1;
    if (option == 0) {
        clearAll();
    } else {
        curveType = option;
    }
    glutPostRedisplay();
}

void transformationMenu(int option) {
    ignoreClicks = 1;
    transformType = option;
    applyTransformation();
    glutPostRedisplay();
}

void polygonMenu(int option) {
    ignoreClicks = 1;
    if (option == 1) {
        polygonClosed = !polygonClosed;
    }
    glutPostRedisplay();
}

void init() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    gluOrtho2D(-windowWidth / 2, windowWidth / 2, -windowHeight / 2, windowHeight / 2);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(windowWidth, windowHeight);
    glutCreateWindow("Curvas e Transformações");

    init();

    int curveMenuId = glutCreateMenu(curveMenu);
    glutAddMenuEntry("Bezier", BEZIER);
    glutAddMenuEntry("Hermite", HERMITE);
    glutAddMenuEntry("B-Spline", BSPLINE);
    glutAddMenuEntry("Catmull-Rom", CATMULLR);
    glutAddMenuEntry("Limpar Pontos", 0);

    int transformMenuId = glutCreateMenu(transformationMenu);
    glutAddMenuEntry("Translação", TRANSLACAO);
    glutAddMenuEntry("Rotação", ROTACAO);
    glutAddMenuEntry("Escala", SCALA);
    glutAddMenuEntry("Cisalhamento", CISALHA);
    glutAddMenuEntry("Espelho", ESPELHO);

    int polygonMenuId = glutCreateMenu(polygonMenu);
    glutAddMenuEntry("Fechar/Desfazer Polígono", 1);

    glutCreateMenu(NULL);
    glutAddSubMenu("Tipo de Curva", curveMenuId);
    glutAddSubMenu("Transformação", transformMenuId);
    glutAddSubMenu("Fechar Polígono", polygonMenuId);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}
