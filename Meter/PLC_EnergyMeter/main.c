/* Medidor monofásico com interface PLC
 * Autor: Adelson Duarte dos Santos
 * Data da documentação 15/03/2018
 *
 * --------------------------------------------------------------
 *
 * Ganho ajustável (Foi testado o algoritmo soldando resistores diferentes, porém o teste com a chave acontecerá na versão V2 de hardware)
 *      //  Gain_2  |   Gain_1  |   R   |           |  Ganho        R1 = 470
 *      //   0            0        470    R             2.6         R2 = 150
 *      //   0            1        235  (R//R1)         4.2
 *      //   1            0        113   (R//R2)        6.0
 *      //   1            1        110   (R//R1//R2)    7.6
 *
 * -----------------------------------------------------------------
 *
 *
 * -----------------------------------------------------------------
 *
 * Parâmetros
 *
 *  1 - Clock
 *  MCLK = SMCLK/2 = 10.855MHz
 *  SMCLK = 21.71MHz
 *
 * 2 - Conversor AD
 * a - Ganho de Corrente
 *  IRMSmax = 10A, Ipico = 14A
 *  Rshunt = 5mohm
 *  Vp entrada AD = 70mV
 *  Vrms entrada AD = 50mV
 *  SD24GAINx = 4
 *
 *  b - Ganho de Tensão
 *  VRMS max 120V, VPico = 170V
 *  Divisor de tensão = 0.001512859
 *  Vp entrada AD max = 181mV
 *  Vrms entrada AD max = 257mV
 *  SD24GAINx = 2
 *
 *  c - Canais
 *  Canal 0: V
 *  Canal 1: I
 *
 *  d - Configuração
 *  Frequency AD = 1.809MHz
 *  OSR = 512
 *  Fsampling = 3540 Samples/s
 *
 *  3 - UART Configuration
 *  BAUDRATE 9600b/s
 *  CLK = SMCLK
 */

#include <msp430afe253.h>

#include "math.h"

#include "PWM.c"

#include "uart_process.c"

#include "Gain.c"

#include "Measure_process.c"

#include "relay_control.c"

#include <stdlib.h>


//Variables

short int Port_flag = 1;
short int s_count = 0, freq_adjust_flag = 0;
int frequency = 0;

//Functions
void clock_init();
void sd_init();
void port_init();
void UART_init();
//

void clock_init() {
  WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
  //SELS = 0 , DIVSx = 0  SCG1 = 0 -------------> SMCLK = DCOCLK
  //SELMx = 1 ou 0, DIVMx = 2 CPUOFF = 0 -------> MCLK = SMCLK/2
  //Whenever necessary, BCSTL2 must be declared before others clock registers, in order to not have MCLK > 12MHz
  BCSCTL2 = DIVM_1; //SELMx = 0, DIVM_1 = /2, SELS = 0, DIVS = 0, DCOR = 0
  DCOCTL = 0xE0; //DCOx = 7, MODx = 0
  BCSCTL1 = 0X8F; // RSELx = 15
}

void sd_init() {
  //Sampling frequency
  volatile unsigned int j;
  SD24CTL |= SD24XDIV_1 | SD24DIV_3 | SD24SSEL_1 | SD24REFON; // clock divider = 24 |  clock source select = 1 | Reference generator on = 1

  //Configure channel 0 - Voltage
  SD24CCTL0 = SD24GRP | SD24IE | SD24DF; //Enable Interrupt, data format = 2's complement, oversampling = 256
  SD24INCTL0 = SD24GAIN_2;

  //Configure Channel 1 - Current
  SD24CCTL1 |= SD24IE | SD24DF; // //Enable Interrupt, data format = 2's complement, oversampling = 512
  SD24INCTL1 = SD24GAIN_4;

  for (j = 0; j < 0x3600; j++); // Delay for 1.2V ref startup
  SD24CCTL1 |= SD24SC; // Set bit to start conversion  |

}

void port_init() {
  P2DIR |= BIT0; 
  P2OUT |= BIT0; //LED
  P1DIR |= BIT0 | BIT1; // P1.0 = Relay, P1.1 = PWM
  P1SEL |= BIT1 | BIT3 | BIT4; //P1.1 = TA1, P1.3,1.4 = USART0 TXD/RXD
  P1OUT |= BIT0;

}

void UART_init() {
  ME1 |= UTXE0 + URXE0; // Enable USART0 TXD/RXD
  U0CTL |= CHAR + SWRST; // 8-bit character, USART in reset state
  U0TCTL |= SSEL1; // UCLK= MCLK
  U0BR0 = 0xD4; // 21.71MHz 9600 b/s
  U0BR1 = 0x08; // 21.71MHz 9600 b/s
  U0MCTL = 0x03; // 21.71MHz 9600 b/s modulation
  U0CTL &= ~SWRST; // 8-bit character, USART ready
  IE1 = URXIE0; // Enable USART0 RX interrupt
}

void main(void) {
  clock_init();
  __bis_SR_register(GIE); // Enable interrupt
  port_init();
  UART_init();
  sd_init();

  while (1) {
    if (s_count == SAMPLE_CYCLE) // S_COUNT = SAMPLE_CYCLE
    {
      SD24CCTL1 &= ~SD24SC; // Set bit to stop conversion
      if (send == 1) {
        P2OUT ^= BIT0;
        PWM(frequency, 1);
        send_param(frequency);
        PWM(frequency, 0);
        send = 0; //clear sample count
      }
      if (send_relay != 'N') {
        relay_resp(send_relay);
      }
      s_count = 0;
      SD24CCTL1 |= SD24SC;
    }
  }

}
#pragma vector = USART0RX_VECTOR
__interrupt void USART0RX() {
  if (freq_adjust_flag == 1) // a way to separate the frequency sweep and the command of the system
  {
    bufferRX = U0RXBUF;
  } else {
    command(U0RXBUF);
  }

}

#pragma vector = SD24_VECTOR
__interrupt void SD24AISR(void) {
  switch (SD24IV) {
  case 2: // SD24MEM Overflow
    break;
  case 4: // SD24MEM0 IFG
    V_AD_RESULT = SD24MEM0;
    I_AD_RESULT = SD24MEM1;
    V_buffer[s_count] = V_AD_RESULT;
    I_buffer[s_count] = I_AD_RESULT;
    V_buffer_accum += V_AD_RESULT; // Accum to calculate average
    I_buffer_accum += I_AD_RESULT;
    s_count++;
    break;
  case 6: // SD24MEM1 IFG
    break;
  case 8: // SD24MEM2 IFG
    break;
  }
}

#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void) {
  //If this interrupt occurs, then there is a gain capable to turn P1.4 off
  Port_flag = 0;
  P1IFG &= ~BIT4; // P1.4 IFG cleared
}
