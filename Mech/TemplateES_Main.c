#include <BOARD.h>
#include <xc.h>
#include <stdio.h>
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "RC_Servo.h"
#include "Shooter.h"

void main(void)
{
    ES_Return_t ErrorType;

    BOARD_Init();
    
    printf("Starting ES Framework Template\r\n");
    printf("using the 2nd Generation Events & Services Framework\r\n");


    // Your hardware initialization function calls go here
    AD_Init();
    PWM_Init();
    RC_Init();
    Shooter_Init();
    Movement_Init();
    Bumpers_Init();
    Beacon_Init();
    Tape_Init();
    LAS_Init();
    RC_AddPins(RC_PORTY07);
    RC_SetPulseTime(RC_PORTY07,500);
    Shooter_On(0);
    // now initialize the Events and Services Framework and start it running
    ErrorType = ES_Initialize();
    if (ErrorType == Success) {
        ErrorType = ES_Run();

    }
    //if we got to here, there was an error
    switch (ErrorType) {
    case FailedPointer:
        printf("Failed on NULL pointer");
        break;
    case FailedInit:
        printf("Failed Initialization");
        break;
    default:
        printf("Other Failure: %d", ErrorType);
        break;
    }
    for (;;)
        ;

};

///*------------------------------- Footnotes -------------------------------*/
///*------------------------------ End of file ------------------------------*/
