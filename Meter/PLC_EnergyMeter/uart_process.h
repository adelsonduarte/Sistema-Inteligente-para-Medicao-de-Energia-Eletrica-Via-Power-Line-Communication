/*
 * uart_process.h
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */

#ifndef UART_PROCESS_H_

#define iddle '0'
#define ready '1'
#define id    '2'
#define get   '3'
#define set   '4'
#define reset '5'

void command(unsigned char);
void send_param(int);

extern unsigned char uart_state;
extern short int send;
extern unsigned char bufferRX;
#define UART_PROCESS_H_






#endif /* UART_PROCESS_H_ */
