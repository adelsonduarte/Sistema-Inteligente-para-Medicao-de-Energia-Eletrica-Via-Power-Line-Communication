/* -----------------------------------------------------------------------------------------------------------------------------------
 * 3-Phase Power Meter
 * Referência: mplementation of a Three-Phase Electronic Watt-Hour Meter Using the MSP430F677x(A)
 * Autor: Adelson Duarte dos Santos
 * Data inicio: 26/09/17
 * Data de Término: 04/10/17
 *
 * -----------------------------------------------------------------------------------------------------------------------------------
 *  Parâmetros
 *
 *  1 - Conversor AD
 *  a - Ganho de corrente
 *  IRMSmax = 50A, Ipico = 50x1,4 = 70.7A
 *  Relação de transformação TC: 1:2500
 *  Rshunt = 15ohm
 *  Vp entrada AD = 424,2mV
 *  Vrms entrada AD = 300mV
 *  SD24GAINx = 4
 *
 *  b - Ganho de Tensão
 *  VRMS max 230V, VPico = 230 x 1,4 = 325V
 *  Divisor de tensão = 0,002418379686
 *  Vp entrada AD max = 786mV
 *  Vrms entrada AD max = 556mV
 *  SD24GAINx = 1
 *
 *  c - Canais
 *  Canal 0: V1
 *  Canal 1: V2
 *  Canal 2: V3
 *  Canal 4: I1
 *  Canal 5: I2
 *  Canal 6: I3
 *
 *  d - Configuração
 *  Frequency AD 1.048576MHz
 *  OSR = 256
 *  Fsampling = 4096 Samples/s
 *
 *  2 - Microcontrolador
 *  ACLK = 32768 Hz
 *  MCLK =  21.692,416 MHz
 *  SMCLK =   21.692,416 MHz
 *
 *  3 - UART Configuration
 *  BAUDRATE 9600b/s
 *  CLK = SMCLK
 *
 *
*/

#include <msp430f67791.h>
#include <stdlib.h>
#include <stdint.h>
#include "driverlib.h"
#include "math.h"
#include "uart_process.c"
#include "Measurement.c"
#include "3P_electric_param_calculation.c"
#include "PWM.c"

#define ONE_SEC_SAMPLES 4096            //Number of Samples at 1 second

//Variaveis;
int frequency=0;
unsigned int scount=0, first_sec_discard;
int one_sec = 0;;
uint8_t send = 0;
//-------------------------------------------------------------------------------------------------------------------------------------------------------

//Functions prototypes
void clock_init();
void sd_init();
void port_init();
void USART_init();
void RTC_init();

void RTC_init()
{
    // Configure RTC_C
    RTCCTL0_H = RTCKEY_H;                   // Unlock RTC_C module
    RTCCTL0_L |= RTCTEVIE | RTCAIE | RTCRDYIE; // Enable RTC time event, alarm event,
                                                // read ready interrupt
    RTCCTL1 |= RTCBCD | RTCHOLD;            // RTC enable BCD mode, RTC hold

    RTCCTL1 &= ~(RTCHOLD);                  // Start RTC calendar mode
    RTCCTL0_H = 0;                          // Lock RTC_C module
    first_sec_discard = 1;

}

void USART_init()
{
    UCA1CTLW0 |= UCSWRST;                   // **Put state machine in reset**
    UCA1CTLW0 |= UCSSEL_2;                  // CLK = SMCLK
    UCA1BRW_L = 0xCA;                       // ~4200kHz/9600
    UCA1BRW_H = 0x08;                       //
    UCA1MCTLW = 0x0300;                     // Modulation UCBRSx=0x53, UCBRFx=0
    UCA1CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
}

void sd_init()
{
    int j=0;
    //***** Configure SD24B *****//
    SD24BCTL0 |= SD24REFS | SD24SSEL__SMCLK | SD24DIV4 | SD24DIV2;       // Select internal REF
                                                              // Select SMCLK as SD24_B clock source
                                                             // Set clock divider by 21
    for (j = 0; j < 0x3600; j++);               // Delay for 1.2V ref startup
    j=0;

    //*** Configure SD24 channel 0 V1, Voltage Sense ***//
    SD24BCCTL0 |= SD24DF0 | SD24ALGN | SD24SCS_4;            // Data form = Twos complement, Group 0, left alligned
    SD24BINCTL0 |= SD24GAIN_1;                               // set PGA gain to 1

    //*** Configure SD24 channel 1 V2, Voltage Sense ***//
    SD24BCCTL1 |= SD24DF0 | SD24SCS_4 | SD24ALGN;            // Data form = Twos complement, Group 0, left alligned
    SD24BINCTL1 |= SD24GAIN_1 ;                              // Set PGA gain to 1

    //*** Configure SD24 channel 2 V3, Voltage Sense ***//
    SD24BCCTL2 |= SD24DF0 | SD24SCS_4 | SD24ALGN;           // Data form = Twos complement, Group 0, left alligned
    SD24BINCTL2 |= SD24GAIN_1 ;                              // Set PGA gain to 1

     //*** Configure SD24 channel 4 I1 ***//
     SD24BCCTL4 |= SD24DF0 | SD24SCS_4 | SD24ALGN;          // Data form = Twos complement, Group 0, left alligned
     SD24BINCTL4 |= SD24GAIN_4;                             // Set PGA gain to 4

     //*** Configure SD24 channel 5 I2 ***//
     SD24BCCTL5 |= SD24DF0 | SD24SCS_4 | SD24ALGN;          // Data form = Twos complement, Group 0, left alligned
     SD24BINCTL5 |= SD24GAIN_4;                             // Set PGA gain to 4

     //*** Configure SD24 channel 6 I3 ***//
     SD24BCCTL6 |= SD24DF0 | SD24SCS_4 | SD24ALGN;          // Data form = Twos complement, Group 0, left alligned
     SD24BINCTL6 |= SD24GAIN_4;                             // Set PGA gain to 4

     SD24BIE |= SD24IE0;

     for (j = 0; j < 0x3600; j++);               // Delay for 1.2V ref startup
     j=0;

    SD24BCTL1 |= SD24GRP0SC;

    for(j=0; j<10; j++)
    {
        while(scount < ONE_SEC_SAMPLES);
        scount = 0;
    }

}

void port_init()
{
    P3SEL0 |= BIT4 | BIT5;                   // Set P3.0, P3.1 to non-IO
    P3DIR |= BIT4 | BIT5;                   // Enable UCA1RXD, UCA1TXD

    P7DIR |= BIT2 | BIT0;                   //LEDS
    P7OUT &= ~BIT2;
    P7OUT &= ~BIT0;

    P2DIR |= BIT0;                          // P2.0/TA0.1 output
    P2SEL0 |= BIT0;                          // Output TA0
    P2DIR |= BIT1;                          // P2.1/TA0.1 output
    P2SEL0 |= BIT1;                          // Output TA1
    P2DIR |= BIT2;                          // P2.2/TA0.2 output
    P2SEL0 |= BIT2;                          // Output TA2

    P7OUT = BIT2;
}


void clock_init()
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop WDT
    PMM_setVCore(1);
    PMM_setVCore(2);
    PMM_setVCore(3);


    P1DIR |= BIT4 | BIT5;            // ACLK, MCLK, SMCLK set out to pins
    P1SEL0 |= BIT4 | BIT5 ;            // P1.5 for debugging purposes.
    // Setup LFXT1
    UCSCTL6 &= ~(XT1OFF);                   // XT1 On

    // Loop until XT1 fault flag is cleared
    do
    {
        UCSCTL7 &= ~DCOFFG;                // Clear DCO fault flags
        UCSCTL7 &= ~XT1LFOFFG;             // Clear XT1 fault flags
    }   while (UCSCTL7 & XT1LFOFFG);         // Test XT1 fault flag

    UCSCTL6 &= ~(XT1DRIVE_3);               // XT1 stable, reduce drive strength

   // Initialize DCO to 16MHz
    __bis_SR_register(SCG0);               // Disable the FLL control loop

    UCSCTL0 = 0x0000;                      // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                   // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_0 | 661;                // Set DCO Multiplier for 16MHz
                                               // D * (N + 1) * FLLRef = Fdco
                                               // 1 *(661 + 1) * 32768 = 21.692,416 MHz
                                               // Set FLL Div = fDCOCLK/1
       __bic_SR_register(SCG0);               // Enable the FLL control loop

       // Worst-case settling time for the DCO when the DCO range bits have been
       // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
       // UG for optimization.
       // 32 x 32 x 16 MHz / 32,768 Hz = 250000 = MCLK cycles for DCO to settle
       __delay_cycles(4000000);

       // Loop until XT1, XT2 & DCO fault flag is cleared
       do
           {
               UCSCTL7 &= ~(XT2OFFG | XT1LFOFFG | DCOFFG);     // Clear XT2,XT1,DCO fault flags
               SFRIFG1 &= ~OFIFG;                 // Clear clock fault flag
           } while (SFRIFG1 & OFIFG);             // Test oscillator fault flag

       UCSCTL4 = SELM__DCOCLK | SELS__DCOCLKDIV | SELA__XT1CLK; //select clock sources for MCLK, SMCLK and ACLK.
       __delay_cycles(500);
}

// RTC Interrupt Service Routine
#pragma vector=RTC_VECTOR
__interrupt void rtc_isr(void)
{
    switch (__even_in_range(RTCIV, 16))
    {
        case RTCIV_NONE:                    // No interrupts
            break;
        case RTCIV_RTCOFIFG:                // RTCOFIFG
            break;
        case RTCIV_RTCRDYIFG:               // RTCRDYIFG
            one_sec = 1;                              // Toggles P1.0 every second
            break;
        case RTCIV_RTCTEVIFG:               // RTCEVIFG
            __no_operation();               // Interrupts every minute
            break;
        case RTCIV_RTCAIFG:                 // RTCAIFG
            __no_operation();               // Interrupts every alarm event
            break;
        case RTCIV_RT0PSIFG:                // RT0PSIFG
            break;
        case RTCIV_RT1PSIFG:                // RT1PSIFG
            break;
//        case 14: break;                     // Reserved
        case 16: break;                     // Reserved
        default: break;
    }
}

#pragma vector=SD24B_VECTOR
__interrupt void SD24BISR(void)
{ //approximately 50us to service the interruption
    switch (SD24BIV)
    {
        case SD24BIV_SD24OVIFG:             // SD24MEM Overflow
            break;
        case SD24BIV_SD24TRGIFG:            // SD24 Trigger IFG
            break;
        case SD24BIV_SD24IFG0:              // SD24MEM0 IFG
            V1_AD_RESULT = SD24BMEMH0;      //Transfer data from AD buffer to variable VX_ad_result
            V2_AD_RESULT = SD24BMEMH1;
            V3_AD_RESULT = SD24BMEMH2;
            I1_AD_RESULT = SD24BMEMH4;
            I2_AD_RESULT = SD24BMEMH5;
            I3_AD_RESULT = SD24BMEMH6;
            buffer_ad(scount);
            scount++;
            break;
    }
}

#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    switch (__even_in_range(UCA1IV, 4))
    {
        case USCI_UART_UCRXIFG:             // RXIFG
            if (freq_adjust_flag == 1)  // a way to separate the frequency sweep and the command of the system
            {
                bufferRX = UCA1RXBUF;
            }
            else
            {
                proc_command(UCA1RXBUF);
            }

            break;
        default: break;
    }
}

int main(void)
{
    int frequency = 0;
    clock_init();
    __bis_SR_register(GIE);
    port_init();
    USART_init();
    frequency = PWM_adjust();
    sd_init();
    RTC_init();
    P7OUT &= ~BIT2;

    while(1)
    {

        if(one_sec  == 1) //one_sec = 1 when RTC count up to 1 second
        {
            scount=0;
            read_buffer(); //***Separe the acquisition buffer (Vx_Intermed) from processing buffer (Vx_intermed2)
            if(first_sec_discard == 1) //Discard first second
            {

                first_sec_discard = 0; //Reset first second flag
                one_sec = 0; //Reset 1 second flag
                offset(); //Offset calculation
                erase();  //Erase all accumulators
            }
            else
            {
                one_sec = 0;
                calculate(); //Use previously offset to calculate RMS values and others
                offset();
                if(send == 1) //Flag to send by serial
                {
                    PWM(frequency,1);
                    P7OUT ^= BIT2;
                    send_param();
                    send = 0;
                    PWM(frequency,0);
                }
                erase();
            }
        }
    }
}
