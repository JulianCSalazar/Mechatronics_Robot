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
#include "Targets_Service.h"
#include "TWD.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/


#define TIMER_0_TICKS 5
#define NUM_OF_TWD 2
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
uint8_t InitTargetService(uint8_t Priority) {
    ES_Event ThisEvent;

    MyPriority = Priority;

    // in here you write your initialization code
    // this includes all hardware and software initialization
    // that needs to occur.
    ES_Timer_InitTimer(TWD_SERVICE_TIMER, TIMER_0_TICKS);
    // post the initial transition event
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
uint8_t PostTargetService(ES_Event ThisEvent) {
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
ES_Event RunTargetService(ES_Event ThisEvent) {
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT;

    //    static ES_EventTyp_t lastState = NO_TRACKWIRE;
    //    ES_EventTyp_t currentState;
    //
    //    unsigned int TWD_val = TWD_Read();
    //    static unsigned int prevTWD_val = 0x0000;


    //    switch (ThisEvent.EventType) {
    //        case ES_INIT:
    //            break;
    //        case ES_TIMERACTIVE:
    //        case ES_TIMERSTOPPED:
    //            break;
    //        case ES_TIMEOUT:
    //            ES_Timer_InitTimer(TAPE_SERVICE_TIMER, TIMER_0_TICKS);
    //            if (TWD_val != prevTWD_val){
    //                currentState = TRACKWIRE_FOUND;
    //                prevTWD_val = TWD_val;
    //            } else{
    //                currentState = NO_TRACKWIRE; 
    //            }
    //            if (currentState != lastState){
    //                ReturnEvent.EventType = currentState;
    //                ReturnEvent.EventParam = TWD_val;
    //                lastState = currentState;
    //                PostTargetService(ReturnEvent);
    //            }
    //            break;
    //    }
    int i;
    int notFound = 0;
    static ES_EventTyp_t lastState = NO_TRACKWIRE;
    ES_EventTyp_t currentState = lastState;
    //ES_Event thisEvent;
    uint8_t returnVal = FALSE;

    uint16_t debounceMask = DEBOUNCE_MASK;
    unsigned int TWDValue = TWD_Read();
    static unsigned int prevTWD_val = 0x0000;

    prevTWD_val <<= 4;
    prevTWD_val |= TWDValue;

    switch (ThisEvent.EventType) {
        case ES_INIT:
            break;
        case ES_TIMEOUT:
            ES_Timer_InitTimer(TWD_SERVICE_TIMER, TIMER_0_TICKS);
            if (lastState == NO_TRACKWIRE) {
                for (i = 0; i < NUM_OF_TWD; i++) {
                    if ((prevTWD_val & debounceMask) == debounceMask) {
                        currentState = TRACKWIRE_FOUND;
                        break;
                    }
                    debounceMask <<= 1;

                }
            } else if (lastState == TRACKWIRE_FOUND) {
                for (i = 0; i < NUM_OF_TWD; i++) {
                    if ((prevTWD_val & debounceMask) == 0x0000) {
                        notFound++;
                    }
                    debounceMask <<= 1;
                }
                if (notFound == NUM_OF_TWD) {
                    currentState = NO_TRACKWIRE;
                }
            }
            if (currentState != lastState) {
                ReturnEvent.EventType = currentState;
                ReturnEvent.EventParam = TWDValue;
                lastState = currentState; // update history
                printf("\r\nEvent: %s\tParam: 0x%X",
                        EventNames[ReturnEvent.EventType], ReturnEvent.EventParam);
                PostTWDService(ReturnEvent);
            }
            break;
            
    }
    return ReturnEvent;
}



/*******************************************************************************
 * PRIVATE FUNCTIONs                                                           *
 ******************************************************************************/

