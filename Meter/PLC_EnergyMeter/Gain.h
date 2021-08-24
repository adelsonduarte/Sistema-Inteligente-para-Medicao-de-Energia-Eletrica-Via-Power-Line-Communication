/*
 * Gain.h
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */

#ifndef GAIN_H_
#define GAIN_H_

#define gain_1 '6'
#define gain_2 '7'
#define gain_3 '8'
#define gain_4 '9'

extern short int Port_flag;
extern unsigned char gain_state;

void gain_adjust(void);

#endif /* GAIN_H_ */
