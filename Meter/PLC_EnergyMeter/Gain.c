#include <msp430afe253.h>

#include "Gain.h"

void gain_adjust() {
  //P1.7 Gain_2, P1.6 Gain_1

  //  Gain_2  |   Gain_1  |   R   |           |  Ganho        R1 = 470
  //   0            0        470    R             2.6         R2 = 150
  //   0            1        235  (R//R1)         4.2
  //   1            0        113   (R//R2)        6.0
  //   1            1        110   (R//R1//R2)    7.6

  unsigned short int c = 0, j = 0;
  short int Port_flag = 1;
  P1IE |= BIT4; // P1.4 interrupt enabled
  P1IES |= BIT4; // P1.4 Hi/lo edge
  P1IFG &= ~BIT4; // P1.4 IFG cleared
  P1DIR |= BIT7 | BIT6;
  gain_state = gain_1;
  while (c != 1) {
    //        Machine state to gain adjust
    switch (gain_state) {
    case gain_1:
      P1OUT &= ~BIT7 & ~BIT6;
      for (j = 0; j < 0x3600; j++); // Delay
      if (Port_flag == 1) //Verify if port stay high
      {
        gain_state = gain_2;
      } else //Minimum gain doesnt work in this powerline
      {
        c = 1;
      }
      break;
    case gain_2:
      for (j = 0; j < 0x3600; j++); // Delay
      P1OUT |= BIT6;
      if (Port_flag == 1) {
        gain_state = gain_3;
      } else {
        gain_state = gain_1;
        P1OUT &= ~BIT7 & ~BIT6;
        c = 1; //Minimum gain doesnt work in this powerline
      }
      break;

    case gain_3:
      P1OUT = BIT7;
      for (j = 0; j < 0x3600; j++); // Delay
      if (Port_flag == 1) {
        //gain2 is ok. Lets increase this gain
        gain_state = gain_4;
      } else {
        P1OUT = BIT6;
        gain_state = gain_2;
        c = 1;
        //gain2 is too high, lets return to minimum gain
      }
      break;

    case gain_4:
      P1OUT |= BIT6;
      for (j = 0; j < 0x3600; j++); // Delay
      if (Port_flag == 1) {
        c = 1;
      } else {
        P1OUT |= BIT6;
        gain_state = gain_3;
        //gain3 is too high, lets return to gain2
        c = 1;
      }
      break;
    default:
      gain_state = gain_1;
    }
  }
  P1IE &= ~BIT4; // P1.4 interrupt enabled

}
