#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h" 

#define MAXVERTEXS 30
#define HERMITE 1
#define BEZIER 2
#define BSPLINE 3
#define CATMULLR 4

#define PHI 3.141572
 
GLenum doubleBuffer;     
 
typedef struct spts 
{ 
    float v[3]; 
} tipoPto; 

tipoPto pvertex[MAXVERTEXS];  // vetor de MAXVERTEXS de v[3] como espaço máximo

int windW, windH;     // medidas das dimensoes da janela Canvas: W largura  H: altura
int tipoPoligono;     // poligono prenchido, contorno ou pontos nos vértices
int gNumVert = 0;    // número de pontos ou vértices do poligono criado
int jaPoligono = 0;   // modo de representação --> 0: pontos;  1: polígono

float gMatriz[3][3];  // matriz de transformação

int gIndVert = -1;   // índice do vértice selecionado
int gTransform = 0;   // transformação: 0: nula; 1: Translação; 2: Rotação; 3: Escala; 4: Espelha; 5: Cisalha 
float gCen[3];       // centroide do polígono
float gAng = 0.0f;    // ângulo para rotação 

int curveType = 0;  // Tipo de curva selecionada (Bezier, Hermite, etc.)

// Declarações das funções
void Draw(void);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void translate(float dx, float dy);
void rotate(float dx, float dy);
void scale(float dx, float dy);
void shear(float dx, float dy);
void mirror(float dx, float dy);
int clipVertex(int x, int y);
void coord_line(void);
void PolygonDraw(void);
void calCentro(float cc[]);
void operaTransforma(float v[]);

void init(void)
{
	int i;

	jaPoligono = 0;
	gNumVert = 0;   // zero pontos
	tipoPoligono = GL_POINTS;

	gTransform = 0; // transformação Nula
	gIndVert = -1;  // índice do vértice selecionado nulo

	// inicializa com ZERO o vetor pvertex 
	for (i = 0; i < MAXVERTEXS; i++) {
		pvertex[i].v[0] = 0.0f;
		pvertex[i].v[1] = 0.0f;
		pvertex[i].v[2] = 1.0f;
	}
	// calcula o ângulo básico de rotação 
	gAng = (2.0f * PHI) / 360.0f;
}

void matrizIdentidade(void) {
	gMatriz[0][0] = 1.0f; gMatriz[0][1] = 0.0f; gMatriz[0][2] = 0.0f;
	gMatriz[1][0] = 0.0f; gMatriz[1][1] = 1.0f; gMatriz[1][2] = 0.0f;
	gMatriz[2][0] = 0.0f; gMatriz[2][1] = 0.0f; gMatriz[2][2] = 1.0f;
}

// Função que calcula o centroide do polígono
void calCentro(float cc[])
{
    int i;
    cc[0] = cc[1] = cc[2] = 0.0f;
    for (i = 0; i < gNumVert; i++) {
        cc[0] += pvertex[i].v[0];
        cc[1] += pvertex[i].v[1];
        cc[2] += pvertex[i].v[2];
    }
    cc[0] /= gNumVert;
    cc[1] /= gNumVert;
    cc[2] /= gNumVert;
}

// Função que aplica a transformação ao vetor
void operaTransforma(float v[])
{
    float temp[3];
    int i, j;
    for (i = 0; i < 3; i++) {
        temp[i] = 0.0f;
        for (j = 0; j < 3; j++) {
            temp[i] += gMatriz[i][j] * v[j];
        }
    }
    for (i = 0; i < 3; i++) {
        v[i] = temp[i];
    }
}

static void Reshape(int width, int height)
{
	windW = width / 2;
	windH = height / 2;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluOrtho2D(-windW, windW, -windH, windH);

	glMatrixMode(GL_MODELVIEW);
}

static void Key(unsigned char key, int x, int y)
{
	switch (key) 
	{
		case 27:
			exit(0);
	}
}

void coord_line(void)
{
	glLineWidth(1);

	glColor3f(1.0, 0.0, 0.0);

	// linha vertical (eixo Y)
	glBegin(GL_LINES);
	glVertex2f(0, -windH);
	glVertex2f(0, windH);
	glEnd();

	glColor3f(0.0, 1.0, 0.0);

	// linha horizontal (eixo X)
	glBegin(GL_LINES);
	glVertex2f(-windW, 0);
	glVertex2f(windW, 0);
	glEnd();
}

void PolygonDraw(void)
{
	int i;

	glColor3f(0.0, 0.0, 0.0); 

	glPolygonMode(GL_FRONT_AND_BACK, tipoPoligono);

	glBegin(tipoPoligono);
	for (i = 0; i < gNumVert; i++) {
		glVertex2fv(pvertex[i].v);
	}
	glEnd();
}

// Implementação das funções de transformação
void translate(float dx, float dy) {
    int i;
    matrizIdentidade();
    gMatriz[0][2] = dx;
    gMatriz[1][2] = dy;
    for (i = 0; i < gNumVert; i++) {
        operaTransforma(pvertex[i].v);
    }
}

void rotate(float dx, float dy) {
    int i;
    float oo, teta, vc[3];
    calCentro(vc);
    translate(-vc[0], -vc[1]);
    oo = pvertex[gIndVert].v[1] * dx - pvertex[gIndVert].v[0] * dy;
    teta = (oo > 0) ? -gAng : gAng;
    matrizIdentidade();
    gMatriz[0][0] = cos(teta); gMatriz[0][1] = -sin(teta);
    gMatriz[1][0] = sin(teta); gMatriz[1][1] = cos(teta);
    for (i = 0; i < gNumVert; i++) {
        operaTransforma(pvertex[i].v);
    }
    translate(vc[0], vc[1]);
}

void scale(float dx, float dy) {
    int i;
    float sx, sy, vc[3];
    calCentro(vc);
    translate(-vc[0], -vc[1]);
    sx = 1 + dx / pvertex[gIndVert].v[0];
    sy = 1 + dy / pvertex[gIndVert].v[1];
    matrizIdentidade();
    gMatriz[0][0] = sx;
    gMatriz[1][1] = sy;
    for (i = 0; i < gNumVert; i++) {
        operaTransforma(pvertex[i].v);
    }
    translate(vc[0], vc[1]);
}

void mirror(float dx, float dy) {
    int i;
    float mx, my, vc[3];
    calCentro(vc);
    translate(-vc[0], -vc[1]);
    mx = (fabs(dx) > 0.1f) ? -1.0f : 1.0f;
    my = (fabs(dy) > 0.1f) ? -1.0f : 1.0f;
    matrizIdentidade();
    gMatriz[0][0] = mx;
    gMatriz[1][1] = my;
    for (i = 0; i < gNumVert; i++) {
        operaTransforma(pvertex[i].v);
    }
    translate(vc[0], vc[1]);
}

void shear(float dx, float dy) {
    int i;
    float kx, ky, vc[3];
    calCentro(vc);
    translate(-vc[0], -vc[1]);
    kx = dx / pvertex[gIndVert].v[1];
    ky = dy / pvertex[gIndVert].v[0];
    matrizIdentidade();
    gMatriz[0][1] = kx;
    gMatriz[1][0] = ky;
    for (i = 0; i < gNumVert; i++) {
        operaTransforma(pvertex[i].v);
    }
    translate(vc[0], vc[1]);
}

int clipVertex(int x, int y) {
    int i;
    float d;
    gIndVert = -1;
    for (i = 0; i < gNumVert; i++) {
        d = sqrt(pow((pvertex[i].v[0] - x), 2.0) + pow((pvertex[i].v[1] - y), 2.0));
        if (d < 3.0) {
            gIndVert = i;
            break;
        }
    }
    return gIndVert;
}

void Draw(void)
{
	glClearColor(1.0, 1.0, 1.0, 0.0); 
	glClear(GL_COLOR_BUFFER_BIT);

	coord_line();

	PolygonDraw();

	switch (curveType) {
		case BEZIER:
			// Implementação da curva Bezier (adicionar aqui)
			break;
		case HERMITE:
			// Implementação da curva Hermite (adicionar aqui)
			break;
		case BSPLINE:
			// Implementação da curva B-Spline (adicionar aqui)
			break;
		case CATMULLR:
			// Implementação da curva Catmull-Rom (adicionar aqui)
			break;
	}

	if (doubleBuffer) {
		glutSwapBuffers();
	} else {
		glFlush();
	}
}

// Evento de arrasto do mouse
void motion(int x, int y)
{
	float dx, dy;
	if (gIndVert > -1) {
		x = x - windW; y = windH - y;
		dx = x - pvertex[gIndVert].v[0];
		dy = y - pvertex[gIndVert].v[1];

		switch (gTransform) {
			case 1: translate(dx, dy); break;
			case 2: rotate(dx, dy); break;
			case 3: scale(dx, dy); break;
			case 4: mirror(dx, dy); break;
			case 5: shear(dx, dy); break;
		}
		Draw();
	}
}

// Evento de clique do mouse
void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_UP) // botão solto
	{
		if (button == GLUT_LEFT_BUTTON) {
			if (jaPoligono == 0) { // ainda não definido o polígono
				x = x - windW;
				y = windH - y;

				pvertex[gNumVert].v[0] = (float)x;
				pvertex[gNumVert].v[1] = (float)y;
				pvertex[gNumVert].v[2] = 1.0f;
				gNumVert++;
			}
		}
	}
	else { // botão pressionado
		if (jaPoligono == 1) { // polígono já definido
			if (gTransform) { // Uma transformação selecionada
				if (button == GLUT_LEFT_BUTTON) {
					x = x - windW;
					y = windH - y;
					gIndVert = clipVertex(x, y); // verificar e identificar um vértice selecionado
				}
			}
		}
	}
	glutPostRedisplay();
}

static void Args(int argc, char **argv)
{
	GLint i;

	doubleBuffer = GL_FALSE;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-sb") == 0) {
			doubleBuffer = GL_FALSE;
		}
		else if (strcmp(argv[i], "-db") == 0) {
			doubleBuffer = GL_TRUE;
		}
	}
}

// Eventos do menu geral
void processMenuEvents(int option)
{
	switch (option) {
		case 0:
			init();
			break;
	}
	glutPostRedisplay();
}

// Eventos do menu de poligono
void processSubMenuEventsPolig(int option)
{
	if (option == 0)
		tipoPoligono = GL_POINTS;
	else if (option == 1) {
		tipoPoligono = GL_LINE_LOOP;
		jaPoligono = 1;
	}
	glutPostRedisplay();
}

// Eventos do menu de transformadas
void processSubMenuEventsTransf(int option)
{
	if (jaPoligono) {
		gTransform = option;
	}
	glutPostRedisplay();
}

// Eventos do menu de curvas
void processSubMenuEventsCurves(int option)
{
	curveType = option;
	glutPostRedisplay();
}

// Definição do Menu principal
void createGLUTMenus()
{
	int menu, submenu1, submenu2, submenu3;

	submenu1 = glutCreateMenu(processSubMenuEventsPolig);
	glutAddMenuEntry("Pontos", 0);
	glutAddMenuEntry("Polígono", 1);

	submenu2 = glutCreateMenu(processSubMenuEventsTransf);
	glutAddMenuEntry("Translação", 1);
	glutAddMenuEntry("Rotação", 2);
	glutAddMenuEntry("Escala", 3);
	glutAddMenuEntry("Espelho", 4);
	glutAddMenuEntry("Cisalha", 5);

	submenu3 = glutCreateMenu(processSubMenuEventsCurves);
	glutAddMenuEntry("Bezier", BEZIER);
	glutAddMenuEntry("Hermite", HERMITE);
	glutAddMenuEntry("B-Spline", BSPLINE);
	glutAddMenuEntry("Catmull-Rom", CATMULLR);

	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Limpar", 0);
	glutAddSubMenu("Tipo Objeto", submenu1);
	glutAddSubMenu("Transformação", submenu2);
	glutAddSubMenu("Curvas", submenu3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
	GLenum type;

	glutInit(&argc, argv);
	Args(argc, argv);

	type = GLUT_RGB;
	type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;

	glutInitDisplayMode(type);
	glutInitWindowSize(600, 500);
	glutCreateWindow("Curvas e Transformações");

	init();

	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Draw);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);

	createGLUTMenus();

	glutMainLoop();

	return 0;
}
