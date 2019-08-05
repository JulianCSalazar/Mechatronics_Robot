//#include "BOARD.h"
//#include "pwm.h"
//#include "IO_Ports.h"
//#include "RC_Servo.h"
//#include "Movement.h"
//#include "AD.h"
//#include "stdio.h"
//#include "serial.h"
//#include "Bumpers.h"
//#include "TWD.h"
//
///**Function Prototypes**/
//void ArtificialDelay(void);
//void ShortDelay(void);
//void ShorterDelay(void);
//
///**Defines**/
//#define DELAY_TIME 1000000
//#define SHORT_DELAY_TIME 10000
//#define SHORTER_DELAY_TIME 1000
//
//#define AD_PINS AD_PORTV3 | AD_PORTV4 | AD_PORTV5 | AD_PORTV6 | AD_PORTV7
//
//int main(void){
//	//Library Initializations
//	BOARD_Init();
//    AD_Init();
//    PWM_Init();
//    RC_Init();
//    Tape_Init();
//
//    RC_AddPins(RC_PORTY07);
//    int i;
//    while(1){
//        RC_SetPulseTime(RC_PORTY07,2000);
//        
//        for (i = 2000; i > 500; i--){
//            RC_SetPulseTime(RC_PORTY07, i);
//            ShortDelay();
//            printf("%d\n", i);
//        }
////        printf("OBJ: %u\n", AD_ReadADPin(AD_PORTW5));
////        printf("TAPE: %u\n", AD_ReadADPin(AD_PORTV3));
////        ShortDelay();
////        ShortDelay();
//    }//end while(1)
//}//end main
//
//void ArtificialDelay(void){
//	int i;
//	for (i = 0; i < DELAY_TIME; i++){}
//}
//
//void ShortDelay(void){
//	int i;
//	for (i = 0; i < SHORT_DELAY_TIME; i++){}
//}
//void ShorterDelay(void){
//    int i;
//    for (i = 0; i < SHORTER_DELAY_TIME; i++){}
//}