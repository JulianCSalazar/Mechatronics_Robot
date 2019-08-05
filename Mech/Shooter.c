#include "Shooter.h"
#include "xc.h"

void Shooter_Init(void){
    PWM_AddPins(PWM_PORTZ06);
    IO_PortsSetPortOutputs(PORTZ, PIN6);
}
void Shooter_On(int speed){
    PWM_SetDutyCycle(PWM_PORTZ06, speed);
}

void Shooter_Off(void){
    PWM_SetDutyCycle(PWM_PORTZ06, 0);
}
