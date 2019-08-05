#define FULL_SPEED 800
#define THREE_QUARTER_SPEED 600
#define HALF_SPEED 400
#define ONE_QUARTER_SPEED 200
#define FULL_STOP 0

/**Sets up ports to turn motors**/
void Movement_Init(void);

/** Turns motors. Takes in two ints between 0 and 1000 dictating speed
	of left and right motors, respectively **/
void Move(int LeftWheelSpeed, int RightWheelSpeed);

/**Turns right motor. Takes values between 0 and 1000 **/
void MoveRightWheel(int speed);

/**Turns left motor. Takes values between 0 and 1000 **/
void MoveLeftWheel(int speed);

/**Turns left motor. Takes values between 0 and 1000 **/
void TankTurnCW(int speed);

/**Turns left motor. Takes values between 0 and 1000 **/
void TankTurnCCW(int speed);

/**Jitters to correct place on tape **/
void Jitter(char direction);

/**Evade, goes around object **/
void Evader(char direction);