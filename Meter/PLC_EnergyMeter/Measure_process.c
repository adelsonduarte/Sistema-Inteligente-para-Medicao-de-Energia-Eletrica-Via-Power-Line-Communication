/*
 * Measure_process.c
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */

#include "Measure_process.h"

void calculate() {
  unsigned int count;
  int I_buffer_accum = 0, V_buffer_accum = 0;
  signed long int V1_avg = 0, I1_avg = 0;
  signed long long int V_acum = 0, I_acum = 0, V_operand = 0, I_operand = 0;
  signed long long int P_acum = 0;
  float V_rms = 0, I_rms = 0;
  float PF = 0, S = 0, P = 0, E = 0, React_E = 0;
  float argument = 0;

  V1_avg = V_buffer_accum / SAMPLE_CYCLE; //Average value
  I1_avg = I_buffer_accum / SAMPLE_CYCLE; //Average value

  for (count = 0; count < SAMPLE_CYCLE; count++) //Subtract average value for each sample
  {
    V_buffer[count] -= V1_avg;
    I_buffer[count] -= I1_avg;

  }
  for (count = 0; count < SAMPLE_CYCLE; count++) //Loop for RMS Calculations
  {
    I_operand = I_buffer[count];
    V_operand = V_buffer[count];
    I_acum += I_operand * I_operand;
    V_acum += V_operand * V_operand;
    P_acum += V_operand * I_operand;
  }

  argument = I_acum / SAMPLE_CYCLE; //RMS Calculation
  I_rms = sqrt(argument);
  argument = 0;

  argument = V_acum / SAMPLE_CYCLE; //RMS Calculation
  V_rms = sqrt(argument);
  argument = 0;

  argument = (P_acum / SAMPLE_CYCLE); //Calculating the Active Power value
  P = argument * ka * kv;
  if (P < 0) {
    P = 0;
  }
  argument = 0;

  voltage.all_VI = V_rms * kv; //Adjust real voltage value
  current.all_VI = I_rms * ka; //Adjust real current value
  ActPower.all_VI = P;

  S = voltage.all_VI * current.all_VI; //Calculating Apparent Power
  ReactPower.all_VI = S;
  PF = P / S; //Calculating the Power Factor
  PowerFactor.all_VI = PF;

  E += P * kw; //Calculating energy in Wh
  energy.all_VI = E;
  React_E += S * kw;
  ReactEnergy.all_VI = React_E;

  I_acum = 0; //clear accumulators
  V_acum = 0;
  P_acum = 0;
  I_buffer_accum = 0;
  V_buffer_accum = 0;
  V1_avg = 0; //clear average values
  I1_avg = 0;

}
