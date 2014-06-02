

/*
 * OpenGLSamples (openglsamples.sf.net) tutorials
 * VC++ users should create a Win32 Console project and link 
 * the program with glut32.lib, glu32.lib, opengl32.lib
 *
 * GLUT can be downloaded from http://www.xmission.com/~nate/glut.html
 * OpenGL is by default installed on your system.
 * For an installation of glut on windows for MS Visual Studio 2010 see: http://nafsadh.wordpress.com/2010/08/20/glut-in-ms-visual-studio-2010-msvs10/
 *
 *
 * main.cpp		
 *
 */

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
int x = -BOUNDS * 2;
int y = -BOUNDS * 2;

struct vector {
	float x;
	float y;
	float z;
} vector;

struct tails {
	struct vector position;
	LIST_ENTRY(tails) pointers;
} tails;

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
	
    // Bind the texture to texture[0]
	//glGenTextures(1, &texture[0]);
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
		boids[i].position.x = RandomCoordinate();
		boids[i].position.y = RandomCoordinate();
		boids[i].position.z = RandomCoordinate();

		boids[i].velocity.x = 0;
		boids[i].velocity.y = 0;
		boids[i].velocity.z = 0;

		boids[i].color[0] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[1] = ((rand() % 100) + 50) / 150.0;
		boids[i].color[2] = ((rand() % 100) + 50) / 150.0;
		
		LIST_INIT(&boids[i].tailsHead);
	}

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
	gluLookAt( x, y, BOUNDS * 2.0, 0,0, (-BOUNDS / 2.0), 0,0,1);					  // Define a viewing transformation
	
	int i;
	int vector = rand() % 10;
	vector = vector * -1;
	//move_towards_centre();

	struct vector *r0;
	struct vector *r1;
	struct vector *r2;
	struct vector *r3;

	for (i = 0; i < MAX_BOIDS; i++) {
		r0 = StayWithinBounds(&boids[i]);
		r1 = MoveTowardsCentre(&boids[i]);
		r2 = MatchVelocity(&boids[i]);
		r3 = KeepDistance(&boids[i]);

		DetermineNewTracePos(&boids[i]);

		VectorAdd(&boids[i].velocity, &boids[i].velocity, r0, 1.0);
		// Add r1 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r1, 1.0);
		// Add r2 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r2, 1.0);
		// Add r3 to velocity
		VectorAdd(&boids[i].velocity, &boids[i].velocity, r3, 1.0);

		LimitSpeed(&boids[i]);
		// Apply velocity to position
		VectorAdd(&boids[i].position, &boids[i].position, &boids[i].velocity, 1.0);
		
	}

	if (tailCount < TAIL_LENGTH) 
		tailCount++;

	// Draw a floor
    	glEnable( GL_LIGHT0 );
    	glEnable( GL_COLOR_MATERIAL );
	glColor3f(2.0, 1.0, 0.0);
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
	
	
	Translation += 0.01;
		
	free(r0);
	free(r1);
	free(r2);
	free(r3);

	glutSwapBuffers();
}


void initialize () 
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


void keyboard ( unsigned char key, int mousePositionX, int mousePositionY )		
{ 
  switch ( key ) 
  {
    case KEY_ESCAPE:        
      exit ( 0 );   
      break;      

    default:      
      break;
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
	win.title = "OpenGL/GLUT Window.";
	win.field_of_view_angle = 45;
	win.z_near = 1.0f;
	win.z_far = 500.0f;

	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	Init();
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );  // Display Mode
	glutInitWindowSize(win.width,win.height);					// set window size
	glutCreateWindow(win.title);								// create Window
	glutDisplayFunc(display);									// register Display Function
	glutTimerFunc(0, Timer, 0);
//	glutIdleFunc( display );									// register Idle Function
        glutKeyboardFunc( keyboard );								// register Keyboard Handler
	initialize();
	glutMainLoop();												// run GLUT mainloop
	
	return 0;
}
