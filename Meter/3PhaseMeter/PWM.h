/*
 * PWM.h
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */

#ifndef PWM_H_
#define PWM_H_
#define FREQUENCY_AMOUNT 23
#define TEST 'T'

extern short int R0,R1;
extern unsigned short int i;
extern short int k_counter;
extern short int s[23];
extern short int k[23];
extern const short int LimR0[23];
extern const short int LimR1[23];
extern short int freq_adjust_flag;
unsigned char bufferRX;

int PWM_adjust(void);
void PWM(short int, short int);





#endif /* PWM_H_ */
