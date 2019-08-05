/*
 * File:   Bumpers.c
 * Author: rkgrant
 *
 * Created on November 26, 2017, 8:10 PM
 */

#include "BOARD.h"
#include "IO_Ports.h"
#include "Bumpers.h"

void Bumpers_Init(void){
    IO_PortsSetPortInputs(PORTZ, PIN3); // Left bumper pin
	IO_PortsSetPortInputs(PORTZ, PIN4); // right bumper pin
}

unsigned int Bumpers_Read(void) {
    unsigned int bumperValue;
    bumperValue = ((IO_PortsReadPort(PORTZ)&PIN3) | (IO_PortsReadPort(PORTZ)&PIN4)) >> 3;
    return bumperValue;
}