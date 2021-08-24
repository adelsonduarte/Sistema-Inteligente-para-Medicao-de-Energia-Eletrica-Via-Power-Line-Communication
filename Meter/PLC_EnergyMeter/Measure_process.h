/*
 * Measure_process.h
 *
 *  Created on: 24 de ago de 2021
 *      Author: adels
 */

#ifndef MEASURE_PROCESS_H_
#define MEASURE_PROCESS_H_

#define kw 0.000462962963
#define kv 0.005925924783
#define ka 0.001001730261
#define SAMPLE_CYCLE 59

extern int I_buffer[59], V_buffer[59];
extern int I_AD_RESULT, V_AD_RESULT;
extern int I_buffer_accum, V_buffer_accum;

extern signed long int V1_avg, I1_avg;
extern signed long long int V_acum, I_acum, V_operand, I_operand;
extern signed long long int P_acum ;

extern float V_rms = 0, I_rms = 0;
extern float PF = 0, S = 0, P = 0, E = 0, React_E = 0;
extern float argument = 0;

union wave
{
    int all_wave;
    char pt_wave[2];

};

extern union wave voltage_wave;
extern union wave current_wave;
//----------------------------//
union VI
{
    float all_VI;
    char pt_VI[4];
};

extern union VI voltage;
extern union VI current;
extern union VI energy;
extern union VI ActPower;
extern union VI ReactEnergy;
extern union VI ReactPower;
extern union VI PowerFactor;

void calculate(void);

#endif /* MEASURE_PROCESS_H_ */
