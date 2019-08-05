/*
 * File:   Beacon.c
 * Author: rkgrant
 *
 * Created on November 26, 2017, 9:10 PM
 */

#include "IO_Ports.h"
#include "Beacon.h"

void Beacon_Init(void){
    IO_PortsSetPortInputs(PORTZ, PIN11);
}

unsigned int Beacon_Read(void){
    unsigned int beaconValue;
    beaconValue = (IO_PortsReadPort(PORTZ)&PIN11) >> 11;
            
    if(beaconValue==0){
        beaconValue = 1;
    }else{
        beaconValue = 0;
    }
    return beaconValue;
}