/******************************************************************************
 * File: HSM.c
 * Authors: Julian Salazar / Ibrahim Al Hanich / Rory Grant
 *
 * Notes:
 * 	- This file is the top level of our HSM for the control of our robot.
 *	- This file layout and functionality is based on a file provided by Prof.
 *		Gabe Elkaim and Max Dunne. All function content is original to JS/IH/RG
 *		and is used exclusively for CMPE118L Fall 2017
 *
 *****************************************************************************/

 /*****************************************************************************
 * INCLUDED FILES
 *****************************************************************************/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "ATM6Targeting.h"
#include "OrientationSubHSM.h"
#include "KillATM6SubHSM.h"
#include "EvadeSubHSM.h"
#include "TapeSensors.h"
#include "RenTargeting.h"
#include "Ren_Evasion.h"
#include "Movement.h"
#include "LAS.h"
#include "TapeFollowSubHSM.h"
#include "FronTapeSensor.h"
#include "RC_Servo.h"

 /*****************************************************************************
 * DEFINITIONS
 *****************************************************************************/
#define EVASION_MIN_TIMER HSM_TIMER
#define OneSecBoi OneSecTimer
#define EVASION_MIN_TIME 750
#define IGNORE_BEACON_TIMER HSM_TIMER1
#define IGNORE_BEACON_TIME 30000
#define White 1
#define Black 0

 /*****************************************************************************
 * ENUMERATIONS
 *****************************************************************************/
typedef enum {
    InitState,
    Orienting,
    ATM6TargetingTop,
    Evade,
    RenTargeting,
    REN_ALIGNMENT,
    REN_LINEUP,
    RETRACT,
    EXTEND,
} HSMState_t;

static const char *StateNames[] = {
	"InitState",
	"Orienting",
	"ATM6TargetingTop",
	"Evade",
	"RenTargeting",
	"REN_ALIGNMENT",
	"REN_LINEUP",
	"RETRACT",
	"EXTEND",
};

 /*****************************************************************************
 * PRIVATE VARIABLES
 *****************************************************************************/
int RenFlag = FALSE;
int ObjectColor = Black;
unsigned int TapeThreshold;

static HSMState_t CurrentState;
static uint8_t MyPriority;
static int oneSecondFlag = FALSE;
static int beaconIgnore = FALSE;
static int renTargetingFlag = FALSE;
int TargetsHit = 0;

 /*****************************************************************************
 * FUNCTIONS
 *****************************************************************************/
/*
 * Function: InitHSM()
 * Parameters: uint8_t Priority
 * Returns: TRUE or FALSE
 * Description: Initializes the HSM for usage within the main function
 */
uint8_t InitHSM(uint8_t Priority)
{
    MyPriority = Priority;
    CurrentState = InitState;
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

uint8_t PostHSM(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

ES_Event RunHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    HSMState_t nextState;

    ES_Tattle();

    switch (CurrentState) {
//        case RETRACT:
//            switch (ThisEvent.EventType) {
//                case ES_INIT:
//                    ThisEvent.EventType = ES_NO_EVENT;
//                case ES_ENTRY:
//                    ES_Timer_InitTimer(HSM_TIMER_5, 400);
//                    LAS_Retract();
//                    break;
//                case ES_TIMEOUT:
//                    if (ThisEvent.EventParam == HSM_TIMER_5) {
//                        makeTransition = TRUE;
//                        nextState = EXTEND;
//                    }
//                    break;
//                case ES_EXIT:
//                    break;
//            }
//            break;
//
//        case EXTEND:
//            switch (ThisEvent.EventType) {
//                case ES_ENTRY:
//                    ES_Timer_InitTimer(HSM_TIMER_5, 2000);
//                    LAS_Extend();
//                    break;
//                case ES_TIMEOUT:
//                    if (ThisEvent.EventParam == HSM_TIMER_5) {
//                        makeTransition = TRUE;
//                        nextState = RETRACT;
//                    }
//                    break;
//                case ES_EXIT:
//                    break;
//            }
//            break;

            //--------------------------------------------------------------------------
        case InitState:
            if (ThisEvent.EventType == ES_INIT) {
                InitATM6Targeting();
                InitOrientationSubHSM();
                nextState = Orienting;
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;
            }
            break;
            //--------------------------------------------------------------------------
        case Orienting:
            ThisEvent = RunOrientationSubHSM(ThisEvent);
            switch (ThisEvent.EventType) {
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) {
                        nextState = ATM6TargetingTop;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        InitTapeFollowSubHSM();
                    }
                    break;
                case BUMPED:
                    nextState = Evade;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    InitEvadeSubHSM();
                    ES_Timer_InitTimer(EVASION_MIN_TIMER, EVASION_MIN_TIME);
                    break;
            }
            break;
            //--------------------------------------------------------------------------
        case ATM6TargetingTop:
            ThisEvent = RunATM6Targeting(ThisEvent);

            switch (ThisEvent.EventType) {
                case BUMPED:
                    nextState = Evade;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    InitEvadeSubHSM();
                    ES_Timer_InitTimer(EVASION_MIN_TIMER, EVASION_MIN_TIME);
                    break;
                case BEACON_FOUND:
                    if (TargetsHit >= 3) {
                        nextState = RenTargeting;
                        RenFlag = TRUE;
                        InitRenTargeting();
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }

            break;
            //--------------------------------------------------------------------------
        case Evade:
            ThisEvent = RunEvadeSubHSM(ThisEvent);

            switch (ThisEvent.EventType) {
                case ES_TIMEOUT:
                    if ((ThisEvent.EventParam == EVASION_MIN_TIMER) || (ThisEvent.EventParam == OneSecBoi)) {
                        oneSecondFlag = TRUE;
                    }
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((oneSecondFlag == TRUE) && ((ThisEvent.EventParam & Front) == Front)) {
                        if (RenFlag == FALSE) {
                            nextState = ATM6TargetingTop;
                            makeTransition = TRUE;
                            oneSecondFlag = FALSE;
                            ThisEvent.EventType = ES_NO_EVENT;
                            InitTapeFollowSubHSM();
                        } else {
                            nextState = RenTargeting;
                            makeTransition = TRUE;
                            oneSecondFlag = FALSE;
                            ThisEvent.EventType = ES_NO_EVENT;
                            InitRenTargeting();
                        }
                    }

                    break;
            }
            break;
            //--------------------------------------------------------------------------
        case RenTargeting:
            ThisEvent = RunRenTargeting(ThisEvent);

            switch (ThisEvent.EventType) {

                case BUMPED:
                    ObjectColor = ReadObject();
                    if (ObjectColor == Black) {
                        nextState = Evade;
                        //nextState = REN_ALIGNMENT;
                        makeTransition = TRUE;
                        //Init_Ren_Evasion();
                        ES_Timer_InitTimer(OneSecBoi, EVASION_MIN_TIME);
                        InitEvadeSubHSM();
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else {
                        nextState = REN_ALIGNMENT;
                        Init_Ren_Evasion();
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
        case REN_ALIGNMENT:
            ThisEvent = Run_Ren_Evasion(ThisEvent);
            break;
            //---------------------------------------------------------------------------
        default:
            break;

    } //end switch statement

    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunHSM(ENTRY_EVENT);
    }

    ES_Tail(); // trace call stack end
    return ThisEvent;
}