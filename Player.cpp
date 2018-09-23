// Header file Includes.
#include "Player.h"

// Player Class Definitions
Player::Player(void)
{
	numberOfShots = 0;
	isBlackPotted = false;
	hitABall = false;
}

Player::~Player(void)
{

}

void Player::Reset(void)
{
	// Resets the instance of the Player.
	numberOfShots = 0;
	isBlackPotted = false;
	hitABall = false;
}