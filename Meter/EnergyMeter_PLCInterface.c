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
#include <stdlib.h>

#define iddle '0'
#define ready '1'
#define id    '2'
#define get   '3'
#define set   '4'
#define reset '5'

#define gain_1 '6'
#define gain_2 '7'
#define gain_3 '8'
#define gain_4 '9'
//#define gain_end 's'
#define TEST 'T'
#define FREQUENCY_AMOUNT 23


#define SAMPLE_CYCLE 59
//#define kw 0.0002731481481 //(59/60^3)
//#define kw 0.000004552469136 //59/60^4)
#define kw 0.000462962963
#define kw 0.000007716049383
#define kv 0.005925924783
#define ka 0.001001730261
//#define kv 1
//#define ka 1

//Variables

// Only for debugging Labview//
union wave
{
    int all_wave;
    char pt_wave[2];

};

union wave voltage_wave;
union wave current_wave;
//----------------------------//
union VI
{
    float all_VI;
    char pt_VI[4];
};

union VI voltage;
union VI current;
union VI energy;
union VI ActPower;
union VI ReactEnergy;
union VI ReactPower;
union VI PowerFactor;

unsigned char uart_state;
unsigned char gain_state;
unsigned char bufferRX;
unsigned char send_relay = 'N';

short int R0 = 0, R1 = 0;
short int Port_flag = 1;
short int s_count = 0, freq_adjust_flag = 0;
short int idx = 0, send = 0;

const short int LimR0[23] = {42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20};
const short int LimR1[23] = {21,21,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10};

int frequency = 0;
int I_buffer[59], V_buffer[59];
int I_AD_RESULT, V_AD_RESULT;
int I_buffer_accum = 0, V_buffer_accum = 0;

signed long int V1_avg = 0, I1_avg = 0;
signed long long int V_acum=0, I_acum=0, V_operand = 0, I_operand = 0;
signed long long int P_acum = 0;

float V_rms = 0, I_rms = 0;
float PF = 0, S = 0, P = 0, E = 0, React_E = 0;
float argument = 0;
//

//Functions
void clock_init();
void sd_init();
void port_init();
void UART_init();
void measurement();
void calculate();
void command(unsigned char);
void send_param(int f);
void relay();
int PWM_adjust();
void PWM(int j, int s);
void relay_resp(unsigned char);
//


void clock_init()
{
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer
//SELS = 0 , DIVSx = 0  SCG1 = 0 -------------> SMCLK = DCOCLK
//SELMx = 1 ou 0, DIVMx = 2 CPUOFF = 0 -------> MCLK = SMCLK/2

//Whenever necessary, BCSTL2 must be declared before others clock registers, in order to not have MCLK > 12MHz
    BCSCTL2 = DIVM_1;                           //SELMx = 0, DIVM_1 = /2, SELS = 0, DIVS = 0, DCOR = 0
    DCOCTL = 0xE0;                              //DCOx = 7, MODx = 0
    BCSCTL1 = 0X8F;                             // RSELx = 15
}

void port_init()
{
    P2DIR |= BIT0;                                    //LED
//    P2OUT &= ~BIT0;                                   //LED
    P2OUT |= BIT0;
//    P1SEL |= BIT0;         // SMCLK Pin Function
//    P1SEL2 |= BIT0;

    P1DIR |= BIT0 | BIT1;                               // P1.0 = Relay, P1.1 = PWM
    P1SEL |= BIT1 | BIT3 | BIT4;                        //P1.1 = TA1, P1.3,1.4 = USART0 TXD/RXD
//    P1OUT &= ~BIT0;
    P1OUT |= BIT0;

}

void UART_init()
{
    ME1 |=  UTXE0 + URXE0;                      // Enable USART0 TXD/RXD
    U0CTL |= CHAR + SWRST;                      // 8-bit character, USART in reset state
    U0TCTL |= SSEL1;                            // UCLK= MCLK
//    U0BR0 = 0x42;                               // 20.3MHz 9600 b/s
//    U0BR1 = 0x08;                               // 20.3MHz 9600 b/s
//    U0MCTL = 0x05;                              // 20.3MHz 9600 b/s modulation
    U0BR0 = 0xD4;                               // 21.71MHz 9600 b/s
    U0BR1 = 0x08;                               // 21.71MHz 9600 b/s
    U0MCTL = 0x03;                              // 21.71MHz 9600 b/s modulation
    U0CTL &= ~SWRST;                            // 8-bit character, USART ready
    IE1 = URXIE0;                               // Enable USART0 RX interrupt
}

void sd_init()
{
    //Frequência de amostragem  =
    volatile unsigned int j;

//    SD24CTL |= SD24XDIV_1 | SD24DIV_2 | SD24SSEL_1 | SD24REFON;     // clock divider = 12 |  clock source select = 1 | Reference generator on = 1
    SD24CTL |= SD24XDIV_1 | SD24DIV_3 | SD24SSEL_1 | SD24REFON;     // clock divider = 24 |  clock source select = 1 | Reference generator on = 1


    //Configure channel 0 - Voltage

//    SD24CCTL0 =  SD24XOSR | SD24GRP | SD24IE | SD24DF;              //Enable Interrupt, data format = 2's complement, oversampling = 512
    SD24CCTL0 =  SD24GRP | SD24IE | SD24DF;              //Enable Interrupt, data format = 2's complement, oversampling = 256
    SD24INCTL0 = SD24GAIN_2;

    //Configure Channel 1 - Current
//    SD24CCTL1 |= SD24XOSR | SD24IE | SD24DF;                        // //Enable Interrupt, data format = 2's complement, oversampling = 512
    SD24CCTL1 |=  SD24IE | SD24DF;                        // //Enable Interrupt, data format = 2's complement, oversampling = 512
    SD24INCTL1 = SD24GAIN_4;

    for (j = 0; j < 0x3600; j++);                                   // Delay for 1.2V ref startup

    SD24CCTL1 |= SD24SC;                                            // Set bit to start conversion  |


}

void gain_adjust()
{
    //P1.7 Gain_2, P1.6 Gain_1

    //  Gain_2  |   Gain_1  |   R   |           |  Ganho        R1 = 470
    //   0            0        470    R             2.6         R2 = 150
    //   0            1        235  (R//R1)         4.2
    //   1            0        113   (R//R2)        6.0
    //   1            1        110   (R//R1//R2)    7.6

    unsigned short int c = 0,j=0;
//    P2DIR |= BIT0;                              //LED
//    P2OUT &= ~BIT0;                             //LED
    P1IE |= BIT4;                               // P1.4 interrupt enabled
    P1IES |= BIT4;                              // P1.4 Hi/lo edge
    P1IFG &= ~BIT4;                             // P1.4 IFG cleared
    P1DIR |= BIT7 | BIT6;
//    P1OUT &= ~BIT7 & ~BIT6;
    gain_state = gain_1;
    while (c!=1)
    {
//        Machine state to gain adjust
        switch(gain_state)
        {
            case gain_1:
                P1OUT &= ~BIT7 & ~BIT6;
                for (j = 0; j < 0x3600; j++);               // Delay
                if (Port_flag == 1)                         //Verify if port stay high
                {
                    gain_state = gain_2;
                }
                else                                        //Minimum gain doesnt work in this powerline
                {
                    c = 1;
                }
                break;
            case gain_2:
                for (j = 0; j < 0x3600; j++);               // Delay
                P1OUT |= BIT6;
                if (Port_flag == 1)
                {
//                    P1OUT = BIT6;                         //Set P1.6 to increase gain (4.2)
                    gain_state = gain_3;
                }
                else
                {
                    gain_state = gain_1;
                    P1OUT &= ~BIT7 & ~BIT6;
                    c=1;                                   //Minimum gain doesnt work in this powerline
                }
            break;

            case gain_3:
                P1OUT = BIT7;
                for (j = 0; j < 0x3600; j++);               // Delay
                if (Port_flag == 1)
                {
//                   P1OUT = BIT7;                          //gain2 is ok. Lets increase this gain (6.0)
                   gain_state = gain_4;
                }
                else
                {
                    P1OUT = BIT6;
                    gain_state = gain_2;
                    c=1;
//                    P1OUT &= ~BIT6;                       //gain2 is too high, lets return to minimum gain
                }
                break;

            case gain_4:
                P1OUT |= BIT6;
                for (j = 0; j < 0x3600; j++);               // Delay
                if (Port_flag == 1)
                {
//                    P1OUT |= BIT6;                        //Set P1.6 to increase gain (7.6)
//                    gain_state = gain_end;
                    c=1;
                }
                else
                {
                    P1OUT |= BIT6;
                    gain_state = gain_3;
//                    P1OUT = BIT6;                         //gain3 is too high, lets return to gain2
                    c=1;
                }
                break;
            default:
                gain_state = gain_1;
        }
    }
    P1IE &= ~BIT4;                                          // P1.4 interrupt enabled

}

//start,MeterId,action,end
void command(unsigned char c)
{
  if(c == '#')
  {
//      P2OUT ^= BIT0;
      uart_state = ready;
  }

  else
  {
     if(uart_state != iddle)
     {
        switch(uart_state)
        {
          case ready:
            if(c == '1')
            {
                uart_state = id;
            }
            else uart_state = iddle;
                break;

          case id:
            switch(c)
            {
              case 'g':
                  uart_state = get;
                break;
              case 's':
                  uart_state = set;
                break;
              case 'r':
                  uart_state = reset;
                break;
              default:
                  uart_state = iddle;
            }
                break;

          case get:
            if(c == ';')
            {
                send = 1;
            }

            uart_state = iddle;
            break;

          case set:
              if(c == ';')
              {
                  send_relay = 'L';
              }

            uart_state = iddle;
            break;

          case reset:
              if(c == ';')
              {
                  send_relay = 'D';
              }
            uart_state = iddle;
            break;

          default:
              uart_state = iddle;
         }
     }
  }
}



void send_param(f)
{
    unsigned int count;
    unsigned int send_count;

     IE1 = 0x00;                                        // Disable USART0 RX interrupt

    U0TXBUF = '*';                                      // Send Header
    while((U0TCTL & TXEPT) != 1);


//                        Only for Labview                                //
//    //Send Voltage Wave
//    for(count = 0; count < SAMPLE_CYCLE; count ++)
//    {
//       voltage_wave.all_wave = V_buffer[count];
//       for(send_count = 2; send_count > 0; send_count --)
//       {
//           U0TXBUF = voltage_wave.pt_wave[(send_count-1)];
//           while((U0TCTL & TXEPT) != 1);
//       }
//    }
//
//    count = 0;
//    // Send Current Wave
//    for(count = 0; count < SAMPLE_CYCLE; count ++)
//    {
//      current_wave.all_wave = I_buffer[count];
//      for(send_count = 2; send_count > 0; send_count --)
//      {
//          U0TXBUF = current_wave.pt_wave[(send_count-1)];
//          while((U0TCTL & TXEPT) != 1);
//      }
//
//    }
//------------------------------------------------------------------------//
//    count = 0;
//    //Send Voltage RMS
   for(count=4;count>0;count--)                        // Loop to send the voltage float value
    {
        U0TXBUF = voltage.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }
   //Send Current RMS
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = current.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }

   //Send PowerFactor
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = PowerFactor.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }

   //Send Energy
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = energy.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }

   //Send ReactEnergy
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = ReactPower.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }

   //Send ActPower
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = ActPower.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }
   //Send ReactPower
   for(count=4;count>0;count--)                    // Loop to send the current float value
    {
        U0TXBUF = ReactPower.pt_VI[(count-1)];
        while((U0TCTL & TXEPT) != 1);               // Wait for the flag TXEPT -> Transmission Complete
    }

////-------- Labview---  //
//   U0TXBUF = f;                                  //Send END
//   while((U0TCTL & TXEPT) != 1);                   // Wait for the flag TXEPT -> Transmission Complete
//// ----------------   //
    U0TXBUF = '!';                                  //Send END
    while((U0TCTL & TXEPT) != 1);                   // Wait for the flag TXEPT -> Transmission Complete

   IE1 = URXIE0;                                   // Enaable USART0 RX interrupt

}

void relay()
{
//    P1OUT |= BIT0;  //Turn on relay
//    send_relay = 1;
}

void relay_resp(unsigned char command)
{
    switch(command)
    {
        case 'L':
             IE1 = 0x00;                                        // Disable USART0 RX interrupt

             P1OUT |= BIT0;                                      //Turn on relay

             send_relay = 'N';
             IE1 = URXIE0;                                   // Enaable USART0 RX interrupt
             break;

        case 'D':
             IE1 = 0x00;                                        // Disable USART0 RX interrupt

             P1OUT &= ~BIT0;                                      //Turn on relay

             send_relay = 'N';
             IE1 = URXIE0;                                   // Enaable USART0 RX interrupt
             break;
        default:
            send_relay = 'N';

    }
}


void calculate()
{
    unsigned int count;

    V1_avg = V_buffer_accum / SAMPLE_CYCLE;             //Average value
    I1_avg = I_buffer_accum / SAMPLE_CYCLE;             //Average value

    for(count=0;count<SAMPLE_CYCLE;count++)             //Subtract average value for each sample
    {
        V_buffer[count] -= V1_avg;
        I_buffer[count] -= I1_avg;

    }
    for(count=0;count<SAMPLE_CYCLE;count++)             //Loop for RMS Calculations
    {
        I_operand = I_buffer[count];
        V_operand = V_buffer[count];
        I_acum += I_operand * I_operand;
        V_acum += V_operand * V_operand;
        P_acum += V_operand * I_operand;
    }

    argument = I_acum / SAMPLE_CYCLE;                   //RMS Calculation
    I_rms =  sqrt(argument);
    argument = 0;

    argument = V_acum / SAMPLE_CYCLE;                   //RMS Calculation
    V_rms = sqrt(argument);
    argument = 0;

     argument=(P_acum/SAMPLE_CYCLE);                    //Calculating the Active Power value
     P = argument * ka * kv;
     if (P < 0)
     {
         P=0;
     }
     argument=0;

     voltage.all_VI = V_rms * kv;                       //Adjust real voltage value
     current.all_VI = I_rms * ka;                       //Adjust real current value
     ActPower.all_VI = P;

     S=voltage.all_VI*current.all_VI;                  //Calculating Apparent Power
     ReactPower.all_VI = S;
     PF=P/S;                                            //Calculating the Power Factor
     PowerFactor.all_VI = PF;

     E += P * kw;                                       //Calculating energy in Wh
     energy.all_VI = E;
     React_E += S * kw;
     ReactEnergy.all_VI = React_E;




     s_count=0;                                         //clear sample count
     I_acum=0;                                          //clear accumulators
     V_acum=0;
     P_acum=0;
     I_buffer_accum = 0;
     V_buffer_accum = 0;
     V1_avg = 0;                                        //clear average values
     I1_avg = 0;

}


void PWM(idx, s)
{
    if (s == 1)                                         //PWM ON
    {
        TACCTL1 = 0x00;                                 //Clear PWM configuration
        R0 = LimR0[idx];                                 //frequency vector
        R1 = LimR1[idx];                                 // duty cycle vector
//        R0 = 42;                                 //frequency vector
//        R1 = 21;                                 // duty cycle vector
        TACTL = 0x00;
        TACTL |= TACLR;                                 //Clear Timer A Counter
        TACCR0 = R0;                                    // PWM Frequency
        TACCTL1 = OUTMOD_6;                             // CCR1 toggle/set
        TACCR1 = R1;                                    // PWM duty cycle
        TACTL = TASSEL_2 + MC_1;                        // SMCLK, up mode
    }
    else                                                //PWM OFF
    {
        TACCTL1 = 0x00;
    }

}

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
        U0TXBUF = TEST;
        while((U0TCTL & TXEPT) != 1);
        __delay_cycles(100);

        if (bufferRX == TEST)
        {
//                  P2OUT = BIT0;
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
//       _delay_cycles(108550000);
    }
    f = j / k_counter;
    freq_adjust_flag = 0;
    return f;
}

#pragma vector=USART0RX_VECTOR
__interrupt void USART0RX ()
{
    if (freq_adjust_flag == 1)  // a way to separate the frequency sweep and the command of the system
    {
        bufferRX = U0RXBUF;
    }
    else
    {
//        P2OUT ^= BIT0;
        command(U0RXBUF);
    }


}

#pragma vector=SD24_VECTOR
__interrupt void SD24AISR(void)
{
  switch (SD24IV)
  {
  case 2:                                   // SD24MEM Overflow
      break;
  case 4:                                   // SD24MEM0 IFG
      V_AD_RESULT = SD24MEM0;
      I_AD_RESULT = SD24MEM1;
      V_buffer[s_count] = V_AD_RESULT;
      I_buffer[s_count] = I_AD_RESULT;
      V_buffer_accum += V_AD_RESULT;        // Accum to calculate average
      I_buffer_accum += I_AD_RESULT;
      s_count++;
    break;
  case 6:                                   // SD24MEM1 IFG
    break;
  case 8:                                   // SD24MEM2 IFG
      break;
  }

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
//If this interrupt occurs, then some gain is turning P1.4 off
//  P2OUT = BIT0;
  Port_flag = 0;
  P1IFG &= ~BIT4;                           // P1.4 IFG cleared
}


void main(void)
{
    clock_init();
    __bis_SR_register(GIE);       // Enable interrupt
    gain_adjust();
    port_init();
    UART_init();
    frequency = PWM_adjust();
    PWM(frequency,0);
    sd_init();

    while(1)
    {
        if(s_count == SAMPLE_CYCLE) // S_COUNT = SAMPLE_CYCLE
        {
//            P2OUT ^= BIT0;
            SD24CCTL1 &= ~SD24SC;                     // Set bit to stop conversion
            calculate();
//            test
//            PWM(frequency,1);
//            U0TXBUF = 'T';
//            while((U0TCTL & TXEPT) != 1);
//            PWM(frequency,0);
            if(send == 1)
            {
                P2OUT ^= BIT0;
                PWM(frequency,1);
                send_param(frequency);
                PWM(frequency,0);
                send = 0;
            }
            if(send_relay != 'N')
            {
//                PWM(frequency,1);
                relay_resp(send_relay);
//                PWM(frequency,0);
            }
            SD24CCTL1 |= SD24SC;

        }
    }
	
}
