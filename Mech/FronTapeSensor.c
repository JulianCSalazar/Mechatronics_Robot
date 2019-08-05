/*
 
 Returns Black or White depending on the object in front of Julian, The Robot Who Loves.
 
 */

#include "FronTapeSensor.h"
//#include "HSM.h"
#include "AD.h"

#define HIGHBOUND 1200
#define LOWBOUND 600
#define White 1
#define Black 0




int ReadObject(void){
    static int value = 0;
    if(LOWBOUND<AD_ReadADPin(AD_PORTV8)){
        value = White;
    }else{
        value = Black;
    }
    //printf("FrontValue: %d\n", AD_ReadADPin(AD_PORTW4));
    return value;
}

