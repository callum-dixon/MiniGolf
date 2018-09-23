// Include Guard, helps prevent recursive inclusion.
#ifndef Player_h_included
#define Player_h_included

// Globals Variables for the Player objects.
const auto NUMBER_OF_PLAYERS = 2;

class Player
{
public:
	// Public member variables.
	float numberOfShots;
	int ballColour;
	bool isBlackPotted;
	bool hitABall;

	// Constructors and Deconstructors
	Player(void);
	~Player(void);

	// Public Member Function Declarations
	void Reset(void);
};
#endif