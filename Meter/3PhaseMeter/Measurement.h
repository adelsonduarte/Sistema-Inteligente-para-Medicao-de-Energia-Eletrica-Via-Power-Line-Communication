
#ifndef MEASUREMENT_H_
#define MEASUREMENT_H_

#define SAMPLES_SHIFT_R 12              // 2^12 = 4096
#define WAVE_SAMPLES 136                // Approximately 2 cycles
#define Kv 0.01477831815014380000       //Voltage gain
#define Ka 0.00150078539069476          //Current gain
#define Kap 0.00002217908398            //Power gain
#define kw_factor 0.0002777777778       //Conversion to Wh

extern int V1_AD_RESULT, V2_AD_RESULT, V3_AD_RESULT,
           I1_AD_RESULT, I2_AD_RESULT, I3_AD_RESULT;

extern signed int V1_buffer[WAVE_SAMPLES], V2_buffer[WAVE_SAMPLES], V3_buffer[WAVE_SAMPLES],
                  I1_buffer[WAVE_SAMPLES], I2_buffer[WAVE_SAMPLES], I3_buffer[WAVE_SAMPLES];

extern long long int V1_avg_accum, V2_avg_accum, V3_avg_accum,
                     I1_avg_accum, I2_avg_accum, I3_avg_accum,
                     V1_avg_accum2, V2_avg_accum2, V3_avg_accum2,
                     I1_avg_accum2, I2_avg_accum2, I3_avg_accum2;

extern long int V1_avg = 0, V2_avg = 0, V3_avg = 0,
                I1_avg = 0, I2_avg = 0, I3_avg = 0;

extern unsigned long long int V1_intermed, V2_intermed, V3_intermed,
                              I1_intermed, I2_intermed, I3_intermed,
                              P1_intermed, P2_intermed, P3_intermed,
                              V1_intermed2, V2_intermed2, V3_intermed2,
                              I1_intermed2, I2_intermed2, I3_intermed2,
                              P1_intermed2, P2_intermed2, P3_intermed2;

extern float argument;
extern float PF1, PF2, PF3,                //Power factors
      S1, S2, S3;                   //Apparent Power
extern float V1_RMS, V2_RMS, V3_RMS,       //RMS VALUES
             I1_RMS, I2_RMS, I3_RMS,
             AP1,    AP2,    AP3,          //Active Power
             EA1,    EA2,    EA3;          //Active Energy

void buffer_ad(int);

#endif /* MEASUREMENT_H_ */
