#include <msp430afe253.h>

#include "PWM.h"


void PWM(short int index, short int select) {
  const short int LimR0[23] = {
    42,
    41,
    40,
    39,
    38,
    37,
    36,
    35,
    34,
    33,
    32,
    31,
    30,
    29,
    28,
    27,
    26,
    25,
    24,
    23,
    22,
    21,
    20
  };
  const short int LimR1[23] = {
    21,
    21,
    20,
    20,
    19,
    19,
    18,
    18,
    17,
    17,
    16,
    16,
    15,
    15,
    14,
    14,
    13,
    13,
    12,
    12,
    11,
    11,
    10
  };
  short int R0, R1;
  if (select == 1) {
    TACCTL1 = 0x00;
    R0 = LimR0[index];
    R1 = LimR1[index];
    TACTL = 0x00;
    TACTL |= TACLR;
    TACCR0 = R0;
    TACCTL1 = OUTMOD_6;
    TACCR1 = R1;
    TACTL = TASSEL_2 + MC_1;
  } else {
    TACCTL1 = 0x00;
  }

}

int PWM_adjust(void) {
  /*freq_adjust_flag = 1;*/

  // Sweep frequency routine
  /* each 'i' value is related to frequency vector position in PWM function, so
   * PWM functions will run 23x to sweep all frequencies sending a T character test.
   * In the beginning If the system receive a T character successfully, k[i] = 1 and s[i]  (1)
   * At the end, an average of the interval which the T character was successfully received  is made
   */
  short int s[23] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };
  short int k[23] = {
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0
  };
  short int k_counter = 0;
  unsigned short int i, j, f;
  unsigned char bufferRX;

  for (i = 0; i < FREQUENCY_AMOUNT; i++) {
    PWM(i, 1);
    __delay_cycles(100);
    U0TXBUF = TEST;
    while ((U0TCTL & TXEPT) != 1);
    __delay_cycles(100);

    if (bufferRX == TEST) {
      k[i] = 1;
      s[i] = i;
      k_counter += 1;
    } else {
      k[i] = 0;
      s[i] = 0;
    }

    j += s[i];
  }
  f = j / k_counter;
  //freq_adjust_flag = 0;
  return f;
}
