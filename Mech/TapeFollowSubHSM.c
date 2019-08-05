#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "ATM6Targeting.h"
#include "TapeFollowSubHSM.h"
#include "Movement.h"
#include "TapeSensors.h"

#define FUCK_ME_UP_FAM 1300

typedef enum {
    LookingForMid,
    Centering,
    Following,
    Correcting,
    Cornering,
    Turning,

} TapeFollowSubHSMState_t;

static const char *StateNames[] = {
	"LookingForMid",
	"Centering",
	"Following",
	"Correcting",
	"Cornering",
	"Turning",
};

static TapeFollowSubHSMState_t CurrentState;
static uint8_t MyPriority;
static int minTimeFlag = FALSE;
extern int oh_Shit;

uint8_t InitTapeFollowSubHSM(void) {
    ES_Event returnEvent;

    CurrentState = LookingForMid;
    returnEvent = RunTapeFollowSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

// This function initializes the function similarly to InitTapeFollowSubHSM(), but starts in Centering state.
// Use this function on the transition from the ATM6Targeting state of the Top HSM to the RenTargeting state,
// after it detects the beacon, and hit its third target.
// Also, use when transitioning from KillATM6 subHSM to back to this HSM in the ATM6TargetingSubHSM.
uint8_t AlternativeInitTapeFollowSubHSM(void) {
	ES_Event returnEvent;
	
	CurrentState = Following;
	returnEvent = RunTapeFollowSubHSM(INIT_EVENT);
	if (returnEvent.EventType == ES_NO_EVENT) {
		return TRUE;
	}
	return FALSE;
}

ES_Event RunTapeFollowSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    TapeFollowSubHSMState_t nextState;

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
            //----------------------------------------------------------------------------
        case LookingForMid: // Moves forward at quarter of full speed until middle tape sensor is on tape
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(ONE_QUARTER_SPEED, ONE_QUARTER_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    ES_Timer_InitTimer(OH_SHIT_TIMER, FUCK_ME_UP_FAM);
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & middle) == middle) { // Middle tape sensor on tape triggers transition to Centering
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT: // Begins tank turning clockwise to align itself w/ tape
                    TankTurnCW(HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Centering: // Moves until front two tape sensors are on tape, indicating alignment with tape
            switch (ThisEvent.EventType) {
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) { // Front tape sensors on tape cause state transition to Following
                        nextState = Following;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((ThisEvent.EventParam & Front) == 0) { // Neither front tape sensors on tape -> must be on corner. State transition to cornering
                        nextState = Cornering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Following:
            switch (ThisEvent.EventType) { // Starts following tape at half speed
                case ES_ENTRY:
                    Move(THREE_QUARTER_SPEED, THREE_QUARTER_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == 0) { // Both front tape sensor go off tape, must have hit corner. State transition to cornering
                        nextState = Cornering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((ThisEvent.EventParam & FrontLeft) == 0) { // Adjust to get left side back on tape. State transition to Centering.
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(HALF_SPEED, ONE_QUARTER_SPEED);
                    } else if ((ThisEvent.EventParam & FrontRight) == 0) { // Adjust to get right side back on tape. State transition to Centering.
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(ONE_QUARTER_SPEED, HALF_SPEED);
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Cornering: // Moves at quarter speed until the middle tape sensor is off tape.
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_NO_EVENT;
                    Move(ONE_QUARTER_SPEED, ONE_QUARTER_SPEED);
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & middle) == 0) { // When middle tape sensor is off tape, tank turn to move back on tape. State transition to Turning.
                        nextState = Turning;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    TankTurnCW(HALF_SPEED);
                    ES_Timer_InitTimer(OH_SHIT_TIMER, FUCK_ME_UP_FAM);
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Turning: // Tank turns until front tape sensors are back on tape, to begin following again.
            switch (ThisEvent.EventType) {
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) { // Front tape sensors are on tape --> state transition to Following.
                        nextState = Following;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == OH_SHIT_TIMER){
                        nextState = LookingForMid;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        default: // all unhandled states fall into here
            break;
    } // end switch on Current State

    if (makeTransition == TRUE) {

        RunATM6Targeting(EXIT_EVENT);
        CurrentState = nextState;
        RunATM6Targeting(ENTRY_EVENT);
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}
