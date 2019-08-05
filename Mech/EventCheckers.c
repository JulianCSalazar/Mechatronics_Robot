/*
 * File:   EventCheckers.c
 * Author: rkgrant
 *
 * Created on November 26, 2017, 8:05 PM
 */

#include "BOARD.h"
#include "Bumpers.h"
#include "Beacon.h"
#include "ES_Configure.h"
#include "EventCheckers.h"
#include "ES_Events.h"
#include "serial.h"
#include "IO_Ports.h"

#define NUM_OF_BUMPERS 2
#define NUM_OF_TAPE_SENSORS 5
#define DEBOUNCE_MASK 0x1111

#define BATTERY_DISCONNECT_THRESHOLD 175
#define THRESHOLD 500

/*******************************************************************************
 * EVENTCHECKER_TEST SPECIFIC CODE                                                             *
 ******************************************************************************/

//#define EVENTCHECKER_TEST
#ifdef EVENTCHECKER_TEST
#include <stdio.h>
#define SaveEvent(x) do {eventName=__func__; storedEvent=x;} while (0)

static const char *eventName;
static ES_Event storedEvent;
#endif

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this EventChecker. They should be functions
   relevant to the behavior of this particular event checker */
void PrintEvent(void);

uint8_t BumperEventChecker(void) {
    int i;
    int unpressed = 0;
    //initializes all ES framework variable stuff
    static ES_EventTyp_t lastState = NOT_BUMPED;
    ES_EventTyp_t currentState = lastState;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    // debounce variables. prevBumpValues will keep a history of four bumper values
    // which we can compare to debounce mask to see if we have had a history of a bumper hit
    // or lack there of
    uint16_t debounceMask = DEBOUNCE_MASK;
    unsigned int bumperValues = Bumpers_Read();
    static unsigned int prevBumpValues = 0x0000;

    //uploads current bumper values to the history
    prevBumpValues <<= 4;
    prevBumpValues |= bumperValues;

    if (lastState == NOT_BUMPED) { //checks for a NOT_BUMPED -> BUMPED transition
        for (i = 0; i < NUM_OF_BUMPERS; i++) {
            if ((prevBumpValues & debounceMask) == debounceMask) {
                currentState = BUMPED;
                break;
            }
            debounceMask <<= 1;
        }
    } else if (lastState == BUMPED) { //checks for a BUMPED -> NOT_BUMPED transition
        for (i = 0; i < NUM_OF_BUMPERS; i++) {
            if ((prevBumpValues & debounceMask) == 0x0000) {
                unpressed++;
            }
            debounceMask <<= 1;
        }
        if (unpressed == NUM_OF_BUMPERS) {
            currentState = NOT_BUMPED;
        }
    }

    //checks for a state change
    if (currentState != lastState) {
        thisEvent.EventType = currentState;
        thisEvent.EventParam = bumperValues;
        PostBumpService(thisEvent);
        returnVal = TRUE;
    }

    lastState = currentState;
    return returnVal;
}

uint8_t BeaconEventChecker(void) {
    //initializes all ES framework variable stuff
    static ES_EventTyp_t lastState = NO_BEACON;
    ES_EventTyp_t currentState = lastState;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    // debounce variables. prevBeaconValues will keep a history of four beacon values
    // which we can compare to debounce mask to see if we have had a history of a beacon detection
    // or lack there of
    uint16_t debounceMask = DEBOUNCE_MASK;
    unsigned int beaconValues = Beacon_Read();
    static unsigned int prevBeaconValues = 0x0000;

    //uploads current beacon values to the history
    prevBeaconValues <<= 4;
    prevBeaconValues |= beaconValues;
    //printf("BeaconValues: %x\n", beaconValues);

    if (lastState == NO_BEACON) { //checks for a NO_BEACON -> BEACON_FOUND transition
        if ((prevBeaconValues & debounceMask) == debounceMask) {
            currentState = BEACON_FOUND;
        }
    } else if (lastState == BEACON_FOUND) { //checks for a BEACON_FOUND -> NO_BEACON transition
        if ((prevBeaconValues & debounceMask) == 0x0000) {
            currentState = NO_BEACON;
        }
    }

    //checks for a state change
    if (currentState != lastState) {
        thisEvent.EventType = currentState;
        thisEvent.EventParam = beaconValues;
        PostHSM(thisEvent);
        returnVal = TRUE;
    }

    lastState = currentState;
    return returnVal;
}

uint8_t TapeEventChecker(void) {
    //initializes all ES framework variable stuff
    static ES_EventTyp_t lastState = NO_TAPE_EVENT;
    ES_EventTyp_t currentState;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    static unsigned int prevTapeValues = 0x0000;
    unsigned int tapeValues = Tape_Read();

    if (tapeValues != prevTapeValues) {
        currentState = TAPE_EVENT;
        prevTapeValues = tapeValues;
    } else {
        currentState = NO_TAPE_EVENT;
    }

    if (currentState != lastState) {
        thisEvent.EventType = currentState;
        thisEvent.EventParam = tapeValues;
        PostTapeService(thisEvent);
        returnVal = TRUE;
    }

    lastState = currentState;
    return returnVal;
}

uint8_t TWDEventChecker(void) {
    //initializes all ES framework variable stuff
    static ES_EventTyp_t lastState = NO_TRACKWIRE;
    ES_EventTyp_t currentState;
    ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    unsigned int TWDValue = TWD_Read();
    if (TWDValue != 0) {
        currentState = TRACKWIRE_FOUND;
    } else {
        currentState = NO_TRACKWIRE;
    }

    if (currentState != lastState) {
        thisEvent.EventType = currentState;
        thisEvent.EventParam = TWDValue;
        PostHSM(thisEvent);
        
        returnVal = TRUE;
    }
    lastState = currentState;
    return returnVal;
}

#ifdef EVENTCHECKER_TEST
#include <stdio.h>
static uint8_t(*EventList[])(void) = {EVENT_CHECK_LIST};

void main(void) {
    BOARD_Init();
    /* user initialization code goes here */
    AD_Init();
    PWM_Init();

    Movement_Init();
    Bumpers_Init();
    Beacon_Init();
    Tape_Init();

    // Do not alter anything below this line
    int i;

    printf("\r\nEvent checking test harness for %s", __FILE__);

    while (1) {
        if (IsTransmitEmpty()) {
            for (i = 0; i< sizeof (EventList) >> 2; i++) {
                if (EventList[i]() == TRUE) {
                    PrintEvent();
                    break;
                }

            }
        }
    }
}

void PrintEvent(void) {
    printf("\r\nFunc: %s\tEvent: %s\tParam: 0x%X", eventName,
            EventNames[storedEvent.EventType], storedEvent.EventParam);
}

#endif