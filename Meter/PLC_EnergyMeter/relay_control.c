#include <msp430afe253.h>

unsigned char send_relay = 'N';

void relay_resp(unsigned char Command) {
  switch (Command) {
  case 'L':
    IE1 = 0x00; // Disable USART0 RX interrupt
    P1OUT |= BIT0; //Turn on relay
    send_relay = 'N';
    IE1 = URXIE0; // Enaable USART0 RX interrupt
    break;

  case 'D':
    IE1 = 0x00; // Disable USART0 RX interrupt
    P1OUT &= ~BIT0; //Turn on relay
    send_relay = 'N';
    IE1 = URXIE0; // Enaable USART0 RX interrupt
    break;
  default:
    send_relay = 'N';
  }
}

void relay(void) {
  send_relay = 1;
}
