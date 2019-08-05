#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "HSM.h"
#include "KillATM6SubHSM.h"
#include "Movement.h"
#include "Shooter.h"

#define LEFT_TURN_TIMER HSM_TIMER_4
#define LOADING_TIMER HSM_TIMER_5
#define TA_TIMER HSM_TIMER_6
#define FULL_ROTATION_TIMER HSM_TIMER_7
#define LEFT_TURN_TIME 2350
#define LOADING_TIME 400
#define TA_WAITING_TIME 3000
#define FULL_ROTATION_TIME 6800
#define MIN_NUM_OF_TARGETS 3

typedef enum {
    FaceATM6,
    LoadBall,
    WaitForTA,
    LookForBeacon,
    BackToLine,
} KillATM6SubHSMState_t;

static const char *StateNames[] = {
	"FaceATM6",
	"LoadBall",
	"WaitForTA",
	"LookForBeacon",
	"BackToLine",
};

static uint8_t MyPriority;
static KillATM6SubHSMState_t CurrentState;
extern int SpinFlag;
extern int TargetsHit;
int Threshold = 570;

uint8_t InitKillATM6SubHSM(void) {
    ES_Event returnEvent;
    CurrentState = FaceATM6;
    returnEvent = RunKillATM6SubHSM(INIT_EVENT);
    Move(0, 0);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

uint8_t AlternateInitKillATM6SubHSM(void) {
    ES_Event returnEvent;
    CurrentState = LoadBall;
    returnEvent = RunKillATM6SubHSM(INIT_EVENT);
    Move(0, 0);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

ES_Event RunKillATM6SubHSM(ES_Event ThisEvent) {
    uint8_t makeTransition = FALSE;
    KillATM6SubHSMState_t nextState;

    ES_Tattle();

    switch (CurrentState) {
            //----------------------------------------------------------------------------
        case FaceATM6:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    TankTurnCCW(200);
                    ThisEvent.EventType = ES_NO_EVENT;
                    ES_Timer_InitTimer(LEFT_TURN_TIMER, LEFT_TURN_TIME);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == LEFT_TURN_TIMER) {
                        TankTurnCW(200);
                        ThisEvent.EventType = ES_NO_EVENT;
                        ES_Timer_InitTimer(FULL_ROTATION_TIMER, LEFT_TURN_TIME);
                        Threshold = 570;
                    } else if (ThisEvent.EventParam == FULL_ROTATION_TIMER){
                        TankTurnCCW(200);
                        ES_Timer_InitTimer(TA_TIMER, LEFT_TURN_TIME-500);
                        ThisEvent.EventType = ES_NO_EVENT;
                    } else if (ThisEvent.EventParam == TA_TIMER){
                        makeTransition = TRUE;
                        ThisEvent.EventType = ES_NO_EVENT;
                        nextState = LoadBall;
                    }
                    break;
                case TRACKWIRE_FOUND:
                    if (ThisEvent.EventParam == 0x1) {
                        nextState = LoadBall;
                        makeTransition = TRUE;
                        Move(FULL_STOP, FULL_STOP);
                        Threshold = 570;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case LoadBall:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    Move(FULL_STOP, FULL_STOP);
                    ES_Timer_InitTimer(LOADING_TIMER, LOADING_TIME);
                    Shooter_On(500);
                    LAS_Retract(); // Ibrahim removed the if statement that stopped it from retracting on the first shot
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == LOADING_TIMER) {
                        ThisEvent.EventType = ES_NO_EVENT;
                        makeTransition = TRUE;
                        nextState = WaitForTA;
                    }
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case WaitForTA:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(TA_TIMER, TA_WAITING_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    TargetsHit++;
                    LAS_Extend();
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == TA_TIMER) {
                        makeTransition = TRUE;
                        if (TargetsHit < MIN_NUM_OF_TARGETS) {
                            nextState = BackToLine;
                            SpinFlag = TRUE;
                        } else {
                            nextState = LookForBeacon;
                        }
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
                    break;
                case ES_EXIT:
                    Shooter_On(0);
                    ThisEvent.EventType = ES_NO_EVENT;
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case BackToLine:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ThisEvent.EventType = ES_NO_EVENT;
                    TankTurnCW(HALF_SPEED);
                    break;
            }
            break;
            //----------------------------------------------------------------------------
        case LookForBeacon:
            switch (ThisEvent.EventType) {
                case ES_ENTRY:
                    ES_Timer_InitTimer(FULL_ROTATION_TIMER, FULL_ROTATION_TIME);
                    ThisEvent.EventType = ES_NO_EVENT;
                    TankTurnCW(ONE_QUARTER_SPEED);
                    break;
                case ES_TIMEOUT:
                    if (ThisEvent.EventParam == FULL_ROTATION_TIMER) {
                        SpinFlag = TRUE;
                        makeTransition = TRUE;
                        nextState = BackToLine;
                        ThisEvent.EventType = ES_NO_EVENT;
                    }
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
        RunKillATM6SubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunKillATM6SubHSM(ENTRY_EVENT);
    }

    ES_Tail();
    return ThisEvent;
}//end function