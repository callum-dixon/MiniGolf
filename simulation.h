/*-----------------------------------------------------------
  Simulation Header File
  -----------------------------------------------------------*/
#include"vecmath.h"
#include <vector>
#include "Player.h"

/*-----------------------------------------------------------
  Macros
  -----------------------------------------------------------*/
#define TABLE_X			(0.6f) 
#define TABLE_Z			(1.2f)
#define TABLE_Y			(0.1f)
#define BALL_RADIUS		(0.02f)
#define BALL_MASS		(0.1f)
#define TWO_PI			(6.2832f)
#define	SIM_UPDATE_MS	(10)
#define NUM_BALLS		(1)
#define NUM_CUSHIONS	(4)
#define NUM_GOALS		(2)
#define NUM_PLAYERS		(2)
#define NUM_COURSES		(3)
#define MAX_PARTICLES	(2000)
#define GOAL_SCORED		(0)

/*-----------------------------------------------------------
  plane normals
  -----------------------------------------------------------*/
/*
extern vec2	gPlaneNormal_Left;
extern vec2	gPlaneNormal_Top;
extern vec2	gPlaneNormal_Right;
extern vec2	gPlaneNormal_Bottom;
*/

/*-----------------------------------------------------------
  cushion class
  -----------------------------------------------------------*/
class cushion
{
public:
	vec2	vertices[2]; //2d
	vec2	centre;
	vec2	normal;

	void MakeNormal(void);
	void MakeCentre(void);
};

/*-----------------------------------------------------------
goal class
-----------------------------------------------------------*/
class goal
{
public:
	// Public member variables.
	vec2	position;
	float	radius;
	vec2	velocity;
	float	mass;


	goal() : position(0.0), radius(BALL_RADIUS), velocity(0.0), mass(BALL_MASS)
	{
	}
};

/*-----------------------------------------------------------
  ball class
  -----------------------------------------------------------*/
class ball
{
	static int ballIndexCnt;
public:
	vec2	position;
	vec2	velocity;
	float	radius;
	float	mass;
	int		index;
	vec2	player_1_pos;
	vec2	player_2_pos;

	ball(): position(0.0), velocity(0.0), radius(BALL_RADIUS), 
		mass(BALL_MASS) {index = ballIndexCnt++; Reset(), player_1_pos(0.0), player_2_pos(0.0);}
	
	void Reset(void);
	void ApplyImpulse(vec2 imp);
	void ApplyFrictionForce(int ms);
	void DoPlaneCollision(const cushion &c);
	void DoGoalCollision(goal &goal);
	void Update(int ms);
	
	bool HasHitPlane(const cushion &c) const;
	bool HasHitGoal(const goal &goal) const;

	void HitPlane(const cushion &c);
	void HitGoal();
};


/*-----------------------------------------------------------
particle class
-----------------------------------------------------------*/
class particle 
{
public:
	vec3 position;
	vec3 velocity;

	particle() {position=0;velocity=0;}
	void update(int ms);
};

class particleSet 
{
public:
	particle *particles[MAX_PARTICLES];
	int num;

	particleSet()
	{
		for(int i=0;i<MAX_PARTICLES;i++) particles[i] = 0;
		num=0;
	}

	~particleSet()
	{
		for(int i=0;i<MAX_PARTICLES;i++)
		{
			if(particles[i]) delete particles[i];
		}
	}

	void AddParticle(const vec3 &pos);
	void update(int ms);
};

/*-----------------------------------------------------------
  table class
  -----------------------------------------------------------*/
class table
{
public:
	ball balls[NUM_BALLS];	
	cushion cushions[NUM_CUSHIONS];
	goal goals[NUM_GOALS];
	particleSet parts;

	float numberOfRedBallsLeft;
	float numberOfYellowBallsLeft;
	bool isWhitePotted;
	bool isBlackPotted;
	int currentPlayerIndex;
	int nextPlayerIndex;
	int extraShotModifier;
	bool playerUsedCue;
	bool swapPlayers;

	int course;

	int player_1_score;
	int player_2_score;
	int winningPlayer;

	std::vector<Player*> players;

	void SetupCushions(void);
	void SetupGoals(void);
	void SetupPlayers();
	void Update(int ms);	
	bool AnyBallsMoving(void) const;
	void SwapPlayers(void);

	void GameOver(void);

	// Constructors and Deconstructors.
	table(void);
	~table(void);
};

/*-----------------------------------------------------------
  global table
  -----------------------------------------------------------*/
extern table gTable;
