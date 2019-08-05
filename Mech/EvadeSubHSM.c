#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "EvadeSubHSM.h"
#include "Movement.h"
#include "Shooter.h"
#include "FronTapeSensor.h"

#define BACK_UP_TIMER HSM_TIMER_2
#define RIGHT_TURN_TIMER HSM_TIMER_3
#define BACK_UP_TIME 500
#define RIGHT_TURN_TIME 750
#define White 1
#define Black 0
#define ALIGNMENT_TIMER HSM_TIMER
#define ALIGNMENT_TIME 10000
#define FUCK_ME_UP_FAM 5000

typedef enum {
    BackUp,
    RightTurn,
    RainbowTurn,
} EvadeSubHSMState_t;

static const char *StateNames[] = {
	"BackUp",
	"RightTurn",
	"RainbowTurn",
};

static uint8_t MyPriority;
static EvadeSubHSMState_t CurrentState;
extern int RenFlag;
extern int ObjectColor;

uint8_t InitEvadeSubHSM(void) {
    ES_Event returnEvent;
    CurrentState = BackUp;
    returnEvent = RunEvadeSubHSM(INIT_EVENT);
    ES_Timer_InitTimer(ALIGNMENT_TIMER, ALIGNMENT_TIME);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunEvadeSubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    EvadeSubHSMState_t nextState;

    ES_Tattle();

    switch (CurrentState) {
            //----------------------------------------------------------------------------
        case BackUp:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ObjectColor = ReadObject();
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
        case RightTurn:
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
            //----------------------------------------------------------------------------
        case RainbowTurn:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(ONE_QUARTER_SPEED, 375);
                    if (ObjectColor != White){
                        ES_Timer_InitTimer(OH_SHIT_TIMER, FUCK_ME_UP_FAM);
                    }
                    break;
                case ES_TIMEOUT:
                    if(ThisEvent.EventParam == OH_SHIT_TIMER){
                        nextState = BackUp;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case BUMPED:
                    nextState = BackUp;
                    makeTransition = TRUE;
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }

            break;
            //----------------------------------------------------------------------------
        default:
            break;
    }// end switch statement


    if (makeTransition == TRUE) {
        RunEvadeSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunEvadeSubHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}//end function