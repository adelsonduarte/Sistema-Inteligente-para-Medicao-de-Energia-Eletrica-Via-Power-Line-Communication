#include "Measurement.h"
#include "driverlib.h"

void buffer_ad(scount)
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



