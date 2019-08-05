#include "BOARD.h"
#include "pwm.h"
#include "IO_Ports.h"
#include "Movement.h"

static struct Motor{
    int R;
    int L;
}Motors;

/**Sets up ports to turn motors**/
void Movement_Init(void){
	PWM_AddPins(PWM_PORTY10 | PWM_PORTY12);
	
	IO_PortsSetPortOutputs(PORTY, PIN9 | PIN11);
	IO_PortsSetPortBits(PORTY, PIN9 | PIN11);
	
	PWM_SetDutyCycle(PWM_PORTY10, FULL_STOP);
	PWM_SetDutyCycle(PWM_PORTY12, FULL_STOP);
}

/** Turns motors. Takes in two ints between 0 - 1000 dictating speed
	of left and right motors, respectively **/
void Move(int LeftWheelSpeed, int RightWheelSpeed){
	MoveLeftWheel(LeftWheelSpeed);
	MoveRightWheel(RightWheelSpeed);
    Motors.L = LeftWheelSpeed;
    Motors.R = RightWheelSpeed;
}

/**Turns right motor. Takes values between 0 and 1000 **/
void MoveRightWheel(int speed){
	if (speed < 0) {
		IO_PortsSetPortBits(PORTY, PIN9);
        speed = -speed;
	} else {
		IO_PortsClearPortBits(PORTY, PIN9);
	}
    Motors.R = speed;
	PWM_SetDutyCycle(PWM_PORTY10, speed);
}

/**Turns left motor. Takes values between 0 and 1000 **/
void MoveLeftWheel(int speed){
	if (speed < 0) {
		IO_PortsSetPortBits(PORTY, PIN11);
        speed = -speed;
	} else {
		IO_PortsClearPortBits(PORTY, PIN11);
	}
	Motors.L = speed;
	PWM_SetDutyCycle(PWM_PORTY12, speed);
}

/**Turns left motor. Takes values between 0 and 1000 **/
void TankTurnCW(int speed){
    MoveLeftWheel(speed);
    MoveRightWheel(-speed);
    Motors.L = speed;
    Motors.R = -speed;
}

/**Turns left motor. Takes values between 0 and 1000 **/
void TankTurnCCW(int speed){
    MoveLeftWheel(-speed);
    MoveRightWheel(speed);
    Motors.L = -speed;
    Motors.R = speed;
}

/**Jitters to correct place on tape **/
void Jitter(char direction){
    if(direction == 'L'){
        MoveLeftWheel(850);
        Motors.L -=150;
    }else{
        MoveRightWheel(850);
        Motors.R -=150;
    }
}

/**Evade, goes around object **/
void Evader(char direction){
    
}