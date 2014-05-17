#define MAXBOIDS 100
#define MAXSELECT 100
#define MAXFEED 300
#define SOLID 1
#define LINE 2
#define POINT 3
#define LOCAL_RADIUS 50
#define MAX_SPEED 3

struct vector {
	float x;
	float y;
} vector;

struct triangle {
	struct vector p1;
	struct vector p2;
	struct vector p3;
	struct vector centre;
} triangle;

struct boid {
	struct triangle t;
	struct vector velocity;

	float v1[2];
	float v2[2];
	float v3[2];

	float color[3];
	float rotate;

	float r1[2];
	float r2[2];
	float r3[2];
	float r4[2];	
} boids[MAXBOIDS];

static void Timer(int value);
struct vector * MoveTowardsCentre(struct boid *b);
struct vector * LimitSpeed(struct boid *b);
static void KeepDistance(void);
struct vector * MatchVelocity(struct boid *b);
void VectorAdd(struct vector *ret, struct vector *a, struct vector *b);
void VectorDivide(struct vector *ret, struct vector *a, float scalar);
struct vector * StayInBounds(struct boid *b);
int check_within_radius(struct vector *centre, struct vector *v);
