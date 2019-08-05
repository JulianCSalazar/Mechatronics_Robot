#include "AD.h"
#include "TapeSensors.h"

#define HIGH_THRESHOLD 700
#define LOW_THRESHOLD 400

#define HIGH_THRESHOLD_CENTER 250
#define LOW_THRESHOLD_CENTER 100


#define WHITE 0
#define BLACK 1

void Tape_Init(void){
	AD_AddPins(AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTV7 | AD_PORTW3 | AD_PORTW4 | AD_PORTW5); 
}

unsigned int Tape_Read(void){
	static unsigned int v3 = 0;
	static unsigned int v4 = 0;
	static unsigned int v5 = 0;
	static unsigned int v6 = 0;
	static unsigned int v7 = 0;
	
    //printf("%d\n", AD_ReadADPin(AD_PORTV3));
	if (AD_ReadADPin(AD_PORTV3) > HIGH_THRESHOLD) {
		v3 = WHITE;
	} else if (AD_ReadADPin(AD_PORTV3) < LOW_THRESHOLD) {
		v3 = BLACK;
	}
	
	if (AD_ReadADPin(AD_PORTV4) > HIGH_THRESHOLD) {
		v4 = WHITE;
	} else if (AD_ReadADPin(AD_PORTV4) < LOW_THRESHOLD) {
		v4 = BLACK;
	}
	
	if (AD_ReadADPin(AD_PORTV5) > HIGH_THRESHOLD) {
		v5 = WHITE;
	} else if (AD_ReadADPin(AD_PORTV5) < LOW_THRESHOLD) {
		v5 = BLACK;
	}
	
	if (AD_ReadADPin(AD_PORTV6) > HIGH_THRESHOLD_CENTER) {
		v6 = WHITE;
	} else if (AD_ReadADPin(AD_PORTV6) < LOW_THRESHOLD_CENTER) {
		v6 = BLACK;
	}
	
	if (AD_ReadADPin(AD_PORTV7) > HIGH_THRESHOLD) {
		v7 = WHITE;
	} else if (AD_ReadADPin(AD_PORTV7) < LOW_THRESHOLD) {
		v7 = BLACK;
	}
	
	return ( v3 | (v4<<1) | (v5<<2) | (v6<<3) | (v7<<4) );
	
}