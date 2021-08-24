#include <msp430f67791.h>
#include "PWM.h"

int PWM_adjust()
{
    unsigned short int i,j=0,f=0;
    short int k_counter = 0;
    short int s[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    short int k[23] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    freq_adjust_flag = 1;

// Sweep frequency routine
/* each 'i' value is related to frequency vector position in PWM function, so
 * PWM functions will run 23x to sweep all frequencies sending a T character test.
 * In the beginning If the system receive a T character successfully, k[i] = 1 and s[i]  (1)
 * At the end, an average of the interval which the T character was successfully received  is made
 */

    for(i=0; i<FREQUENCY_AMOUNT; i++)
    {
        PWM(i,1);
        __delay_cycles(100);
        UCA1TXBUF = TEST;
        while (!(UCA1IFG & UCTXIFG)) ;
        __delay_cycles(100);

        if (bufferRX == TEST)
        {
          P7OUT ^= BIT2;
          k[i] = 1;
          s[i] = i;
          k_counter += 1;
        }
        else
        {
          k[i] = 0;
          s[i] = 0;
        }

       j += s[i];
    }
    f = j / k_counter;
    freq_adjust_flag = 0;
    return f;
}

void PWM(short int index, short int select)
{
    if (select == 1)
    {
        TA0CCTL2 = 0x00;
        R0 = LimR0[index];
        R1 = LimR1[index];
        TA0CCR0 = R0;
        TA0CCTL2 = OUTMOD_6;                    // // CCR1 toggle/set
        TA0CCR2 = R1;
        TA0CTL = TASSEL_2 | MC_1 | TACLR;       // SMCLK, up-mode, clear TAR
    }
    else
    {
        TA0CTL |= TACLR;                         //clear TAR
        TA0CCTL2 = 0x00;                         // Turn off PWM configuration
    }
}



