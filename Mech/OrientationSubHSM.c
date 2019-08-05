#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "OrientationSubHSM.h"
#include "Movement.h"

#define TURN_AROUND_TIMER HSM_TIMER
#define TURN_AROUND_TIME 1700
#define FULL_ROTATION_TIME 3400

typedef enum {
	LookingForBeacon,
	FaceTape,
	GoToTape,
} OrientationSubHSMState_t;

static const char *StateNames[] = {
	"LookingForBeacon",
	"FaceTape",
	"GoToTape",
};

static uint8_t MyPriority;
static OrientationSubHSMState_t CurrentState;

uint8_t InitOrientationSubHSM(void){
	ES_Event returnEvent;
	
	CurrentState = LookingForBeacon;
	returnEvent = RunOrientationSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunOrientationSubHSM(ES_Event ThisEvent) {
	uint8_t makeTransition = FALSE;
	OrientationSubHSMState_t nextState;
	
	ES_Tattle();
	
	switch(CurrentState){
//----------------------------------------------------------------------------
		case LookingForBeacon:
			switch(ThisEvent.EventType){
				case ES_INIT:
                    TankTurnCW(HALF_SPEED);
                    ES_Timer_InitTimer(TURN_AROUND_TIMER, TURN_AROUND_TIME);
					ThisEvent.EventType = ES_NO_EVENT;
					break;
				case BEACON_FOUND:
					makeTransition = TRUE;
					nextState = FaceTape;
					ThisEvent.EventType = ES_NO_EVENT;
					break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == TURN_AROUND_TIMER){
                        makeTransition = TRUE;
                        nextState = FaceTape;
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
		case FaceTape:
			switch(ThisEvent.EventType){
				case ES_ENTRY:
					ES_Timer_InitTimer(TURN_AROUND_TIMER, TURN_AROUND_TIME);
                    TankTurnCW(HALF_SPEED);
					ThisEvent.EventType = ES_NO_EVENT;
					break;
				case ES_TIMEOUT:
					if (ThisEvent.EventParam == TURN_AROUND_TIMER){
						makeTransition = TRUE;
						nextState = GoToTape;
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
		case GoToTape:
			switch(ThisEvent.EventType){
				case ES_ENTRY:
					Move(HALF_SPEED, HALF_SPEED);
					ThisEvent.EventType = ES_NO_EVENT;
					break;
				case ES_EXIT:
					ThisEvent.EventType = ES_NO_EVENT;
					break;
			}
			break;
//----------------------------------------------------------------------------
		default:
			break;
	}// end switch statement
	
	
   if (makeTransition == TRUE) {
       RunOrientationSubHSM(EXIT_EVENT);
       CurrentState = nextState;
       RunOrientationSubHSM(ENTRY_EVENT);
   }

    ES_Tail();
    return ThisEvent;
}//end function