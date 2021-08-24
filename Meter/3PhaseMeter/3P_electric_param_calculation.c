#include "Measurement.h"
#include "driverlib.h"
#include "uart_process.h"
#include "math.h"

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
    EA1 += AP1 * kw_factor;
    energy2.all_VI = EA1;

    //Energy 2
    EA2 += AP2 * kw_factor;
    energy2.all_VI = EA2;

    //Energy 3
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

