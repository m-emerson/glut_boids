#include <sys/queue.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>	   // The GL Utility Toolkit (GLUT) Header
#include <math.h>
#include <soil.h>

#define KEY_ESCAPE 27
#define MAX_BOIDS 50
#define BOUNDS 10 
#define MAX_SPEED 0.5
#define TAIL_LENGTH 20 

typedef struct {
    int width;
	int height;
	char* title;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;
glutWindow win;

float Rotation;
float Translation = 0.01;
float refresh = 50;
int tailCount = 0;
GLuint texture[0];

// Initial camera position
int x = 0;
int y = -BOUNDS * 2;

// Amount of prey on screen
int preyCount = 0;


struct vector {
	float x;
	float y;
	float z;
} vector;

struct tails {
	struct vector position;
	LIST_ENTRY(tails) pointers;
} tails;

struct prey {
	struct vector position;
	float color[3];
	LIST_ENTRY(prey) pointers;
} prey;

// Global linked list for prey
LIST_HEAD(preysHead, prey) preysHead;

struct boid {
	struct vector position;
	struct vector velocity;
	float color[3];
	LIST_HEAD(tailsHead, tails) tailsHead;
	struct vector tailPositions[10];
} boids[MAX_BOIDS];

int LoadGLTextures(char *filename)
{
	texture[0] = SOIL_load_OGL_texture
	(	
		filename,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);
	
	if (texture[0] == 0)
		return 0;
	
	glBindTexture(GL_TEXTURE_2D, texture[0]);
 	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_REPEAT);
	return 1;
}

void FreeTexture()
{
	glDeleteTextures(1, &texture[0]);
}

// Multiplyer should be 1.0 or -1.0, so we can use this function for VectorMinus too
void
VectorAdd(struct vector *ret, struct vector *a, struct vector *b, float scalar)
{
	ret->x = a->x + (b->x * scalar);
	ret->y = a->y + (b->y * scalar);
	ret->z = a->z + (b->z * scalar);
}

void
VectorDivide(struct vector *ret, struct vector *a, float scalar)
{
	ret->x = a->x / scalar;
	ret->y = a->y / scalar;
	ret->z = a->z / scalar;
}

void
VectorMultiply(struct vector *ret, struct vector *a, float scalar)
{
	ret->x = a->x * scalar;
	ret->y = a->y * scalar;
	ret->z = a->z * scalar;
}

float
square(float a)
{
	float ret;
	ret = a * a;
	return(ret);
}

float
GetMagnitude(struct vector *v)
{
	float t;
	t = square(v->x) + square(v->y) + square(v->z);
	return sqrt(t);

}

int
CheckWithinRadius(struct vector *position, struct vector *v, float radius)
{
	// c^2 = x^2 + y^2 + z^2
	if ( (square(v->x - position->x) + square(v->y - position->y) + square(v->z - position->z)) < square(radius) )
		return 1;
	else
		return 0;
}

// Generates a random number between -5.0 and 5.0
float
RandomCoordinate()
{
	int random;
	int neg;
	random = rand() % 1000;
	float f_rand = (float)random/(float)100.0;
	
	neg = rand() % 2;

	if (neg) {
		f_rand = f_rand * -1.0;
	}

	printf("%f\n", f_rand);
	return f_rand;

}

void
ProcessKeys(int key, int xx, int yy)
{
	switch(key) {
		case GLUT_KEY_UP : y++; break;
		case GLUT_KEY_DOWN: y--; break;
		case GLUT_KEY_LEFT : x--; break;
		case GLUT_KEY_RIGHT: x++; break;
	}
}


void
AddPrey()
{
	preyCount++;
	struct prey *p = malloc(sizeof(struct prey));
	p->position.x = RandomCoordinate() - 1;
	p->position.y = RandomCoordinate() - 1;
	p->position.z = RandomCoordinate() - 1;

	p->color[0] = 1.0;
	p->color[1] = 1.0;
	p->color[2] = 1.0;

	LIST_INSERT_HEAD(&preysHead, p, pointers);
}

void
DrawPrey()
{
	struct prey *np;
	LIST_FOREACH(np, &preysHead, pointers) {
		glPushMatrix();
		glColor3fv(np->color);	
		glTranslatef(np->position.x, np->position.y, np->position.z);
		glutSolidCube(0.5);
		glPopMatrix();
	}
}

void
ProcessMouseButton(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		AddPrey();
	}
}

void
DetermineNewTracePos(struct boid *b)
{
	int i = 0;
	struct tails *np;
	struct tails *t = malloc(sizeof(struct tails));
	memcpy(&t->position, &b->position, sizeof(struct vector));
	if (tailCount == TAIL_LENGTH) {
		LIST_FOREACH(np, &b->tailsHead, pointers) {
			if (i == tailCount - 1)
				LIST_REMOVE(np, pointers);
			i++;
		}
	}
	LIST_INSERT_HEAD(&b->tailsHead, t, pointers);
	//memcpy(&b->tailEnd, &b->position, sizeof(struct vector));
	//VectorAdd(&b->tailHead, &b->position, &b->velocity, 1.0);*/
}

// Initialisation function, set up boid coordinates
void Init()
{
	srand(time(NULL));
	int i;	
	for (i = 0; i < MAX_BOIDS; i++) {
		boids[i].position.x = RandomCoordinate(10);
		boids[i].position.y = RandomCoordinate(10);
		boids[i].position.z = RandomCoordinate(10);

		boids[i].velocity.x = 0;
		boids[i].velocity.y = 0;
		boids[i].velocity.z = 0;

		boids[i].color[0] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[1] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[2] = ((rand() % 100) + 50) / 150.0;
		
		LIST_INIT(&boids[i].tailsHead);
	}

	// Initialised our linked list of preys
	LIST_INIT(&preysHead);

}

struct vector *
MoveTowardsGoal(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct prey));

	struct vector t_closest; // Temporary for holding calculated values
	struct vector t;

	v->x = 0;
	v->y = 0;
	v->z = 0;	

	struct prey *closest;
	struct prey *np;

	// If there's no prey, don't affect the vector 
	if (preyCount == 0)
		return v;

	int init = 1;

	LIST_FOREACH(np, &preysHead, pointers) {
		VectorAdd(&t, &b->position, &np->position, -1.0);
		if (init) {
			/* First one will be the closest we have found so far */
			closest = np;
			memcpy(&t_closest, &t, sizeof(struct vector));
			init = 0;
		} else {
			if (GetMagnitude(&t) < GetMagnitude(&t_closest)) {
				closest = np;
				memcpy(&t_closest, &t, sizeof(struct vector));
			}
		}
	}
	
	VectorAdd(v, &closest->position, &b->position, -1.0);
	VectorDivide(v, v, 100);

	// If we are touching the prey (within 2 units magnitude), make it disappear
	if (GetMagnitude(&t_closest) < 2) {	
		preyCount--;
		LIST_REMOVE(closest, pointers);
	}
	
	return v;

}

struct vector *
KeepDistance(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	// Temporary for doing calculations
	struct vector *t = malloc(sizeof(struct vector));

	v->x = 0;
	v->y = 0;
	v->z = 0;

	int i;
		
	for (i = 0; i < MAX_BOIDS; i++) {
		if (b == &boids[i])
			continue;
	
		if (CheckWithinRadius(&b->position, &boids[i].position, 1.0)) {
			VectorAdd(t, &boids[i].position, &b->position, -1.0);
			VectorAdd(v, v, t, -1.0);
			VectorDivide(v, v, 8);
		}
	}

	free(t);
	return(v);
}

struct vector *
StayWithinBounds(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	
	v->x = 0;
	v->y = 0;
	v->z = 0;

	if (b->position.x < (float)-BOUNDS)
		v->x = 0.1;
	else if (b->position.x > (float)BOUNDS)
		v->x = -0.1;

	if (b->position.y < (float)-BOUNDS)
		v->y = 0.1;
	else if (b->position.y > (float)BOUNDS)
		v->y = -0.1;
	
	if (b->position.z < (float)-BOUNDS)
		v->z = 0.1;
	else if (b->position.z > (float)BOUNDS)
		v->z = -0.1;

	return v;
	

}

void
LimitSpeed(struct boid *b)
{
	if (GetMagnitude(&b->velocity) > MAX_SPEED) {
		VectorDivide(&b->velocity, &b->velocity, GetMagnitude(&b->velocity));
		VectorMultiply(&b->velocity, &b->velocity, (float)MAX_SPEED);
	}
}

struct vector *
MoveTowardsCentre(struct boid *b)
{
	struct vector *v = malloc(sizeof(struct vector));
	int i, j;

	int boids_count = 0;
	
	v->x = 0;
	v->y = 0;
	v->z = 0;

	for (i = 0; i < MAX_BOIDS; i++) {
		if (b == &boids[i])
			continue;
	
		VectorAdd(v, v, &boids[i].position, 1.0);
	}
	
	// calculate vector offset
	VectorDivide(v, v, (float)(MAX_BOIDS - 1));
	VectorAdd(v, v, &b->position, -1.0);
	// Move about 1/100th the way towards the centre 
	VectorDivide(v, v, 100);
	
	return v;

}

struct vector *
MatchVelocity(struct boid *b)
{
	int i;
	struct vector *v = malloc(sizeof(struct vector));

	v->x = 0;
	v->y = 0;
	v->z = 0;
	
	for (i = 0; i < MAX_BOIDS; i++) {
		if (b == &boids[i])
			continue;
		
		VectorAdd(v, v, &boids[i].velocity, 1.0);
	}

	VectorDivide(v, v, (float)(MAX_BOIDS - 1));
	
	// Add a portion to the current velocity (for now, 1/8th)
	VectorAdd(v, v, &b->velocity, -1.0);
	VectorDivide(v, v, 8.0);
	
	return v;
		
}

void
draw_boids()
{	
	int i;
	int count = 0;
	struct tails *np;
	struct tails *last;
	for (i = 0; i < MAX_BOIDS; i++) {
    		glEnable(GL_LIGHTING);
    		glEnable(GL_LIGHT0); 
		glPushMatrix();
			glTranslatef(boids[i].position.x, boids[i].position.y, boids[i].position.z);
			glColor3fv(boids[i].color);
			glutSolidSphere(0.2, 20, 20);
		glPopMatrix();


		// Disable the lighting for the tails	
		glDisable(GL_LIGHTING);
		glDisable(GL_LIGHT0);

	
		// Draw 
		glPushMatrix();
			glColor3fv(boids[i].color);
			glLineWidth(1.0);
			glBegin(GL_LINES);
				LIST_FOREACH(np, &boids[i].tailsHead, pointers) {
					glVertex3f(np->position.x, np->position.y, np->position.z);
				}
			glEnd();
		glPopMatrix();
	}
}

void
draw_trace()
{
		glColor3f(1.0, 0.0, 0.0);
}

void
display() 
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		     // Clear Screen and Depth Buffer
	glLoadIdentity();
	// Set camera centred at (0, -5.0, 1) with the z axis pointing up
	gluLookAt( x, y, BOUNDS * 2.0, 0,0, (-BOUNDS / 2.0), 0,0,1);					  // Define a viewing transformati
	
	int i;
	int vector = rand() % 10;
	vector = vector * -1;
	//move_towards_centre();

	struct vector *r0;
	struct vector *r1;
	struct vector *r2;
	struct vector *r3;
	struct vector *r4;

	for (i = 0; i < MAX_BOIDS; i++) {
		r0 = StayWithinBounds(&boids[i]);
		r1 = MoveTowardsCentre(&boids[i]);
		r2 = MatchVelocity(&boids[i]);
		r3 = KeepDistance(&boids[i]);
		r4 = MoveTowardsGoal(&boids[i]);

		DetermineNewTracePos(&boids[i]);

		VectorAdd(&boids[i].velocity, &boids[i].velocity, r0, 1.0);
		// Add r1 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r1, 1.0);
		// Add r2 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r2, 1.0);
		// Add r3 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r3, 1.0);
		// Add r4 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r4, 1.0);

		LimitSpeed(&boids[i]);
		// Apply velocity to position
		VectorAdd(&boids[i].position, &boids[i].position, &boids[i].velocity, 1.0);
		
	}

	if (tailCount < TAIL_LENGTH) 
		tailCount++;

	// Ensure lighting is enabled
    	glEnable( GL_LIGHT0 );
	glEnable( GL_LIGHTING );

	// Draw a floor
	glColor3f(2.0, 1.0, 0.0);
    	glEnable( GL_COLOR_MATERIAL );
   	glEnable( GL_TEXTURE_2D );
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);	glVertex3f(-BOUNDS, -BOUNDS, -BOUNDS);
		glTexCoord2f(0, 1);	glVertex3f(-BOUNDS, BOUNDS, -BOUNDS);
		glTexCoord2f(1, 1);	glVertex3f(BOUNDS, BOUNDS, -BOUNDS);
		glTexCoord2f(1, 0);	glVertex3f(BOUNDS, -BOUNDS, -BOUNDS);
	glEnd();
	glDisable( GL_TEXTURE_2D );

	// Draw wire frame
	glPushMatrix();
		glColor3f(1.0, 1.0, 1.0);
		glutWireCube(BOUNDS * 2);
	glPopMatrix();

	// Draw the prey
	DrawPrey();
	// Draw the boids
	draw_boids();
		


	//avoid_walls();
		//glRotatef(Rotation,0,1,0);						  // Multiply the current matrix by a rotation matrix 
		// Face on
		/*glBegin( GL_TRIANGLES );
			glVertex3f(0.0, 1.0, 0.5f);
			glVertex3f(-0.5, -0.5, 0.5f);
			glVertex3f(0.5, -0.5, 0.5f);*/
			/*glVertex3f(boids[i].b.x, boids[0].b.x, boids[0].b.z);
			glVertex3f(boids[i].b.y, boids[0].b.y, boids[0].b.z);
			glVertex3f(boids[i].b.x, boids[0].b.y, boids[0].b.z);*/
	
	
	free(r0);
	free(r1);
	free(r2);
	free(r3);
	free(r4);

	glutSwapBuffers();
}


void
initialize () 
{
    glMatrixMode(GL_PROJECTION);												// select projection matrix
    glViewport(0, 0, win.width, win.height);									// set the viewport
    glMatrixMode(GL_PROJECTION);												// set matrix mode
    glLoadIdentity();															// reset projection matrix
    GLfloat aspect = (GLfloat) win.width / win.height;
    gluPerspective(win.field_of_view_angle, aspect, win.z_near, win.z_far);		// set up a perspective projection matrix
    glMatrixMode(GL_MODELVIEW);													// specify which matrix is the current matrix
    glShadeModel( GL_SMOOTH );
    glClearDepth( 1.0f );														// specify the clear value for the depth buffer
    glEnable( GL_DEPTH_TEST );
    glDepthFunc( GL_LEQUAL );
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );						// specify implementation-specific hints

    GLfloat blue[] = {0.0f, 0.0f, 0.1f, 1.0f};
    GLfloat amb_light[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[] = { 0.6, 0.6, 0.6, 1 };
    GLfloat specular[] = { 0.7, 0.7, 0.3, 1 };
    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, amb_light );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHT0 );
    glEnable( GL_COLOR_MATERIAL );
    glShadeModel( GL_SMOOTH );
    glLightModeli( GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE );
    glDepthFunc( GL_LEQUAL );
    glEnable( GL_DEPTH_TEST );
    glClearColor(0, 0, 0, 0);

    // Bind texture
   if (LoadGLTextures("sand.jpg") == 0) {
      printf("error loading sand texture");
   }
}

static void
Timer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(refresh, Timer, 0);
}

int main(int argc, char **argv) 
{
	// set window values
	win.width = 1000;
	win.height = 1000;
	win.title = "OpenGL Boids 3D";
	win.field_of_view_angle = 45;
	win.z_near = 1.0f;
	win.z_far = 500.0f;

	glutInit(&argc, argv);
	Init();
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize(win.width,win.height);
	glutCreateWindow(win.title);
	glutDisplayFunc(display);
	glutTimerFunc(0, Timer, 0);
        glutSpecialFunc(ProcessKeys);
	glutMouseFunc(ProcessMouseButton);
	initialize();
	glutMainLoop();
	
	return 0;
}
