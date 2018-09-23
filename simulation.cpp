/*-----------------------------------------------------------
  Simulation Source File
  -----------------------------------------------------------*/
#include"stdafx.h"
#include"simulation.h"
#include<stdio.h>
#include<stdlib.h>
#include <iostream>
#include"glut.h"

/*-----------------------------------------------------------
  macros
  -----------------------------------------------------------*/
#define SMALL_VELOCITY		(0.01f)

/*-----------------------------------------------------------
  globals
  -----------------------------------------------------------*/
/*
vec2	gPlaneNormal_Left(1.0,0.0);
vec2	gPlaneNormal_Top(0.0,1.0);
vec2	gPlaneNormal_Right(-1.0,0.0);
vec2	gPlaneNormal_Bottom(0.0,-1.0);
*/

table gTable;

int course = 0;

float gCoeffRestitution = 0.5f;
float gCoeffFriction = 0.03f;
float gGravityAccn = 9.8f;

bool player_1_scored = false;
bool player_2_scored = false;

int player_1_score = 0;
int player_2_score = 0;

/*-----------------------------------------------------------
  cushion class members
  -----------------------------------------------------------*/
void cushion::MakeNormal(void)
{
	//can do this in 2d
	vec2 temp = vertices[1]-vertices[0];
	normal(0) = temp(1);
	normal(1) = -temp(0);
	normal.Normalise();
}

void cushion::MakeCentre(void)
{
	centre = vertices[0];
	centre += vertices[1];
	centre/=2.0;
}

/*-----------------------------------------------------------
  ball class members
  -----------------------------------------------------------*/
int ball::ballIndexCnt = 0;

void ball::Reset(void)
{
	//set velocity to zero
	velocity = 0.0;

	//work out rack position
	if(index==0)
	{
		position(1) = 1;
		position(0) = 0.0;
		return;
	}
	
	static const float sep = (BALL_RADIUS*3.0f);
	static const float rowSep = (BALL_RADIUS*2.5f);
	int row = 1;
	int rowIndex = index;
	while(rowIndex > row)
	{
		rowIndex -= row;
		row++;
	}
	position(0) = (((row-1)*sep)/2.0f) - (sep*(row-rowIndex));
	position(1) = -(rowSep * (row - 1));
	position(2) = 0.0, 0.0, 0.0;
}

void ball::ApplyImpulse(vec2 imp)
{
	if (gTable.currentPlayerIndex == 0)
	{
		gTable.player_1_score++;
	}
	else
	{
		gTable.player_2_score++;
	}
	velocity = imp;
}

void ball::ApplyFrictionForce(int ms)
{
	if(velocity.Magnitude()<=0.0) return;

	//accelaration is opposite to direction of motion
	vec2 accelaration = -velocity.Normalised();
	//friction force = constant * mg
	//F=Ma, so accelaration = force/mass = constant*g
	accelaration *= (gCoeffFriction * gGravityAccn);
	//integrate velocity : find change in velocity
	vec2 velocityChange = ((accelaration * ms)/1000.0f);
	//cap magnitude of change in velocity to remove integration errors
	if(velocityChange.Magnitude() > velocity.Magnitude()) velocity = 0.0;
	else velocity += velocityChange;
}

void ball::DoGoalCollision(goal &goal)
{
	if (HasHitGoal(goal)) HitGoal();
}

void ball::DoPlaneCollision(const cushion &b)
{
	if(HasHitPlane(b)) HitPlane(b);
}

void ball::Update(int ms)
{
	//apply friction
	ApplyFrictionForce(ms);
	//integrate position
	position += ((velocity * ms)/1000.0f);
	//set small velocities to zero
	if(velocity.Magnitude()<SMALL_VELOCITY) velocity = 0.0;
}

bool ball::HasHitPlane(const cushion &c) const
{
	//if moving away from plane, cannot hit
	if(velocity.Dot(c.normal) >= 0.0) return false;
	
	//if in front of plane, then have not hit
	vec2 relPos = position - c.vertices[0];
	double sep = relPos.Dot(c.normal);
	if(sep > radius) return false;
	return true;
}

bool ball::HasHitGoal(const goal &goal) const
{
	//work out relative position of ball from other ball,
	//distance between balls
	//and relative velocity
	vec2 relPosn = position - goal.position;
	float dist = (float)relPosn.Magnitude();
	vec2 relPosnNorm = relPosn.Normalised();
	vec2 relVelocity = velocity - goal.velocity;

	//if moving apart, cannot have hit
	if (relVelocity.Dot(relPosnNorm) >= 0.0) return false;
	//if distnce is more than sum of radii, have not hit
	if (dist > (radius + goal.radius)) return false;
	return true;
}

void ball::HitPlane(const cushion &c)
{
	//reverse velocity component perpendicular to plane  
	double comp = velocity.Dot(c.normal) * (1.0+gCoeffRestitution);
	vec2 delta = -(c.normal * comp);
	velocity += delta;

	//make some particles
	int n = (rand()%4)+3;
	vec3 pos(position(0),radius/2.0,position(1));
	vec3 oset(c.normal(0),0.0,c.normal(1));
	pos+=(oset*radius);
	for(int i=0;i<n;i++)
	{
		gTable.parts.AddParticle(pos);
	}
}

void ball::HitGoal()
{
	if (gTable.currentPlayerIndex == 0)
	{
		std::cout << "Player 1 Scored" << std::endl;
		Reset();
		gTable.SwapPlayers();
		player_1_scored = true;
	}
	else
	{
		std::cout << "Player 2 ended the game" << std::endl;
		player_2_scored = true;
		Reset();
		gTable.GameOver();
	}
}

/*-----------------------------------------------------------
  particle class members
  -----------------------------------------------------------*/
void particle::update(int ms)
{
	position += (velocity*ms)/1000.0;
	velocity(1) -= (4.0*ms)/1000.0; //(9.8*ms)/1000.0;
}

/*-----------------------------------------------------------
  particle set class members
  -----------------------------------------------------------*/
void particleSet::AddParticle(const vec3 &pos)
{
	if(num >= MAX_PARTICLES) return;
	particles[num] = new particle;
	particles[num]->position = pos;

	particles[num]->velocity(0) = ((rand() % 200)-100)/200.0;
	particles[num]->velocity(2) = ((rand() % 200)-100)/200.0;
	particles[num]->velocity(1) = 2.0*((rand() % 100)/100.0);

	num++;
}

void particleSet::update(int ms)
{
	int i=0;
	while(i<num)
	{
		particles[i]->update(ms);
		if((particles[i]->position(1) < 0.0) && (particles[i]->velocity(1)<0.0))
		{
			delete particles[i];
			particles[i] = particles[num-1];
			num--;
		}
		else i++;
	}
}

/*-----------------------------------------------------------
  table class members
  -----------------------------------------------------------*/
table::table(void)
{
	currentPlayerIndex = 0;
	nextPlayerIndex = 0;
	playerUsedCue = false;
	swapPlayers = false;
	winningPlayer = -1;
}

table::~table(void)
{
	for (int i = 0; i < players.size(); i++)
	{
		delete players[i];
	}
}

void table::SetupCushions(void)
{
	if (course == 0)
	{
		cushions[0].vertices[0](0) = -TABLE_X;
		cushions[0].vertices[0](1) = -TABLE_Z;
		cushions[0].vertices[1](0) = -TABLE_X;
		cushions[0].vertices[1](1) = TABLE_Z;

		cushions[1].vertices[0](0) = -TABLE_X;
		cushions[1].vertices[0](1) = TABLE_Z;
		cushions[1].vertices[1](0) = TABLE_X;
		cushions[1].vertices[1](1) = TABLE_Z;

		cushions[2].vertices[0](0) = TABLE_X;
		cushions[2].vertices[0](1) = TABLE_Z;
		cushions[2].vertices[1](0) = TABLE_X;
		cushions[2].vertices[1](1) = -TABLE_Z;

		cushions[3].vertices[0](0) = TABLE_X;
		cushions[3].vertices[0](1) = -TABLE_Z;
		cushions[3].vertices[1](0) = -TABLE_X;
		cushions[3].vertices[1](1) = -TABLE_Z;
	}

	if (course == 1)
	{
		cushions[0].vertices[0](0) = -TABLE_X + 0.5;
		cushions[0].vertices[0](1) = -TABLE_Z;
		cushions[0].vertices[1](0) = -TABLE_X ;
		cushions[0].vertices[1](1) = TABLE_Z - 1;

		cushions[1].vertices[0](0) = -TABLE_X;
		cushions[1].vertices[0](1) = TABLE_Z - 1;
		cushions[1].vertices[1](0) = TABLE_X - 0.6;
		cushions[1].vertices[1](1) = TABLE_Z;

		cushions[2].vertices[0](0) = TABLE_X - 0.6;
		cushions[2].vertices[0](1) = TABLE_Z;
		cushions[2].vertices[1](0) = TABLE_X;
		cushions[2].vertices[1](1) = -TABLE_Z + 1;

		cushions[3].vertices[0](0) = TABLE_X;
		cushions[3].vertices[0](1) = -TABLE_Z + 1;
		cushions[3].vertices[1](0) = -TABLE_X + 0.5;
		cushions[3].vertices[1](1) = -TABLE_Z;
	}

	if (course == 2)
	{
		cushions[0].vertices[0](0) = 0;
		cushions[0].vertices[0](1) = 0;
		cushions[0].vertices[1](0) = 0;
		cushions[0].vertices[1](1) = 0;

		cushions[1].vertices[0](0) = 0;
		cushions[1].vertices[0](1) = 0;
		cushions[1].vertices[1](0) = 0;
		cushions[1].vertices[1](1) = 0;

		cushions[2].vertices[0](0) = 0;
		cushions[2].vertices[0](1) = 0;
		cushions[2].vertices[1](0) = 0;
		cushions[2].vertices[1](1) = 0;

		cushions[3].vertices[0](0) = 0;
		cushions[3].vertices[0](1) = 0;
		cushions[3].vertices[1](0) = 0;
		cushions[3].vertices[1](1) = 0;
	}



	for(int i=0;i<NUM_CUSHIONS;i++)
	{
		cushions[i].MakeCentre();
		cushions[i].MakeNormal();
	}
}

void table::SetupGoals(void)
{
	if (course == 0)
	{
		goals[0].position = -0.5;
		goals[0].position(1) = 0;

		goals[1].position = 0.1;
		goals[1].position(1) = -0.1;
	}

	if (course == 1)
	{
		goals[0].position = -0.2;
		goals[0].position(1) = 0;

		goals[1].position = 0.4;
		goals[1].position(1) = 0;
	}

	if (course == 2)
	{
		goals[0].position = -10;
		goals[0].position(1) = -10;

		goals[1].position = -10;
		goals[1].position(1) = -10;
	}
}

void table::Update(int ms)
{
	//check for collisions for each ball
	for(int i=0;i<NUM_BALLS;i++) 
	{
		for(int j=0;j<NUM_CUSHIONS;j++)
		{
			balls[i].DoPlaneCollision(cushions[j]);
		}

		for (int j=0; j<NUM_GOALS; j++)
		{
			balls[i].DoGoalCollision(goals[j]);
		}
	}

	//update all balls
	for(int i=0;i<NUM_BALLS;i++) balls[i].Update(ms);

	//update particles
	parts.update(ms);

	//make some new particles
	//vec3 pos(0.0,BALL_RADIUS,0.0);
	//parts.AddParticle(pos);
}

bool table::AnyBallsMoving(void) const
{
	//return true if any ball has a non-zero velocity
	for(int i=0;i<NUM_BALLS;i++) 
	{
		if(balls[i].velocity(0)!=0.0) return true;
		if(balls[i].velocity(1)!=0.0) return true;
	}
	return false;
}

void table::SetupPlayers(void)
{
	// Setup each player.
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		players.push_back(new Player());
	}

	// If current player is 1, swap to player 2
	if (currentPlayerIndex == 0)
	{
		nextPlayerIndex = 1;
		players[currentPlayerIndex]->Reset();
		players[nextPlayerIndex]->Reset();
	}

	// If current player is 2, swap to player 1
	else if (currentPlayerIndex == 1)
	{
		nextPlayerIndex = 0;
		players[currentPlayerIndex]->Reset();
		players[nextPlayerIndex]->Reset();
	}
	swapPlayers = false;
}

void table::SwapPlayers(void)
{
	currentPlayerIndex++;
	nextPlayerIndex++;
		if (currentPlayerIndex == NUM_PLAYERS)
		{
			currentPlayerIndex = 0;
		}
		if (nextPlayerIndex == NUM_PLAYERS)
		{
			nextPlayerIndex = 0;
		}

		playerUsedCue = false;
		players[currentPlayerIndex]->hitABall = false;
		players[nextPlayerIndex]->hitABall = false;
		swapPlayers = false;
}

void table::GameOver(void)
{
	course++;
	gTable.SetupCushions();
	gTable.SetupGoals();
	gTable.SwapPlayers();
	player_1_scored = false;
	player_2_scored = false;
	std::cout << "course number:" << course << std::endl;
	if (course > 1)
	{
		gTable.SetupCushions();
		gTable.SetupGoals();
		if (player_1_score < player_2_score)
		{
			gTable.winningPlayer = 0;
		}
		else
		{
			gTable.winningPlayer = 1;
		}
		if (player_1_score == player_2_score)
		{
			gTable.winningPlayer = 2;
		}
	}
}
