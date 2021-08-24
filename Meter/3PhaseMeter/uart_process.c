#include <msp430f67791.h>
#include "uart_process.h"

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
            if(c == '0')  // Id
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
                send = 1;
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

void send_param()
{
    //int count = 0;
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



