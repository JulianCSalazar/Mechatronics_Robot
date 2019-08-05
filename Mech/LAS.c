//included files
#include "BOARD.h"
#include "IO_Ports.h"
#include "LAS.h"

void LAS_Init(void){
	IO_PortsSetPortOutputs(PORTZ, PIN7);
    LAS_Extend();
}

void LAS_Retract(void){
	IO_PortsSetPortBits(PORTZ, PIN7);
}

void LAS_Extend(void) {
	IO_PortsClearPortBits(PORTZ, PIN7);
}