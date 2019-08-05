#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "EvadeSubHSM.h"
#include "Movement.h"
#include "Shooter.h"
#include "Ren_Evasion.h"
#include "TapeSensors.h"
#include "RC_Servo.h"

typedef enum {
    BackUp,
    RightTurn,
    RainbowTurn,
    LookingForMid,
    Centering,
    Smashing,
    BackUpFromSmash,
    LineUp,
    Shoot,
    LoadBall,
    Wait,
    SmooshIn,
} RenEvadeSubHSMState_t;

static const char *StateNames[] = {
	"BackUp",
	"RightTurn",
	"RainbowTurn",
	"LookingForMid",
	"Centering",
	"Smashing",
	"BackUpFromSmash",
	"LineUp",
	"Shoot",
	"LoadBall",
	"Wait",
	"SmooshIn",
};

#define BACK_UP_TIMER HSM_TIMER_2
#define RIGHT_TURN_TIMER HSM_TIMER_3
#define BACK_UP_TIME 500
#define SMASH_BACK_UP_TIME 1000
#define RIGHT_TURN_TIME 550
#define PERPENDICULAR_TIME 800
#define T_TAPE_TIME 1500
#define IGNORE_TIME 300

#define LINEUP_TIME 2000

#define LOADING_TIMER HSM_TIMER_5
#define LOADING_TIME 400

#define TA_TIMER HSM_TIMER_6
#define TA_WAITING_TIME 4000

#define EVASION_MIN_TIMER HSM_TIMER
#define EVASION_MIN_TIME 750

static RenEvadeSubHSMState_t CurrentState = BackUp; // <- change name to match ENUM
static uint8_t MyPriority;
static int oneSecondFlag = FALSE;
static int SmashCounter = 0;

uint8_t Init_Ren_Evasion(void) {
    ES_Event returnEvent;
    CurrentState = BackUp;
    returnEvent = Run_Ren_Evasion(INIT_EVENT);

    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

//Basically we will check the tape sensors to see if we're looking at an obstacle
// or the REN ship. If it's the obstacle we will copy the Evasion SUB HSM and if
// it isnt an obstacle then we will hard code a maneuver to line up to the REN ship

ES_Event Run_Ren_Evasion(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    RenEvadeSubHSMState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case BackUp: // We back up from hitting the side of the Ren target
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    SmashCounter = 0;
                    ES_Timer_InitTimer(BACK_UP_TIMER, BACK_UP_TIME);
                    Move(-HALF_SPEED, -HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BACK_UP_TIMER) {
                        makeTransition = TRUE;
                        nextState = RightTurn;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    Move(FULL_STOP, FULL_STOP);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case RightTurn: // We tank turn right 90 degrees after hitting the side of the ren target
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(RIGHT_TURN_TIMER, RIGHT_TURN_TIME);
                    TankTurnCW(HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == RIGHT_TURN_TIMER) {
                        makeTransition = TRUE;
                        nextState = RainbowTurn;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case BUMPED:
                    ThisEvent.EventType = ES_NO_EVENT;
                    makeTransition = TRUE;
                    nextState = BackUp;
                    break;
                case ES_EXIT:
                    Move(FULL_STOP, FULL_STOP);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case RainbowTurn: // Positions us on the T tape
            switch (ThisEvent.EventType) {
                case ES_ENTRY:

                    Move(ONE_QUARTER_SPEED, 375);
                    ES_Timer_InitTimer(EVASION_MIN_TIMER, EVASION_MIN_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((oneSecondFlag == TRUE) && ((ThisEvent.EventParam & Front) == Front)) {
                        nextState = LookingForMid;
                        makeTransition = TRUE;
                        oneSecondFlag = FALSE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        InitTapeFollowSubHSM();
                    }
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == EVASION_MIN_TIMER) {
                        oneSecondFlag = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LookingForMid: // Moves forward at quarter of full speed until middle tape sensor is on tape
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(ONE_QUARTER_SPEED, ONE_QUARTER_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == 0) { // We wait for the front to be off
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT: // Begins tank turning clockwise to align itself w/ tape
                    TankTurnCCW(450);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Centering: // Moves until front two tape sensors are on tape, indicating alignment with tape
            switch (ThisEvent.EventType) {
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) { // Front tape sensors on tape cause state transition to Following
                        nextState = Smashing;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(FULL_STOP, FULL_STOP);
                    }
                    break;
                case ES_EXIT:
                    Move(600, 600);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case Smashing: // This is the smashing (british accent) state, where we crash into the front of ren
            switch (ThisEvent.EventType) {
                case BUMPED:
                    nextState = SmooshIn;
                    ThisEvent.EventType = ES_NO_EVENT;
                    makeTransition = TRUE;
                    SmashCounter++;
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;

        case SmooshIn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(BACK_UP_TIMER, SMASH_BACK_UP_TIME);
                    Move(FULL_SPEED, FULL_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BACK_UP_TIMER) {
                        if (SmashCounter < 2) {
                            nextState = BackUpFromSmash;
                        } else {
                            nextState = LineUp;
                            ES_Timer_InitTimer(LINEUP_TIMER, LINEUP_TIME);
                        }
                        ThisEvent.EventType = ES_NO_EVENT;
                        makeTransition = TRUE;
                    }
                    break;

                case ES_EXIT:
                    Move(FULL_STOP, FULL_STOP);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;

        case BackUpFromSmash:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(BACK_UP_TIMER, SMASH_BACK_UP_TIME);
                    Move(-200, -200);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == BACK_UP_TIMER) {
                        makeTransition = TRUE;
                        nextState = Smashing;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    Move(500, 510);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LineUp:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(FULL_STOP, FULL_STOP);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == LINEUP_TIMER) {
                        makeTransition = TRUE;
                        nextState = LoadBall;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                case ES_EXIT:
                    Move(FULL_STOP, FULL_STOP);
                    RC_SetPulseTime(RC_PORTY07, 1700);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
        case LoadBall:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(FULL_STOP, FULL_STOP);
                    ES_Timer_InitTimer(LOADING_TIMER, LOADING_TIME);
                    LAS_Retract();
                    ThisEvent.EventType = ES_NO_EVENT;
                    Shooter_On(250);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == LOADING_TIMER) {
                        ThisEvent.EventType = ES_NO_EVENT;
                        makeTransition = TRUE;
                        nextState = Wait;
                        LAS_Extend();
                    }
                    break;
            }
            break;
        case Wait:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(TA_TIMER, TA_WAITING_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    //TargetsHit++;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TA_TIMER) {
                        makeTransition = TRUE;
                        nextState = LoadBall;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    //Shooter_On(0);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;

            }
            break;
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        Run_Ren_Evasion(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        Run_Ren_Evasion(ENTRY_EVENT); // <- rename to your own Run function
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}