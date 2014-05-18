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

// Global counter for amount of prey on screen
int preyCount = 0;

// Create linked list for prey
LIST_HEAD(preysHead, prey) preysHead;

// List to contain colours that boids can be
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

void
DetermineTailPos(struct boid *b)
{
	// Copy the centre value over for the tail end
	memcpy(&b->tailEnd, &b->centre, sizeof(struct vector));
	// New beginning of tail is at the centre
	VectorAdd(&b->tailHead, &b->centre, &b->velocity);
}

void
AddPrey(int x, int y)
{
	preyCount++;
	// Allocate a new prey
	struct prey *p = malloc(sizeof(struct prey));
	
	// Put the centre on mouse click
	p->centre.x = x;
	p->centre.y = y;

	// Make the prey white
	p->color[0] = 1.0;
	p->color[1] = 1.0;
	p->color[2] = 1.0;

	// Set this to 5 for now	
	p->radius = 10;
	
	// Insert new prey into linked list	
	LIST_INSERT_HEAD(&preysHead, p, pointers);
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

		// Determine initial centre for boid
		x = (rand() % 300) - 150;
		y = (rand() % 300) - 150;
	
		boids[i].centre.x = x;
		boids[i].centre.y = y;	

		boids[i].radius = 5;
	
		// Set the triangle variables for each boid
		/*boids[i].t.p1.x = x;
		boids[i].t.p2.x = x + 5;
		boids[i].t.p3.x = x + 2.5;
		boids[i].t.p1.y = y;
		boids[i].t.p2.y = y;
		boids[i].t.p3.y = y + 10;*/

		boids[i].color[0] = colours[cIndex][0]; 
		boids[i].color[1] = colours[cIndex][1];
		boids[i].color[2] = colours[cIndex][2];

		boids[i].velocity.x = 0;
		boids[i].velocity.y = 0;

		// Initialise rotation angle to 0
		boids[i].rotate = 0.0f;

		LIST_INIT(&preysHead);

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
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(boids[i].centre.x, boids[i].centre.y);
		for (angle=1.0f;angle<361.0f;angle+=0.2) {
    			x = boids[i].centre.x + sin(angle) * boids[i].radius;
    			y = boids[i].centre.y + cos(angle) * boids[i].radius;
    			glVertex2f(x,y);
		}
		glEnd();

		// Draw the tail
		glLineWidth(2.5);
		glColor3fv(boids[i].color);
		glBegin(GL_LINES);
			glVertex2f(boids[i].tailHead.x, boids[i].tailHead.y);
			glVertex2f(boids[i].tailEnd.x, boids[i].tailEnd.y);
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

	struct prey *np;
	// Draw any prey
	LIST_FOREACH(np, &preysHead, pointers) {
		glColor3fv(np->color);
		glBegin(GL_TRIANGLE_FAN);
		glVertex2f(np->centre.x, np->centre.y);
		for (angle=1.0f;angle<361.0f;angle+=0.2) {
    			x = np->centre.x + sin(angle) * np->radius;
    			y = np->centre.y + cos(angle) * np->radius;
    			glVertex2f(x,y);
		}
		glEnd();
	} 
	free(np);
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
	if (check_within_radius(&b->centre, &MousePos[0], MAX_RADIUS)) { 
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

	struct prey *np;
	struct prey *removePrey;
	// Draw any prey

	int init = 1;

	LIST_FOREACH(np, &preysHead, pointers) {
	// For every prey that exists
		if (init) {
			// Get the first magnitude so we have something
			// to compare to
			VectorMinus(&t, &b->centre, &np->centre);
			magnitude = GetMagnitude(&t);
			init = 0;
			removePrey = np;
			continue;
		}

		VectorMinus(&t, &b->centre, &np->centre);
		diff_magnitude = GetMagnitude(&t);
		if (diff_magnitude < magnitude) {
			v->x = t.x;
			v->y = t.y;
			magnitude = diff_magnitude;
			removePrey = np;
		}
		
	}

	// If we are touching the prey, make it disappear

	if (magnitude < 10) {
		printf("VERY CLOSE!!!\n");
		// Find the prey to remove
		preyCount--;
		LIST_FOREACH(np, &preysHead, pointers) {
			printf("removing now, i think\n");
			if (removePrey  == np) {
				printf("removing from the list\n");
				LIST_REMOVE(np, pointers);
			}
		}
	}
	
	// Closest prey is now in v
	VectorMinus(v, v, &b->centre);
	VectorDivide(v, v, 80.0);	
	free(np);
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

		if (check_within_radius(&b->centre, &boids[i].centre, 15)) {
			VectorMinus(t, &boids[i].centre, &b->centre);
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

		// Determine the tail position after this movement
		DetermineTailPos(&boids[i]);	

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
		VectorAdd(&boids[i].centre, &boids[i].centre, &boids[i].velocity);
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
	
	if (b->centre.x < (float)XMin)
		v->x = 10;
	else if (b->centre.x > (float)XMax)
		v->x = -10;
	
	if (b->centre.y < (float)YMin)
		v->y = 10;
	else if (b->centre.y > (float)YMax) 
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
				VectorAdd(v, v, &boids[i].centre);
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
	VectorMinus(v, v, &boids[i].centre);	
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
