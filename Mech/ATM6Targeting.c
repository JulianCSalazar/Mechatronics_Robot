#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "ATM6Targeting.h"
#include "KillATM6SubHSM.h"
#include "Movement.h"
#include "TapeSensors.h"
#include "TapeFollowSubHSM.h"

#define MINIMUM_TIME 3000
#define minTime MIN_TIMER
#define ALIGNMENT_TAPE_TIMER OH_SHIT_TIMER
#define ALIGNMENT_TAPE_TIME 5000

typedef enum {
    FollowTape,
    OH_SHIT,
    KillATM6,

} ATM6TargetingState_t;

static const char *StateNames[] = {
	"FollowTape",
	"OH_SHIT",
	"KillATM6",
};

static ATM6TargetingState_t CurrentState;
static uint8_t MyPriority;
static int minTimeFlag = FALSE;
int SpinFlag = FALSE;
int oh_Shit = 0;

uint8_t InitATM6Targeting(void) {
    ES_Event returnEvent;

    CurrentState = FollowTape;
    returnEvent = RunATM6Targeting(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunATM6Targeting(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    ATM6TargetingState_t nextState;

    ES_Tattle(); // trace call stack

    switch (CurrentState) {
            //----------------------------------------------------------------------------
        case FollowTape: // Follows tape until it finds a target with its trackwire
            ThisEvent = RunTapeFollowSubHSM(ThisEvent);

            switch (ThisEvent.EventType) {
                case TRACKWIRE_FOUND:
                    if (ThisEvent.EventParam == 0x2) {
                        nextState = KillATM6;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(FULL_STOP, FULL_STOP);
                        InitKillATM6SubHSM();
                        ES_Timer_InitTimer(minTime, MINIMUM_TIME);
                    }
                    break;
            }
            break;

        case KillATM6: // Kills an ATM6. Upon completion begin following tape again.
            ThisEvent = RunKillATM6SubHSM(ThisEvent);

            switch (ThisEvent.EventType) {
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == minTime) {
                        minTimeFlag = TRUE;
                    }
                    break;
                case TAPE_EVENT:
                    if (TWD_Read() & 0x2) {
                        nextState = KillATM6;
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        Move(FULL_STOP, FULL_STOP);
                        InitKillATM6SubHSM();
                        ES_Timer_InitTimer(minTime, MINIMUM_TIME);
                    } else if (((ThisEvent.EventParam & Front) == Front) && (minTimeFlag == TRUE) && (SpinFlag == TRUE)) {
                        nextState = FollowTape;
                        makeTransition = TRUE;
                        AlternativeInitTapeFollowSubHSM();
                        ThisEvent.EventType = ES_NO_EVENT;
                        minTimeFlag = FALSE;
                        SpinFlag = FALSE;
                        Move(0, 0);
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
