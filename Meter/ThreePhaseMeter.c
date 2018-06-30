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
#include "driverlib.h"
#include "math.h"
#include <stdlib.h>

#define iddle '0'
#define ready '1'
#define id    '2'
#define get   '3'
#define set   '4'
#define value '5'

#define Kv 0.01477831815014380000       //Voltage gain
#define Ka 0.00150078539069476          //Current gain
#define Kap 0.00002217908398            //Power gain
#define kw_factor 0.0002777777778       //Conversion to Wh
#define SAMPLES_SHIFT_R 12              // 2^12 = 4096
#define ONE_SEC_SAMPLES 4096            //Number of Samples at 1 second
#define WAVE_SAMPLES 136                // Approximately 2 cycles
#define FREQUENCY_AMOUNT 23
#define TEST 'T'
#define gain_1 '6'
#define gain_2 '7'
#define gain_3 '8'
#define gain_4 '9'

//Variaveis;
float argument;
float PF1, PF2, PF3,                //Power factors
      S1, S2, S3;                   //Apparent Power
float V1_RMS, V2_RMS, V3_RMS,       //RMS VALUES
      I1_RMS, I2_RMS, I3_RMS,
      AP1,    AP2,    AP3,          //Active Power
      EA1,    EA2,    EA3;          //Active Energy
unsigned long long int V1_intermed, V2_intermed, V3_intermed,
                       I1_intermed, I2_intermed, I3_intermed,
                       P1_intermed, P2_intermed, P3_intermed,
                       V1_intermed2, V2_intermed2, V3_intermed2,
                       I1_intermed2, I2_intermed2, I3_intermed2,
                       P1_intermed2, P2_intermed2, P3_intermed2;

int V1_AD_RESULT, V2_AD_RESULT, V3_AD_RESULT,
    I1_AD_RESULT, I2_AD_RESULT, I3_AD_RESULT;

const short int LimR0[23] = {42,41,40,39,38,37,36,35,34,33,32,31,30,29,28,27,26,25,24,23,22,21,20};
const short int LimR1[23] = {21,21,20,20,19,19,18,18,17,17,16,16,15,15,14,14,13,13,12,12,11,11,10};

int frequency=0;
short int idx = 0;
short int R0 = 0, R1 = 0;
short int freq_adjust_flag = 0;
short int Port_flag = 1;

long int V1_avg = 0, V2_avg = 0, V3_avg = 0,
         I1_avg = 0, I2_avg = 0, I3_avg = 0;

long long int V1_avg_accum, V2_avg_accum, V3_avg_accum,
              I1_avg_accum, I2_avg_accum, I3_avg_accum,
              V1_avg_accum2, V2_avg_accum2, V3_avg_accum2,
              I1_avg_accum2, I2_avg_accum2, I3_avg_accum2;



signed int V1_buffer[WAVE_SAMPLES], V2_buffer[WAVE_SAMPLES], V3_buffer[WAVE_SAMPLES],
           I1_buffer[WAVE_SAMPLES], I2_buffer[WAVE_SAMPLES], I3_buffer[WAVE_SAMPLES];
char mode;

unsigned int scount=0, first_sec_discard;
int one_sec = 0, send = 0;
unsigned char bufferRX;
unsigned char gain_state;

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

union VI voltage1;
union VI current1;
union VI energy1;

union VI ActPower1;
union VI ActPower2;
union VI ActPower3;
union VI PowerFactor1;
union VI PowerFactor2;
union VI PowerFactor3;


union VI voltage2;
union VI current2;
union VI energy2;

union VI voltage3;
union VI current3;
union VI energy3;

//-------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned char uart_state;




//Functions prototypes
void clock_init();
void sd_init();
void port_init();
void USART_init();
void RTC_init();
void measurement();
void gain_adjust();
void calculate();
void command(unsigned char);
void send_param();
void read_buffer();
void erase();
void PWM(int j, int s);

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
//    UCA1BRW_L = 0xB4;                       // ~4200kHz/9600
//    UCA1BRW_H = 0x01;                       //
//    UCA1MCTLW = 0x0400;                     // Modulation UCBRSx=0x53, UCBRFx=0
    UCA1CTLW0 &= ~UCSWRST;                  // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt
}

void sd_init()
{
    int j=0;
    //***** Configure SD24B *****//
//    SD24BCTL0 |= SD24REFS | SD24SSEL__SMCLK | SD24PDIV_2;       // Select internal REF
//                                                              // Select SMCLK as SD24_B clock source
//                                                             // Set clock divider by 4

//    SD24BCTL0 |= SD24REFS | SD24SSEL__SMCLK | SD24DIV4 | SD24DIV1 | SD24DIV0 ;       // Select internal REF
//                                                              // Select SMCLK as SD24_B clock source
//                                                             // Set clock divider by 20
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

//    __bis_SR_register(GIE);

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

//    P1DIR |= BIT2 | BIT4 | BIT5;            // ACLK, MCLK, SMCLK set out to pins
//    P1SEL0 |= BIT2 |  BIT4 | BIT5;            // P1.2,4,5 for debugging purposes.

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
//    PMMCTL0_H = PMMPW_H;                                   // Unlock all PMM registers, (PMMCOREV = 0, SVSMHRRL = 0)
//    PMMCTL0 = PMMCOREV_1;
//    PMMCTL0_H = 0;                                         // Lock all PMM registers
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

    //           UCSCTL0 = 0x0000;                      // Set lowest possible DCOx, MODx
    //           UCSCTL1 = DCORSEL_4;                   // Select DCO range 16MHz operation
    //           UCSCTL2 = FLLD_2 | 127;                // Set DCO Multiplier for 16MHz
    //                                                      // D * (N + 1) * FLLRef = Fdco
    //                                                      // 4 *(127 + 1) * 32768 = 16.777,216 MHz
    //                                                      // Set FLL Div = fDCOCLK/1
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

void PWM(idx, s)
{
    short int i = 0;
    if (s == 1)
    {
        TA0CCTL2 = 0x00;
        R0 = LimR0[idx];
        R1 = LimR1[idx];
        TA0CCR0 = R0;
        TA0CCTL2 = OUTMOD_6;                    // // CCR1 toggle/set
        TA0CCR2 = R1;
        TA0CTL = TASSEL_2 | MC_1 | TACLR;       // SMCLK, up-mode, clear TAR
//        for(i=0;i<20000000000;i++);
//        _delay_cycles(43380000);
    }
    else
    {
        TA0CTL |= TACLR;                         //clear TAR
        TA0CCTL2 = 0x00;                         // Turn off PWM configuration
    }


}


void send_param()
{
    int count = 0;
    short int send_count;

    // Disable USART0 RX interrupt
    UCA1IE = 0X00;

    UCA1TXBUF = '*';      // Send Header Start
    while (!(UCA1IFG & UCTXIFG)) ;

//    //Send V2 wave to Labview (DEBUGUING)
//    for(count = 0; count < WAVE_SAMPLES; count ++)
//    {
//       voltage2_wave.all_wave = V2_buffer[count];
//       for(send_count = 2; send_count > 0; send_count --)
//       {
//           UCA1TXBUF = voltage2_wave.pt_wave[(send_count-1)];
//           while (!(UCA1IFG & UCTXIFG)) ;
//       }
//    }
//
//    count = 0;

//    //Send I2 wave to Labview (DEBUGUING)
//    for(count = 0; count < WAVE_SAMPLES; count ++)
//    {
//       current2_wave.all_wave = I2_buffer[count];
//       for(send_count = 2; send_count > 0; send_count --)
//       {
//           UCA1TXBUF = current2_wave.pt_wave[(send_count-1)];
//           while (!(UCA1IFG & UCTXIFG)) ;
//       }
//    }
//
//    count = 0;


    // Send V1 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = voltage1.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send V2 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = voltage2.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }


    // Send V3 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = voltage3.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send I1 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = current1.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

//    // Send I2 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
   {
       UCA1TXBUF = current2.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }



    // Send I3 RMS serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = current3.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

//    // Send sample counter serial
//    for(send_count = 2; send_count > 0; send_count --)
//    {
//        UCA1TXBUF = contador.pt_wave[(send_count-1)];
//        while (!(UCA1IFG & UCTXIFG)) ;
//    }

    // Send E1 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = energy1.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

//    // Send E2 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = energy2.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send E3 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = energy3.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send Active Power 1 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = ActPower1.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }


//    // Send Active Power 2 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = ActPower2.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send Active Power 3 serial
    for(send_count = 4; send_count > 0; send_count --)
    {
       UCA1TXBUF = ActPower3.pt_VI[(send_count-1)];
       while (!(UCA1IFG & UCTXIFG)) ;
    }

    // Send Power Factor 1 serial
     for(send_count = 4; send_count > 0; send_count --)
     {
        UCA1TXBUF = PowerFactor1.pt_VI[(send_count-1)];
        while (!(UCA1IFG & UCTXIFG)) ;
     }

//    // Send Power Factor 2 serial
     for(send_count = 4; send_count > 0; send_count --)
     {
        UCA1TXBUF = PowerFactor2.pt_VI[(send_count-1)];
        while (!(UCA1IFG & UCTXIFG)) ;
    }

     // Send Power Factor 3 serial
      for(send_count = 4; send_count > 0; send_count --)
      {
         UCA1TXBUF = PowerFactor3.pt_VI[(send_count-1)];
         while (!(UCA1IFG & UCTXIFG)) ;
      } 

    UCA1TXBUF = '!';      // Send Header out
    while (!(UCA1IFG & UCTXIFG)) ;

    UCA1IE = UCRXIE;    // Turn ON UART interrupt


}

void proc_command(unsigned char c)
{

  if(c == '#')
  {
      uart_state = ready;
  }

  else
  {
     if(uart_state != iddle)
     {
        switch(uart_state)
        {
          case ready:
            if(c == '0')  // Identificação  - Cada módulo slave tera um id definido por software
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
              default:
                  uart_state = iddle;
            }
                break;

          case get:
            if(c == ';')
            {
//                send = 1;
                PWM(frequency,1);
                P7OUT ^= BIT2;
                send_param();
//                send = 0;
                PWM(frequency,0);
            }

            uart_state = iddle;
            break;

          case set:
            uart_state = iddle;
            break;

          default:
              uart_state = iddle;
         }
     }
  }
}
void offset()
{
    V1_avg = V1_avg_accum2 >> SAMPLES_SHIFT_R;
    V2_avg = V2_avg_accum2 >> SAMPLES_SHIFT_R;
    V3_avg = V3_avg_accum2 >> SAMPLES_SHIFT_R;

    I1_avg = I1_avg_accum2 >> SAMPLES_SHIFT_R;
    I2_avg = I2_avg_accum2 >> SAMPLES_SHIFT_R;
    I3_avg = I3_avg_accum2 >> SAMPLES_SHIFT_R;

}
void calculate()
{
    //V1 RMS
    argument= V1_intermed2 >> SAMPLES_SHIFT_R;
    V1_RMS = sqrt(argument);
    V1_RMS = V1_RMS*Kv;
    argument=0;
    voltage1.all_VI = V1_RMS;

    //V2 RMS
    argument= V2_intermed2 >> SAMPLES_SHIFT_R;
    V2_RMS = sqrt(argument);
    V2_RMS = V2_RMS*Kv;
    argument=0;
    voltage2.all_VI = V2_RMS;

    //V3 RMS
    argument= V3_intermed2 >> SAMPLES_SHIFT_R;
    V3_RMS = sqrt(argument);
    V3_RMS = V3_RMS*Kv;
    argument=0;
    voltage3.all_VI = V3_RMS;

    //I1 RMS
    argument= I1_intermed2 >> SAMPLES_SHIFT_R;
    I1_RMS = sqrt(argument);
    I1_RMS = I1_RMS*Ka;
    argument=0;
    current1.all_VI = I1_RMS;

    //I2 RMS
    argument= I2_intermed2 >> SAMPLES_SHIFT_R;
    I2_RMS = sqrt(argument);
    I2_RMS = I2_RMS*Ka;
    argument=0;
    current2.all_VI = I2_RMS;


    //I3 RMS
    argument= I3_intermed2 >> SAMPLES_SHIFT_R;
    I3_RMS = sqrt(argument);
    I3_RMS = I3_RMS*Ka;
    current3.all_VI= I3_RMS;
    argument=0;

    //Active Power 1
    argument = P1_intermed2 >> SAMPLES_SHIFT_R;
    AP1 = argument*Kap;
    argument=0;

    //Active Power 2
    argument = P2_intermed2 >> SAMPLES_SHIFT_R;
    AP2 = argument*Kap;
    ActPower2.all_VI = AP2;
    argument=0;

    //Active Power 3
    argument = P3_intermed2 >> SAMPLES_SHIFT_R;
    AP3 = argument*Kap;
    argument=0;

    //Energy 1
/*    MPY32_setOperandOne32Bit(MPY32_MULTIPLY_SIGNED,AP1);
    MPY32_setOperandTwo16Bit(ONE_SEC_SAMPLES);
    EA1 = MPY32_getResult();*/

    EA1 += AP1 * kw_factor;
    energy2.all_VI = EA1;

    //Energy 2
    /*MPY32_setOperandOne32Bit(MPY32_MULTIPLY_SIGNED,AP2);
    MPY32_setOperandTwo16Bit(ONE_SEC_SAMPLES);
    EA2 = MPY32_getResult();
    energy2.all_VI = EA2;*/

    EA2 += AP2 * kw_factor;
    energy2.all_VI = EA2;

    //Energy 3
/*    MPY32_setOperandOne32Bit(MPY32_MULTIPLY_SIGNED,AP3);
    MPY32_setOperandTwo16Bit(ONE_SEC_SAMPLES);
    EA3 = MPY32_getResult(); */

    EA3 += AP3 * kw_factor;
    energy3.all_VI = EA3;

    S1 = V1_RMS * I1_RMS;
    S2 = V2_RMS * I2_RMS;
    S3 = V3_RMS * I3_RMS;

    PF1 = AP1/S1;
    PF2 = AP2/S2;
    PowerFactor2.all_VI = PF2;
    PF3 = AP3/S3;

}

void read_buffer()
{
    V1_avg_accum2 = V1_avg_accum;
    V1_avg_accum = 0;

    V2_avg_accum2 = V2_avg_accum;
    V2_avg_accum = 0;

    V3_avg_accum2 = V3_avg_accum;
    V3_avg_accum = 0;

    I1_avg_accum2 = I1_avg_accum;
    I1_avg_accum = 0;

    I2_avg_accum2 = I2_avg_accum;
    I2_avg_accum = 0;

    I3_avg_accum2 = I3_avg_accum;
    I3_avg_accum = 0;

    V1_intermed2 = V1_intermed;
    V1_intermed = 0;

    V2_intermed2 = V2_intermed;
    V2_intermed = 0;

    V3_intermed2 = V3_intermed;
    V3_intermed = 0;

    I1_intermed2 = I1_intermed;
    I1_intermed = 0;

    I2_intermed2 = I2_intermed;
    I2_intermed = 0;

    I3_intermed2 = I3_intermed;
    I3_intermed = 0;

    P1_intermed2 = P1_intermed;
    P1_intermed = 0;

    P2_intermed2 = P2_intermed;
    P2_intermed = 0;

    P3_intermed2 = P3_intermed;
    P3_intermed = 0;
}

void buffer_ad()
{
    if(scount < WAVE_SAMPLES) //Buffer to send wave by serial (LABVIEW only)
    {
        //V1_buffer[scount] = V1_AD_RESULT;
      V2_buffer[scount] = V2_AD_RESULT;
      //V3_buffer[scount] = V3_AD_RESULT;

      //I1_buffer[scount] = I2_AD_RESULT;
        I2_buffer[scount] = I2_AD_RESULT;
      //I3_buffer[scount] = I2_AD_RESULT;
     }
                             //
    //Accumulate Channel offset
    V1_avg_accum += V1_AD_RESULT;
    V2_avg_accum += V2_AD_RESULT;
    V3_avg_accum += V3_AD_RESULT;
    I1_avg_accum += I1_AD_RESULT;
    I2_avg_accum += I2_AD_RESULT;
    I3_avg_accum += I3_AD_RESULT;
    //

    //Subtract channel offset for each channel sample
    V1_AD_RESULT -= V1_avg;
    V2_AD_RESULT -= V2_avg;
    V3_AD_RESULT -= V3_avg;
    I1_AD_RESULT -= I1_avg;
    I2_AD_RESULT -= I2_avg;
    I3_AD_RESULT -= I3_avg;
    //

    //Intermediate RMS value
    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V1_AD_RESULT);
    MPY32_setOperandTwo16Bit(V1_AD_RESULT);
    V1_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V2_AD_RESULT);
    MPY32_setOperandTwo16Bit(V2_AD_RESULT);
    V2_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V3_AD_RESULT);
    MPY32_setOperandTwo16Bit(V3_AD_RESULT);
    V3_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,I1_AD_RESULT);
    MPY32_setOperandTwo16Bit(I1_AD_RESULT);
    I1_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,I2_AD_RESULT);
    MPY32_setOperandTwo16Bit(I2_AD_RESULT);
    I2_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,I3_AD_RESULT);
    MPY32_setOperandTwo16Bit(I3_AD_RESULT);
    I3_intermed += MPY32_getResult();

    //Intermediate Power value
    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V1_AD_RESULT);
    MPY32_setOperandTwo16Bit(I1_AD_RESULT);
    P1_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V2_AD_RESULT);
    MPY32_setOperandTwo16Bit(I2_AD_RESULT);
    P2_intermed += MPY32_getResult();

    MPY32_setOperandOne16Bit(MPY32_MULTIPLY_SIGNED,V3_AD_RESULT);
    MPY32_setOperandTwo16Bit(I3_AD_RESULT);
    P3_intermed += MPY32_getResult();

}

void erase()
{
    V1_intermed2 = 0;
    V2_intermed2 = 0;
    V3_intermed2 = 0;
    I1_intermed2 = 0;
    I2_intermed2 = 0;
    I3_intermed2 = 0;
    P1_intermed2 = 0;
    P2_intermed2 = 0;
    P3_intermed2 = 0;
    V1_avg_accum2 = 0;
    V2_avg_accum2 = 0;
    V3_avg_accum2 = 0;
    I1_avg_accum2 = 0;
    I2_avg_accum2 = 0;
    I3_avg_accum2 = 0;

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
        UCA1TXBUF = TEST;
        while (!(UCA1IFG & UCTXIFG)) ;
        __delay_cycles(100);

        if (bufferRX == TEST)
        {
          P7OUT ^= BIT2;
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

//       _delay_cycles(108500000);
    }
    f = j / k_counter;
    freq_adjust_flag = 0;
    return f;
}

void gain_adjust()
{
    //P2.5 Gain_2, P2.6 Gain_1

    //  Gain_2  |   Gain_1  |   R   |           |  Ganho        R1 = 470
    //   0            0        470    R             2.6         R2 = 150
    //   0            1        235  (R//R1)         4.2
    //   1            0        113   (R//R2)        6.0
    //   1            1        110   (R//R1//R2)    7.6

    unsigned short int c = 0,j=0;
//    P3IE |= BIT4;                               // P3.4 interrupt enabled
//    P3IES |= BIT4;                              // P3.4 Hi/lo edge
//    P3IFG &= ~BIT4;                             // P3.4 IFG cleared
    P2DIR |= BIT5 | BIT6;                       // Ganho 2 e ganho 1
    gain_state = gain_1;
    while (c!=1)
    {
//        Machine state to gain adjust
        switch(gain_state)
        {
            case gain_1:
                P2OUT &= ~BIT5 & ~BIT6;
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
                P2OUT |= BIT6;
                if (Port_flag == 1)
                {
//                    P1OUT = BIT6;                         //Set P1.6 to increase gain (4.2)
                    gain_state = gain_3;
                }
                else
                {
                    gain_state = gain_1;
                    P2OUT &= ~BIT5 & ~BIT6;
                    c=1;                                   //Minimum gain doesnt work in this powerline
                }
            break;

            case gain_3:
                P2OUT = BIT5;
                for (j = 0; j < 0x3600; j++);               // Delay
                if (Port_flag == 1)
                {
//                                             //gain2 is ok. Lets increase this gain (6.0)
                   gain_state = gain_4;
                }
                else
                {
                    P2OUT = BIT6;
                    gain_state = gain_2;
                    c=1;
//                    P1OUT &= ~BIT6;                       //gain2 is too high, lets return to minimum gain
                }
                break;

            case gain_4:
                P2OUT |= BIT6;
                for (j = 0; j < 0x3600; j++);               // Delay
                if (Port_flag == 1)
                {
//                                            //Set P1.6 to increase gain (7.6)
//                    gain_state = gain_end;
                    c=1;
                }
                else
                {
                    P2OUT |= BIT6;
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
            buffer_ad();
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
    clock_init();
    __bis_SR_register(GIE);
//    gain_adjust(); //Verificar como implementar (Não tem interrupção no pino referente ao RX
    port_init();
    USART_init();
    frequency = PWM_adjust();
//    PWM(frequency,1);
    sd_init();
    RTC_init();


    P7OUT &= ~BIT2;

    while(1)
    {

        if(one_sec  == 1) //one_sec = 1 when RTC count up to 1 second
        {
//            contador.all_wave = scount; //to send scount by serial (LABVIEW)
//            P7OUT ^= BIT2;
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
