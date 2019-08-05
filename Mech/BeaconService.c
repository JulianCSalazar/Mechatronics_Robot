/*
 * File: TemplateService.h
 * Author: J. Edward Carryer
 * Modified: Gabriel H Elkaim
 *
 * Template file to set up a simple service to work with the Events and Services
 * Framework (ES_Framework) on the Uno32 for the CMPE-118/L class. Note that this file
 * will need to be modified to fit your exact needs, and most of the names will have
 * to be changed to match your code.
 *
 * This is provided as an example and a good place to start.
 *
 * Created on 23/Oct/2011
 * Updated on 13/Nov/2013
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "BOARD.h"
#include "AD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BeaconService.h"
#include "Beacon.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/
#define TIMER_0_TICKS 10
#define DEBOUNCE_MASK 0x1111

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/
/* Prototypes for private functions for this machine. They should be functions
   relevant to the behavior of this state machine */

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/
/* You will need MyPriority and maybe a state variable; you may need others
 * as well. */

static uint8_t MyPriority;
static int BeaconCount = 0;
/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitTemplateService(uint8_t Priority)
 * @param Priority - internal variable to track which event queue to use
 * @return TRUE or FALSE
 * @brief This will get called by the framework at the beginning of the code
 *        execution. It will post an ES_INIT event to the appropriate event
 *        queue, which will be handled inside RunTemplateService function. Remember
 *        to rename this to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t InitBeaconService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.

    // post the initial transition event
    ES_Timer_InitTimer(BEACON_SERVICE_TIMER, TIMER_0_TICKS);
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to queue
 * @return TRUE or FALSE
 * @brief This function is a wrapper to the queue posting function, and its name
 *        will be used inside ES_Configure to point to which queue events should
 *        be posted to. Remember to rename to something appropriate.
 *        Returns TRUE if successful, FALSE otherwise
 * @author J. Edward Carryer, 2011.10.23 19:25 */
uint8_t PostBeaconService(ES_Event ThisEvent) {
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunTemplateService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be responded.
 * @return Event - return event (type and param), in general should be ES_NO_EVENT
 * @brief This function is where you implement the whole of the service,
 *        as this is called any time a new event is passed to the event queue. 
 * @note Remember to rename to something appropriate.
 *       Returns ES_NO_EVENT if the event have been "consumed." 
 * @author J. Edward Carryer, 2011.10.23 19:25 */
ES_Event RunBeaconService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    /********************************************
     in here you write your service code
     *******************************************/
    static ES_EventTyp_t lastState = NO_BEACON;
    ES_EventTyp_t currentState = lastState;
    ES_EventTyp_t currentStat = NO_ORIENTING_BEACON;
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

    switch (ThisEvent.EventType) {
        case ES_INIT:
            // No hardware initialization or single time setups, those
            // go in the init function above.
            //
            // This section is used to reset service for some reason
            break;

        case ES_TIMEOUT:
            ES_Timer_InitTimer(BEACON_SERVICE_TIMER, TIMER_0_TICKS);
            if (lastState == NO_BEACON) { //checks for a NO_BEACON -> BEACON_FOUND transition
                if ((prevBeaconValues & debounceMask) == debounceMask) {
                    if (BeaconCount == 1) {
                        currentStat = ORIENTING_BEACON;
                        BeaconCount+=1;
                    } else if (BeaconCount > 1) {
                        currentStat = REN_BEACON;
                    }else{
                        BeaconCount+=1;
                    }
                    currentState = BEACON_FOUND;
                }
            } else if (lastState == BEACON_FOUND) { //checks for a BEACON_FOUND -> NO_BEACON transition
                if ((prevBeaconValues & debounceMask) == 0x0000) {
                    if (BeaconCount == 1) {
                        currentStat = NO_ORIENTING_BEACON;
                    } else if (BeaconCount > 1) {
                        currentStat = NO_REN_BEACON;
                    }
                    currentState = NO_BEACON;
                }
            }

            //checks for a state change
            if (currentState != lastState) {
                ReturnEvent.EventType = currentStat;
                ReturnEvent.EventParam = beaconValues;
                printf("\r\nEvent: %s\tParam: 0x%X",
                        EventNames[ReturnEvent.EventType], ReturnEvent.EventParam);
                PostBeaconService(ReturnEvent);
                lastState = currentState;

            }
            break;
            return ReturnEvent;

    }

}

/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/

