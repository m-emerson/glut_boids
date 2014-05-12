#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include "boids.h"

#define MAXBOIDS 20
#define MAXSELECT 100
#define MAXFEED 300
#define	SOLID 1
#define	LINE 2
#define	POINT 3

GLint windW = 300, windH = 300;
GLuint selectBuf[MAXSELECT];
GLfloat feedBuf[MAXFEED];
GLint vp[4];
float zRotation = 90.0;
float zoom = 1.0;
float refresh = 50;

float mouseX, mouseY;
GLint objectCount;
GLint numObjects;

// Vector struct
struct vector {
	int x;
	int y;
} vector;

// Triangle has three vectors/points
struct triangle {
	struct vector p1;
	struct vector p2;
	struct vector p3;
	struct vector centre;
} triangle;
	
// Struct for boid
struct boid {
	float v1[2];
	float v2[2];
	float v3[2];
	float color[3];
	float rotate;

	struct triangle t;
	// Velocity

	float velocity[2];

	// Vectors for applying rules
	float r1[2];
	float r2[2];
	float r3[2];
	float r4[2];

} boids[MAXBOIDS];

GLenum linePoly = GL_FALSE;

static void
InitObjects(int num)
{
	GLint i;
	float x, y;

	if (num > MAXBOIDS) {
		num = MAXBOIDS;
	}
	if (num < 1) {
		num = 1;
	}
	objectCount = num;

	srand((unsigned int) time(NULL));
	for (i = 0; i < num; i++) {
		x = (rand() % 300) - 150;
		y = (rand() % 300) - 150;

		boids[i].t.p1.x = x;
		boids[i].t.p2.x = x + 10;
		boids[i].t.p3.x = x + 5;
		boids[i].t.p1.y = y;
		boids[i].t.p2.y = y;
		boids[i].t.p2.y = y + 20;

		boids[i].v1[0] = x;
		boids[i].v2[0] = x + 10;
		boids[i].v3[0] = x + 5;
		boids[i].v1[1] = y;
		boids[i].v2[1] = y;
		boids[i].v3[1] = y + 20;
		boids[i].color[0] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[1] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[2] = ((rand() % 100) + 50) / 150.0;

		boids[i].velocity[0] = 0;
		boids[i].velocity[1] = 0;

		// Initialise rotation angle to 0
		boids[i].rotate = 0.0f;
	}
}

static void
Init(void)
{
	numObjects = 20;
	InitObjects(numObjects);
}

static void
Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(refresh, Timer, 0);
}

static void
Reshape(int width, int height)
{
	windW = width;
	windH = height;
	glViewport(0, 0, windW, windH);
	glGetIntegerv(GL_VIEWPORT, vp);
}

static void
Render(GLenum mode)
{
	GLint i;

	for (i = 0; i < objectCount; i++) {
		if (mode == GL_SELECT) {
			glLoadName(i);
		}
		//glRotatef(boids[i].rotate, 0.0f, 0.0f, 0.5f);
		glColor3fv(boids[i].color);
		glBegin(GL_POLYGON);
		glVertex2fv(boids[i].v1);
		glVertex2fv(boids[i].v2);
		glVertex2fv(boids[i].v3);
		glEnd();

		// Make the fish rotate
		boids[i].rotate += 1.0f;
	}
}

static GLint
DoSelect(GLint x, GLint y)
{
	GLint hits;

	glSelectBuffer(MAXSELECT, selectBuf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(~0);

	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix(x, windH - y, 4, 4, vp);
	gluOrtho2D(-175, 175, -175, 175);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	Render(GL_SELECT);

	glPopMatrix();

	hits = glRenderMode(GL_RENDER);
	if (hits <= 0) {
		return -1;
	}
	return selectBuf[(hits - 1) * 4 + 3];
}

static void
RecolorTri(GLint h)
{
	boids[h].color[0] = ((rand() % 100) + 50) / 150.0;
	boids[h].color[1] = ((rand() % 100) + 50) / 150.0;
	boids[h].color[2] = ((rand() % 100) + 50) / 150.0;
}

static void
DeleteTri(GLint h)
{
	boids[h] = boids[objectCount - 1];
	objectCount--;
}

static void
GrowTri(GLint h)
{
	float v[2];
	float *oldV;
	GLint i;

	v[0] = boids[h].v1[0] + boids[h].v2[0] + boids[h].v3[0];
	v[1] = boids[h].v1[1] + boids[h].v2[1] + boids[h].v3[1];
	v[0] /= 3;
	v[1] /= 3;

	for (i = 0; i < 3; i++) {
		switch (i) {
			case 0:
				oldV = boids[h].v1;
				break;
			case 1:
				oldV = boids[h].v2;
				break;
			case 2:
				oldV = boids[h].v3;
				break;
		}
		oldV[0] = 1.5 * (oldV[0] - v[0]) + v[0];
		oldV[1] = 1.5 * (oldV[1] - v[1]) + v[1];
	}
}

static void
MoveMouse(int x, int y)
{
	// Convert it to the window coordinates
	mouseX = (x / (float)windW) - 0.5f;
	mouseY = (x / (float)windH) - 0.5f; 
}

static void
AvoidPredator()
{
	printf("%f\n", mouseX);
	int i;
	for (i = 0; i < MAXBOIDS; i++) {
		// Begin avoiding if the mouse only if the vectors are close
		boids[i].r4[0] = -0.01 * ((mouseX - boids[i].v1[0]) / 100);
		boids[i].r4[1] = -0.01 * ((mouseY - boids[i].v1[0]) / 100);
	}
}

static void
Mouse(int button, int state, int mouseX, int mouseY)
{
	GLint hit;
	if (state == GLUT_DOWN) {
		hit = DoSelect((GLint) mouseX, (GLint) mouseY);
		if (hit != -1) {
			if (button == GLUT_LEFT_BUTTON) {
				RecolorTri(hit);
			} else if (button == GLUT_MIDDLE_BUTTON) {
				GrowTri(hit);
			} else if (button == GLUT_RIGHT_BUTTON) {
				DeleteTri(hit);
			}
			glutPostRedisplay();
		}
	}
}

static void
Draw(void)
{
	int i;

	glPushMatrix();
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluOrtho2D(-175, 175, -175, 175);
	gluOrtho2D(-500, 500, -500, 500);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	// R1	
	MoveTowardsCentre();

	// R2
	KeepDistance();

	// R3
	MatchVelocity();

	// R4
	AvoidPredator();

	for (i = 0; i < MAXBOIDS; i++) {

		boids[i].velocity[0] = boids[i].velocity[0] + boids[i].r1[0] + boids[i].r2[0] + boids[i].r3[0] + boids[i].r4[0];
		boids[i].velocity[1] = boids[i].velocity[1] + boids[i].r1[1] + boids[i].r2[1] + boids[i].r3[0] + boids[i].r4[1];
		boids[i].v1[0] = boids[i].v1[0] + boids[i].velocity[0];
		boids[i].v1[1] = boids[i].v1[1] + boids[i].velocity[1];
		boids[i].v2[0] = boids[i].v2[0] + boids[i].velocity[0];
		boids[i].v2[1] = boids[i].v2[1] + boids[i].velocity[1];
		boids[i].v3[0] = boids[i].v3[0] + boids[i].velocity[0];
		boids[i].v3[1] = boids[i].v3[1] + boids[i].velocity[1];
	}

	Render(GL_RENDER);
	glPopMatrix();
	glutSwapBuffers();
}

static void
DumpFeedbackVert(GLint * i, GLint n)
{
	GLint index;

	index = *i;
	if (index + 7 > n) {
		*i = n;
		printf("  ???\n");
		return;
	}
	printf("  (%g %g %g), color = (%4.2f %4.2f %4.2f)\n",
			feedBuf[index],
			feedBuf[index + 1],
			feedBuf[index + 2],
			feedBuf[index + 3],
			feedBuf[index + 4],
			feedBuf[index + 5]);
	index += 7;
	*i = index;
}

static void
DrawFeedback(GLint n)
{
	GLint i;
	GLint verts;

	printf("Feedback results (%d floats):\n", n);
	for (i = 0; i < n; i++) {
		switch ((GLint) feedBuf[i]) {
			case GL_POLYGON_TOKEN:
				printf("Polygon");
				i++;
				if (i < n) {
					verts = (GLint) feedBuf[i];
					i++;
					printf(": %d vertices", verts);
				} else {
					verts = 0;
				}
				printf("\n");
				while (verts) {
					DumpFeedbackVert(&i, n);
					verts--;
				}
				i--;
				break;
			case GL_LINE_TOKEN:
				printf("Line:\n");
				i++;
				DumpFeedbackVert(&i, n);
				DumpFeedbackVert(&i, n);
				i--;
				break;
			case GL_LINE_RESET_TOKEN:
				printf("Line Reset:\n");
				i++;
				DumpFeedbackVert(&i, n);
				DumpFeedbackVert(&i, n);
				i--;
				break;
			default:
				printf("%9.2f\n", feedBuf[i]);
				break;
		}
	}
	if (i == MAXFEED) {
		printf("...\n");
	}
	printf("\n");
}

static void
DoFeedback(void)
{
	GLint x;

	glFeedbackBuffer(MAXFEED, GL_3D_COLOR, feedBuf);
	(void) glRenderMode(GL_FEEDBACK);

	glPushMatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-175, 175, -175, 175);
	glMatrixMode(GL_MODELVIEW);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glScalef(zoom, zoom, zoom);
	glRotatef(zRotation, 0, 0, 1);

	Render(GL_FEEDBACK);

	glPopMatrix();

	x = glRenderMode(GL_RENDER);
	if (x == -1) {
		x = MAXFEED;
	}
	DrawFeedback((GLint) x);
}

/* ARGSUSED1 */
static void
Key(unsigned char key, int x, int y)
{

	switch (key) {
		case 'z':
			zoom /= 0.75;
			glutPostRedisplay();
			break;
		case 'Z':
			zoom *= 0.75;
			glutPostRedisplay();
			break;
		case 'f':
			DoFeedback();
			glutPostRedisplay();
			break;
		case 'l':
			linePoly = !linePoly;
			if (linePoly) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			} else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			glutPostRedisplay();
			break;
		case 27:
			exit(0);
	}
}

/* ARGSUSED1 */
static void
SpecialKey(int key, int x, int y)
{

	switch (key) {
		case GLUT_KEY_LEFT:
			zRotation += 0.5;
			glutPostRedisplay();
			break;
		case GLUT_KEY_RIGHT:
			zRotation -= 0.5;
			glutPostRedisplay();
			break;
	}
}

// Rule 1
void
MoveTowardsCentre(void)
{
	int i, j;
	/* Move all the boids towards the perceived centre */

	for (i = 0; i < MAXBOIDS; i++) {
		// Initialise to 0
		boids[i].r1[0] = 0;
		boids[i].r1[1] = 0;

		for (j = 0; j < MAXBOIDS; j++) {
			// Skip the positional information of the boid we're looking at
			if (i == j)
				continue;

			// In future we may want to set the 'position' of the boid as it's centre, but for now let's just pick one point
			boids[i].r1[0] += boids[j].v1[0];
			boids[i].r1[1] += boids[j].v1[1];
		}

		// Calculate vector offset
		// pcj = pcj / n - 1
		boids[i].r1[0] = boids[i].r1[0] / (MAXBOIDS - 1);
		boids[i].r1[1] = boids[i].r1[0] / (MAXBOIDS - 1);

		// 1% towards the centre = pcj = bj.position / 100 
		boids[i].r1[0] = (boids[i].r1[0] - boids[i].v1[0]) / 100;
		boids[i].r1[1] = (boids[i].r1[1] - boids[i].v1[1]) / 100;
	}
}

// Rule 2
static void
KeepDistance(void)
{
	int i, j;
	float distance, a, b, c_squared;
	// Initialise our new vector to 0
	for (i = 0; i < MAXBOIDS; i++) {
		boids[i].r2[0] = 0;
		boids[i].r2[1] = 0;
		for (j = 0; j < MAXBOIDS; j++) {
			if (i == j)
				continue;
			// Use pythagorus to determine distance between two boids
			a = (boids[i].v1[0] - boids[j].v1[0]);
			b = (boids[i].v1[1] - boids[j].v1[1]);
			c_squared = (a * a) + (b * b);
			distance = sqrt(c_squared);
			// If the distance is less than 10 away from each other, apply rule
			if (distance < 20) {
				boids[i].r2[0] = boids[i].r2[0] - (boids[j].v1[0] - boids[i].v1[0]);
				boids[i].r2[1] = boids[i].r2[1] - (boids[j].v1[1] - boids[i].v1[1]);
			}
 		} 
	}
}

// Rule 3
static void
MatchVelocity(void)
{
	int i, j;
	for (i = 0; i < MAXBOIDS; i++) {
		for (j = 0; j < MAXBOIDS; j++) {
			if (i == j)
				continue;
			boids[i].r3[0] = boids[i].r3[0] + boids[j].velocity[0];
			boids[i].r3[1] = boids[i].r3[1] + boids[j].velocity[1];
		}	
		// Calcualte perceived velocity
		boids[i].r3[0] = boids[i].r3[0] / (MAXBOIDS - 1);
		boids[i].r3[1] = boids[i].r3[1] / (MAXBOIDS - 1);
		
		// Add a portion to the current velocity (lets say 1/8th)
		boids[i].r3[0] = (boids[i].r3[0] - boids[i].velocity[0]) / 20;
		boids[i].r3[1] = (boids[i].r3[1] - boids[i].velocity[1]) / 20;
		
	}
}

static void
DetermineNewPositions(void)
{
	/* each boids new position can be determined through the following rules:
	   1. boids try to fly towards teh centre of mass of neighbouring boids
	   pcj = b1.position + b2.position + ... + bj-1.position + bj+1.position + ... + bn.position) / (N-1) */

	// Apply rule 1

}

int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Select Test");
	Init();
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutSpecialFunc(SpecialKey);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(MoveMouse);
	glutDisplayFunc(Draw);
	glutTimerFunc(0, Timer, 0);
	glutMainLoop();
	return 0;             /* ANSI C requires main to return int. */
}
