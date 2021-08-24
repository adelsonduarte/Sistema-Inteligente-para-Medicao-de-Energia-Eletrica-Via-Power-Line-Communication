/*
 * uart_process.h
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */
# include <stdint.h>

#ifndef UART_PROCESS_H_
#define UART_PROCESS_H_

#define iddle '0'
#define ready '1'
#define id    '2'
#define get   '3'
#define set   '4'
#define value '5'

extern unsigned char uart_state;
extern int frequency;
extern uint8_t send;

//Used to send wave tests  by Serial port ---------------------------------------------------------------------------------------------------------------
union wave
{
    int all_wave;
    char pt_wave[2];

};

union wave voltage1_wave;
union wave current1_wave;
union wave energy1_wave;

union wave voltage2_wave;
union wave current2_wave;
union wave energy2_wave;

union wave voltage3_wave;
union wave current3_wave;
union wave energy3_wave;

//-------------------------------------------------------------------------------------------------------------------------------------------------------

//Just LABVIEW Debbuguing -------------------------------------------------------------------------------------------------------------------------------
union wave contador;
//-------------------------------------------------------------------------------------------------------------------------------------------------------

//Used to send Values by Serial port --------------------------------------------------------------------------------------------------------------------
union VI
{
    float all_VI;
    char pt_VI[4];
};

extern union VI voltage1;
extern union VI current1;
extern union VI energy1;

extern union VI ActPower1;
extern union VI ActPower2;
extern union VI ActPower3;
extern union VI PowerFactor1;
extern union VI PowerFactor2;
extern union VI PowerFactor3;

extern union VI voltage2;
extern union VI current2;
extern union VI energy2;

extern union VI voltage3;
extern union VI current3;
extern union VI energy3;

#endif /* UART_PROCESS_H_ */
