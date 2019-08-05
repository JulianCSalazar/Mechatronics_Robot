#include "TWD.h"
#include "IO_Ports.h"
#include "AD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "KillATM6SubHSM.h"

#define HIGH_THRESHOLD 570
#define LOW_THRESHOLD 505

#define TRACK_WIRE_FOUND 1
#define TRACK_WIRE_LOST 0

extern int Threshold;

unsigned int TWD_Read(void) {
    static unsigned int w3 = 0;
    static unsigned int w4 = 0;
    
    if (AD_ReadADPin(AD_PORTW3) > Threshold){
        w3 = TRACK_WIRE_FOUND;
    } else if (AD_ReadADPin(AD_PORTW3) < LOW_THRESHOLD) {
        w3 = TRACK_WIRE_LOST;
    }
    
    if (AD_ReadADPin(AD_PORTW4) > HIGH_THRESHOLD){
        w4 = TRACK_WIRE_FOUND;
    } else if (AD_ReadADPin(AD_PORTW4) < LOW_THRESHOLD) {
        w4 = TRACK_WIRE_LOST;
    }
    
    return ((w3)|(w4<<1));
}