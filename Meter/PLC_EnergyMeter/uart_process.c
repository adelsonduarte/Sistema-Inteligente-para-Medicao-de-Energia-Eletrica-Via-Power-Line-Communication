//start,MeterId,action,end

#include <msp430afe253.h>

#include "uart_process.h"

#include "Measure_process.h"

#include "relay_control.h"

short int send = 0;

void command(unsigned char c) {
  if (c == '#') {

    uart_state = ready;
  } 
  else {
    if (uart_state != iddle) {
      
	  switch (uart_state) {
      case ready:
        if (c == '1') {
          uart_state = id;
        } else uart_state = iddle;
        break;

      case id:
        switch (c) {
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
        if (c == ';') {
          send = 1;
        }

        uart_state = iddle;
        break;

      case set:
        if (c == ';') {
          send_relay = 'L';
        }

        uart_state = iddle;
        break;

      case reset:
        if (c == ';') {
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

void send_param(f) {
  unsigned short int count;
  unsigned short int send_count;

  union VI voltage;
  union VI current;
  union VI energy;
  union VI ActPower;
  union VI ReactEnergy;
  union VI ReactPower;
  union VI PowerFactor;
  /*union wave voltage_wave;
  union wave current_wave;*/

  IE1 = 0x00; // Disable USART0 RX interrupt

  U0TXBUF = '*'; // Send Header
  while ((U0TCTL & TXEPT) != 1);

  //                        Only for Labview Debug                             //
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
  //Send Voltage RMS
  for (count = 4; count > 0; count--) // Loop to send the voltage float value
  {
    U0TXBUF = voltage.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }
  //Send Current RMS
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = current.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }

  //Send PowerFactor
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = PowerFactor.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }

  //Send Energy
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = energy.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }

  //Send ReactEnergy
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = ReactEnergy.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }

  //Send ActPower
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = ActPower.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }
  //Send ReactPower
  for (count = 4; count > 0; count--) // Loop to send the current float value
  {
    U0TXBUF = ReactPower.pt_VI[(count - 1)];
    while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete
  }

  ////-------- Labview Debug---  //
  //   U0TXBUF = f;                                    //Send END
  //   while((U0TCTL & TXEPT) != 1);                   // Wait for the flag TXEPT -> Transmission Complete
  //// ----------------   //
  U0TXBUF = '!'; //Send END
  while ((U0TCTL & TXEPT) != 1); // Wait for the flag TXEPT -> Transmission Complete

  IE1 = URXIE0; // Enable USART0 RX interrupt

}
