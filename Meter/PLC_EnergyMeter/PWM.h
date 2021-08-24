#ifndef PWM_H
#define PWM_H
#define TEST 'T'
#define FREQUENCY_AMOUNT 23
#include <stdint.h>


extern short int R0,R1;
extern unsigned short int i;
extern short int k_counter;
extern short int s[23];
extern short int k[23];
extern const short int LimR0[23];
extern const short int LimR1[23];

int PWM_adjust(void);
void PWM(short int, short int);

#endif
