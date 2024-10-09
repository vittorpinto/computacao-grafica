import sys
import math
from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *

# Constantes
MAXVERTEXS = 30
NPOLYGON = 6
PHI = 3.141572

# Tipos de curvas
HERMITE = 1
BEZIER = 2
BSPLINE = 3
CATMULLR = 4

# Tipos de transformações
TRANSLACAO = 1
ROTACAO = 2
SCALA = 3
CISALHA = 4
ESPELHO = 5

# Variáveis globais
doubleBuffer = True
ptsCurva = [[0.0, 0.0, 0.0] for _ in range(MAXVERTEXS)]
ptsContrle = [[0.0, 0.0, 0.0] for _ in range(MAXVERTEXS)]
nPtsCtrole, nPtsCurva = 0, 0
jaCurva = False
ptoSelect = -1
tipoCurva = 0
tipoTransforma = 0
windW, windH = 600 // 2, 500 // 2
lastX, lastY = 0, 0

# Matrizes de curvas e transformações
M = [[0.0] * 4 for _ in range(4)]
Mh = [[2.0, -2.0, 1.0, 1.0],
      [-3.0, 3.0, -2.0, -1.0],
      [0.0, 0.0, 1.0, 0.0],
      [1.0, 0.0, 0.0, 0.0]]

Mb = [[-1.0, 3.0, -3.0, 1.0],
      [3.0, -6.0, 3.0, 0.0],
      [-3.0, 3.0, 0.0, 0.0],
      [1.0, 0.0, 0.0, 0.0]]

Ms = [[1.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0],
      [0.0, 0.0, 0.0, 1.0]]

Mc = [[1.0, 0.0, 0.0, 0.0],
      [0.0, 1.0, 0.0, 0.0],
      [0.0, 0.0, 1.0, 0.0],
      [0.0, 0.0, 0.0, 1.0]]

MCor = [[1.0, 0.5, 0.0],
        [0.8, 0.7, 0.5],
        [0.5, 0.5, 0.5],
        [0.5, 1.0, 0.5],
        [0.5, 0.5, 1.0],
        [1.0, 0.5, 1.0],
        [0.0, 0.0, 1.0],
        [0.0, 1.0, 0.0],
        [1.0, 0.0, 0.0]]

def init():
    global nPtsCtrole, nPtsCurva, jaCurva
    nPtsCtrole = 0
    nPtsCurva = 0
    jaCurva = False
    for i in range(MAXVERTEXS):
        ptsContrle[i] = [0.0, 0.0, 0.0]
        ptsCurva[i] = [0.0, 0.0, 0.0]

def ptoCurva(t, j, pp):
    global tipoCurva
    pp[0] = pp[1] = pp[2] = 0.0
    ptsCont = [[0.0, 0.0, 0.0] for _ in range(4)]

    for i in range(4):
        ji = (j + i) % nPtsCtrole
        ptsCont[i] = ptsContrle[ji]

    if tipoCurva == HERMITE:
        ptsCont[j + 1] = ptsContrle[j + 3]
        ptsCont[j + 2] = [ptsContrle[j][0] - ptsContrle[j + 1][0],
                          ptsContrle[j][1] - ptsContrle[j + 1][1],
                          ptsContrle[j][2] - ptsContrle[j + 1][2]]
        ptsCont[j + 3] = [ptsContrle[j + 3][0] - ptsContrle[j + 2][0],
                          ptsContrle[j + 3][1] - ptsContrle[j + 2][1],
                          ptsContrle[j + 3][2] - ptsContrle[j + 2][2]]

    for i in range(4):
        cc = t**3 * M[0][i] + t**2 * M[1][i] + t * M[2][i] + M[3][i]
        pp[0] += cc * ptsCont[i][0]
        pp[1] += cc * ptsCont[i][1]
        pp[2] += cc * ptsCont[i][2]

def coord_line():
    glLineWidth(1)
    glColor3f(1.0, 0.0, 0.0)
    glBegin(GL_LINE_STRIP)
    glVertex2f(-windW, 0)
    glVertex2f(windW, 0)
    glEnd()

    glBegin(GL_LINE_STRIP)
    glVertex2f(0, -windH)
    glVertex2f(0, windH)
    glEnd()

def verticesDraw(n, vertices, cr, cg, cb):
    glColor3f(cb, cg, cr)
    glPointSize(3)
    glBegin(GL_POINTS)
    for i in range(n):
        glVertex2f(vertices[i][0], vertices[i][1])
    glEnd()

def Poligono(n, vertices, cb, cg, cr):
    glColor3f(cb, cg, cr)
    glBegin(GL_LINE_STRIP)
    for i in range(n):
        glVertex2fv(vertices[i])
    glEnd()

def geraCurva(j):
    global nPtsCurva, jaCurva
    t = 0.0
    dt = 1.0 / (MAXVERTEXS - 1)

    if nPtsCtrole > 3:
        for nPtsCurva in range(MAXVERTEXS):
            t = float(nPtsCurva) * dt
            ptoCurva(t, j, ptsCurva[nPtsCurva])
        jaCurva = True

def Draw():
    glClearColor(1.0, 1.0, 1.0, 0.0)
    glClear(GL_COLOR_BUFFER_BIT)

    coord_line()
    verticesDraw(nPtsCtrole, ptsContrle, 0.0, 1.0, 0.0)
    Poligono(nPtsCtrole, ptsContrle, 0.0, 0.0, 0.0)

    if jaCurva:
        j = 0
        while j < nPtsCtrole - 3:
            geraCurva(j)
            c = j % (len(MCor) - 3)
            Poligono(nPtsCurva, ptsCurva, MCor[c][0], MCor[c][1], MCor[c][2])
            j += 1

    glutSwapBuffers() if doubleBuffer else glFlush()

def Reshape(width, height):
    global windW, windH
    windW = width // 2
    windH = height // 2

    glViewport(0, 0, width, height)
    glMatrixMode(GL_PROJECTION)
    glLoadIdentity()
    gluOrtho2D(-windW, windW, -windH, windH)
    glMatrixMode(GL_MODELVIEW)

    Draw()

def createGLUTMenus():
    submenu1 = glutCreateMenu(processMenuCurvas)
    glutAddMenuEntry("Hermite", HERMITE)
    glutAddMenuEntry("Bezier", BEZIER)
    glutAddMenuEntry("B-Spline", BSPLINE)
    glutAddMenuEntry("CatmullR", CATMULLR)

    submenu2 = glutCreateMenu(processMenuTransforma)
    glutAddMenuEntry("Translacao", TRANSLACAO)
    glutAddMenuEntry("Rotacao", ROTACAO)
    glutAddMenuEntry("Scala", SCALA)
    glutAddMenuEntry("Cisalha", CISALHA)
    glutAddMenuEntry("Espelho", ESPELHO)
    glutAddMenuEntry("Manipular Pto", 0)

    menu = glutCreateMenu(processMenuEvents)
    glutAddMenuEntry("Poligono de Control", 1)
    glutAddSubMenu("Tipo de Curva", submenu1)
    glutAddSubMenu("Transformacao", submenu2)
    glutAddMenuEntry("Limpar Tudo...", 2)
    glutAttachMenu(GLUT_RIGHT_BUTTON)

def main():
    glutInitContextVersion(3, 2)  # Defina uma versão compatível
    glutInitContextProfile(GLUT_CORE_PROFILE)  # Força o perfil core
    glutInit(sys.argv)
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE)
    glutInitWindowSize(600, 500)
    glutCreateWindow("Basic Program Using PyOpenGL and GLUT")

    init()
    glutReshapeFunc(Reshape)
    glutDisplayFunc(Draw)
    createGLUTMenus()
    glutMainLoop()

if __name__ == "__main__":
    main()
