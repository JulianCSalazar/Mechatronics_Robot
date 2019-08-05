#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "RenTargeting.h"
#include "Movement.h"
#include "TapeSensors.h"

#define FUCK_ME_UP_FAM 1300

typedef enum {
    Find_Tape,
    Following,
    Centering,
    Cornering,
    Turning,
} RenTargetingState_t;

static const char *StateNames[] = {
	"Find_Tape",
	"Following",
	"Centering",
	"Cornering",
	"Turning",
};

static RenTargetingState_t CurrentState = Find_Tape; // <- change name to match ENUM
static uint8_t MyPriority;

uint8_t InitRenTargeting(void) {
    ES_Event returnEvent;

    if ((Tape_Read() & 0x3) != 0x0) {
        CurrentState = Following;
    } else {
        CurrentState = Find_Tape;
    }
    returnEvent = RunRenTargeting(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunRenTargeting(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE; // use to flag transition
    RenTargetingState_t nextState; // <- change type to correct enum

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
        case Find_Tape:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TankTurnCCW(HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) {
                        Move(FULL_STOP, FULL_STOP);
                        nextState = Following;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Following:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(HALF_SPEED, HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == 0) {
                        nextState = Cornering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((ThisEvent.EventParam & FrontLeft) == 0) {
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(THREE_QUARTER_SPEED, ONE_QUARTER_SPEED);
                    } else if ((ThisEvent.EventParam & FrontRight) == 0) {
                        nextState = Centering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(ONE_QUARTER_SPEED, THREE_QUARTER_SPEED);
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Centering: // Tank Turns until Forward Left and Back Right tape sensors are triggered
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) {
                        nextState = Following;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if ((ThisEvent.EventParam & Front) == 0) {
                        nextState = Cornering;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    Move(HALF_SPEED, HALF_SPEED);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case Cornering: // Moves at quarter speed until the middle tape sensor is off
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_NO_EVENT;
                    Move(ONE_QUARTER_SPEED, ONE_QUARTER_SPEED);
                    break;
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & middle) == 0) {
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
        case Turning:
            switch (ThisEvent.EventType) {
                case TAPE_EVENT:
                    if ((ThisEvent.EventParam & Front) == Front) {
                        nextState = Following;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == OH_SHIT_TIMER) {
                        nextState = Find_Tape;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    Move(HALF_SPEED, HALF_SPEED);
                    break;
            }
            break;
    }
    if (makeTransition == TRUE) { // making a state transition, send EXIT and ENTRY
        // recursively call the current state with an exit event
        RunRenTargeting(EXIT_EVENT); // <- rename to your own Run function
        CurrentState = nextState;
        RunRenTargeting(ENTRY_EVENT); // <- rename to your own Run function
    }
    ES_Tail(); // trace call stack end
    return ThisEvent;
}