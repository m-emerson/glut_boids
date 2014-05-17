#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <GL/glut.h>
#include "boids.h"
#include <sys/queue.h>

GLint windW = 1000, windH = 800;
GLuint selectBuf[MAXSELECT];
GLfloat feedBuf[MAXFEED];
GLint vp[4];
float zRotation = 90.0;
float zoom = 1.0;
float refresh = 50;

GLint objectCount;
GLint numObjects;

GLenum linePoly = GL_FALSE;

// Bounds for position
int XMax = 300;
int XMin = -300;
int YMax = 300;
int YMin = -300;

int preyCount = 0;
struct prey **preys = NULL;

float colours[3][3];

// Global mouse position
struct vector MousePos[0];

void GetOGLPos(int x, int y, struct vector *MousePos)
{
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble posX, posY, posZ;
 
    glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
    glGetDoublev( GL_PROJECTION_MATRIX, projection );
    glGetIntegerv( GL_VIEWPORT, viewport );
 
    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels( x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
 
    gluUnProject( winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
    MousePos->x = posY;
    MousePos->y = -posX;
}

void AddPrey(int x, int y)
{
	int i;
	// Add to the prey and reallocate
	preyCount++;

	// Reallocate our prey array according to the prey
	preys = realloc(preys, preyCount * sizeof(struct prey *));

	// Allocate a new prey
	preys[preyCount - 1] = malloc(sizeof(struct prey));
	
	// Put the centre on mouse click
	preys[preyCount - 1]->centre.x = x;
	preys[preyCount - 1]->centre.y = y;

	// Give it a pretty random colour
	preys[preyCount - 1]->color[0] = ((rand() % 100) + 50) / 150.0;
	preys[preyCount - 1]->color[1] = ((rand() % 100) + 50) / 150.0;
	preys[preyCount - 1]->color[2] = ((rand() % 100) + 50) / 150.0;
	
	// Set this to 10 for now
	preys[preyCount - 1]->radius = 5;

	printf("We have: %d preys\n", preyCount);
	for (i = 0; i < preyCount; i++) {
		printf("centre[%d]: %f,%f\n", i, preys[i]->centre.x, preys[i]->centre.y);
	}
}

// Adds two vectors
struct vector
AddVectors(struct vector v1, struct vector v2)
{
	struct vector ret;
	ret.x = v1.x + v2.x;
	ret.y = v1.x + v2.y;
	return ret;
}

float
square(float a)
{
	float ret;
	ret = a * a;
	return(ret);
}

int
check_within_radius(struct vector *centre, struct vector *v, float radius)
{
	// c^2 = a^2 + b^2
	if ( (square(v->x - centre->x) + square(v->y - centre->y)) < square(radius) )
		return 1;
	else
		return 0;
}

float
GetMagnitude(struct vector *v)
{
	float t;
	t = v->x*v->x + v->y*v->y;
	return sqrt(t);	
}

static void
InitObjects(int num)
{
	GLint i;
	float x, y;
	int cIndex;

	// Create some random colours
	for (i = 0; i < MAX_FISH_TYPES; i++) {
		colours[i][0] = ((rand() % 100) + 50) / 150.0;
		colours[i][1] = ((rand() % 100) + 50) / 150.0;
		colours[i][2] = ((rand() % 100) + 50) / 150.0;
	}

	if (num > MAXBOIDS) {
		num = MAXBOIDS;
	}
	if (num < 1) {
		num = 1;
	}
	objectCount = num;

	srand((unsigned int) time(NULL));
	for (i = 0; i < num; i++) {
		cIndex = (rand() % 3);
		x = (rand() % 300) - 150;
		y = (rand() % 300) - 150;

		// Set the triangle variables for each boid
		boids[i].t.p1.x = x;
		boids[i].t.p2.x = x + 5;
		boids[i].t.p3.x = x + 2.5;
		boids[i].t.p1.y = y;
		boids[i].t.p2.y = y;
		boids[i].t.p3.y = y + 10;

		boids[i].color[0] = colours[cIndex][0]; 
		boids[i].color[1] = colours[cIndex][1];
		boids[i].color[2] = colours[cIndex][2];

		boids[i].velocity.x = 0;
		boids[i].velocity.y = 0;

		// Initialise rotation angle to 0
		boids[i].rotate = 0.0f;
	}
}

static void
Init(void)
{
	numObjects = MAXBOIDS;
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
	// Circle variables
	int x, y;

	float angle;

	// Draw all the boids
	for (i = 0; i < numObjects; i++) {
		if (mode == GL_SELECT) {
			glLoadName(i);
		}
		//glRotatef(boids[i].rotate, 0.0f, 0.0f, 0.5f);
		glColor3fv(boids[i].color);
		glBegin(GL_POLYGON);
		glVertex2f(boids[i].t.p1.x, boids[i].t.p1.y);
		glVertex2f(boids[i].t.p2.x, boids[i].t.p2.y);
		glVertex2f(boids[i].t.p3.x, boids[i].t.p3.y);
		glEnd();

		// Draw the slider
		/*glColor3d(1, 1, 1); 
		glBegin(GL_POLYGON);
    			glVertex2d(425, -425);
    			glVertex2d(425, -375);
    			glVertex2d(450, -450);
    			glVertex2d(450, -375);
		glEnd();

		glLineWidth(2.5);
		glColor3f(1.0, 1.0, 1.0);
		glBegin(GL_LINES);
			glVertex2f(400, -400);
			glVertex2f(400, -475);
		glEnd();*/

		// Make the fish rotate
//		boids[i].rotate += 1.0f;
	}

	// Draw any prey
	for (i = 0; i < preyCount; i++) {
		glColor3fv(preys[i]->color);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(preys[i]->centre.x, preys[i]->centre.y);
		for (angle=1.0f;angle<361.0f;angle+=0.2) {
    			x = preys[i]->centre.x + sin(angle) * preys[i]->radius;
    			y = preys[i]->centre.y + cos(angle) * preys[i]->radius;
    			glVertex2f(x,y);
		}
		glEnd();
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
		GetOGLPos(x, y, &MousePos[0]);
}

float
AvoidPredator(struct boid *b)
{

	// Scatter the flock if its within the same radius of the mouse
	if (check_within_radius(&b->t.p1, &MousePos[0], MAX_RADIUS)) { 
		return -10;
	} else {
		return 1;
	}
}

struct vector *
MoveTowardsGoal(struct boid *b)
{
	int i;
	struct vector *v = malloc(sizeof(struct vector));

	// For determining the closest prey
	struct vector t;
	int preyIndex;

	float diff_magnitude;
	float magnitude;
	
	// Furthest possible distance
	v->x = 0;
	v->y = 0;

	// If there are no prey, there's no need to go towards it
	if (preyCount == 0)
		return v;

	v->x = preys[0]->centre.x;
	v->y = preys[0]->centre.y;
	
	// Get the magnitude from our first prey
	VectorMinus(&t, &b->t.p1, v);
	magnitude = GetMagnitude(&t);

	printf("First magnitude: %f\n", magnitude);
		
	// For every prey that exists
	for (i = 1; i < preyCount; i++) {
		VectorMinus(&t, &b->t.p1, &preys[i]->centre);
		diff_magnitude = GetMagnitude(&t);
		if (diff_magnitude < magnitude) {
			v->x = t.x;
			v->y = t.y;
			magnitude = diff_magnitude;
			preyIndex = i;
		}
		
	}

	// If we are touching the prey, make it disappear

	if (magnitude < 10) {
		printf("WE ARE CLOSE!!!\n");
		printf("Magnitude is: %f\n", magnitude);
		preyCount--;	
		preys[preyIndex] = NULL;
	}
	
	// Closest prey is now in v
	VectorMinus(v, v, &b->t.p1);
	VectorDivide(v, v, 100.0);	
	return v;
	
}

struct vector *
KeepDistance(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	// Temporary for doing calculations
	struct vector *t = malloc(sizeof(struct vector));
	
	// Set up initial values
	v->x = 0;
	v->y = 0;

	int i, j;
	int boids_count;
	
	for (i = 0; i < MAXBOIDS; i++) {
		if (b == &boids[i])
			continue;

		if (check_within_radius(&b->t.p1, &boids[i].t.p1, 15)) {
			VectorMinus(t, &boids[i].t.p1, &b->t.p1);
			VectorMinus(v, v, t);
		}
	}	

	free(t);
	return(v);
	

}

static void
Mouse(int button, int state, int mouseX, int mouseY)
{
	GLint hit;
	float x, y;
	
	struct vector clickPos;

	if (state == GLUT_DOWN) {
		GetOGLPos(mouseX, mouseY, &clickPos);
		AddPrey(clickPos.x, clickPos.y);
		//printf("Mouse position: x:%f,y:%f\n", MousePos[0].x, MousePos[0].y);
		
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
DrawSlider(void)
{
}

static void
Draw(void)
{
	int i;

	struct vector *r0;
	struct vector *r1;
	struct vector *r2;
	struct vector *r3;
	struct vector *r4;

	float m1 = 1; // multiplier

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

	for (i = 0; i < MAXBOIDS; i++) {
		
		m1 = AvoidPredator(&boids[i]);
		// Apply Rules
		r0 = StayInBounds(&boids[i]);
		r1 = MoveTowardsCentre(&boids[i]);
		// Add a negative multiplying factor if a mouse is near the swarm
		VectorMultiply(r1, r1, m1);
		r2 = MatchVelocity(&boids[i]);
		r3 = KeepDistance(&boids[i]);
		r4 = MoveTowardsGoal(&boids[i]);

		// Add R0 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r0);
		// Add R1 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r1);
		// Add R2 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r2);
		// Add R3 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r3);
		// Add R4 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r4);
			


	
		// Limit the speed according to the maximum speed	

		LimitSpeed(&boids[i]);
		if (boids[i].velocity.y > 1000) {
			exit(1);
		}
	
		// Apply velocity to position	
		VectorAdd(&boids[i].t.p1, &boids[i].t.p1, &boids[i].velocity);
		VectorAdd(&boids[i].t.p2, &boids[i].t.p2, &boids[i].velocity);
		VectorAdd(&boids[i].t.p3, &boids[i].t.p3, &boids[i].velocity);

		free(r0);
		free(r1);
		free(r2);
		free(r3);
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

void
VectorAdd(struct vector *ret, struct vector *a, struct vector *b)
{
	ret->x = a->x + b->x;
	ret->y = a->y + b->y;
}

void
VectorMinus(struct vector *ret, struct vector *a, struct vector *b)
{
	ret->x = a->x - b->x;
	ret->y = a->y - b->y;
}

void
VectorDivide(struct vector *ret, struct vector *a, float scalar)
{

	ret->x = a->x / scalar;
	ret->y = a->y / scalar;
}

void
VectorMultiply(struct vector *ret, struct vector *a, float scalar)
{
	ret->x = a->x * scalar;
	ret->y = a->y * scalar;

}

struct vector *
StayInBounds(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));

	// Initialise values to 0
	v->x = 0;
	v->y = 0;

	//printf("t.p1.x: %f\n", b->t.p1.x);	
	
	if (b->t.p1.x < (float)XMin)
		v->x = 10;
	else if (b->t.p1.x > (float)XMax)
		v->x = -10;
	
	if (b->t.p1.y < (float)YMin)
		v->y = 10;
	else if (b->t.p1.y > (float)YMax) 
		v->y = -10;

	return v;
}

void
LimitSpeed(struct boid *b)
{
	if (GetMagnitude(&b->velocity) > MAX_SPEED) {
		VectorDivide(&b->velocity, &b->velocity, GetMagnitude(&b->velocity));
		b->velocity.x = b->velocity.x * (float)MAX_SPEED;
		b->velocity.y = b->velocity.y * (float)MAX_SPEED;
	}
}

int
CheckSameFlock(struct boid *a, struct boid *b)
{
	if ( (a->color[0] == b->color[0])
		&& (a->color[1] == b->color[1])
		&& (a->color[2] == b->color[2]) ) {
		return 1;
	} else {
		return 0;
	}
}

// Rule 1
struct vector *
MoveTowardsCentre(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	int i, j; 

	int boids_count = 0; // Count of boids in local radius

	v->x = 0;
	v->y = 0;

	for (i = 0; i < MAXBOIDS; i++) {
		// Check if its the same boid (memory addresses are the same)
		// If it is, skip it
		if (b == &boids[i])
			continue;
			
			//if (check_within_radius(&b->t.p1, &boids[i].t.p1, MAX_RADIUS)) { 
				//printf("It is within the correct radius\n");
				VectorAdd(v, v, &boids[i].t.p1);
				boids_count++;
			//}
	}
	
	// Nothing in the local radius
	if (v->x == 0 && v->y == 0)
		return v;

	// Calculate vector offset
	VectorDivide(v, v, (float)(boids_count));
	//VectorDivide(v, v, (float)(MAXBOIDS - 1));
	// 1% towards the centre = pcj = bj.position / 100
	VectorMinus(v, v, &boids[i].t.p1);	
	VectorDivide(v, v, 100);
	
	return v; 
}

// Rule 2
/*static void
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
}*/

// Rule 3
struct vector *
MatchVelocity(struct boid *b)
{
	int i, j;
	struct vector *v = malloc(sizeof(struct vector));

	int boids_count = 0;
	v->x = 0;
	v->y = 0;

	for (i = 0; i < MAXBOIDS; i++) {
		if (b == &boids[i])
			continue;

			//if (check_within_radius(&b->t.p1, &boids[i].t.p1, MAX_RADIUS)) {
				VectorAdd(v, v, &boids[i].velocity);
				boids_count++;
		//	}
	}	

	if (!boids_count)
		return v;
		
	// Calcualte perceived velocity
	VectorDivide(v, v, boids_count);
	
	// Add a portion to the current velocity (lets say 1/8th)
	VectorMinus(v, v, &b->velocity);
	VectorDivide(v, v, 8.0);	

	return v;
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
	glutInitWindowSize(windW, windH);
	glutCreateWindow("OpenGL Boids");
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
