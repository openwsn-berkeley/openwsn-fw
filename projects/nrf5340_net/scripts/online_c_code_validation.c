#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define NUM_SAMPLES           0xA0
#define NUM_SAMPLES_REFERENCE 64
#define LENGTH_PACKET         125+LENGTH_CRC // maximum length is 127 bytes
#define CHANNEL               0             // 24ghz: 11 = 2.405GHz, subghz: 11 = 865.325 in  FSK operating mode #1
#define FREQUENCY             
#define LENGTH_SERIAL_FRAME   ((NUM_SAMPLES*4)+10)   // length of the serial frame

#define DF_ENABLE             1
#define CALCULATE_ON_BOARD    1
#define INVAILD_ANGLE         361.0

#define ANGLE_HISTORY_LEN     16
#define INDEX_MASK            0x0f

#define UART_DEBUG            1

int16_t data[] = {158, 179, 199, -182, -160, -136, -112, -91, -71, -51, -32, -11, 11, 30, 47, 65, 84, 106, 127, 146, 164, 181, 199, -181, -161, -140, -121, -100, -81, -60, -39, -21, 2, 22, 45, 65, 84, 103, 124, 147, 167, 186, -198, -181, -163, -145, -126, -108, -91, -73, -54, -33, -14, 6, 25, 43, 62, 81, 100, 121, 138,155, 166, 177, 88, 110, 130, 151, 169, 185, 200, -190, -184, -183, -188, 199, 182, 170, 169, 178, 196, -182, -160, -141, -125, -111, -100, -96, -104, -127, -150, -158, -158, -152, -142, -128, -111, -91, -71, -52, -33, -16, -1, 11, 19, 21, 17, 5, -13, -24, -26, -18, 1, 24, 48, 68, 86, 100, 112, 117, 110, 88, 67, 59, 59, 64, 74, 88, 107, 128, 149, 168, 184, 200, -188, -179, -173, -174, -180, -196, 185, 173, 171, 180, 198, -180, -159, -140, -123, -107, -89, -71, -51, -28, 2, 47, 98, 131, 153, 170
};

typedef struct {
    // phase data
                int16_t    phase_data[NUM_SAMPLES];
                int16_t    reference_data[NUM_SAMPLES+NUM_SAMPLES_REFERENCE];


} app_vars_t;

app_vars_t app_vars;

// aoa
double calculate_aoa(void);
bool  variation_check(int16_t* data, uint8_t length);

int main() {
      uint16_t i;
      double angle;

      memcpy(app_vars.phase_data, &data[0], 2*NUM_SAMPLES);

      angle = calculate_aoa();

      printf("angle=%f",angle);
}


bool variation_check(int16_t* data, uint8_t length) {
    return true;
}

#define VALID_PHASE_DIFF_RANG 113   // calculated as (402 * ANT_DISTANCE / 0.12468) 
#define MAX_PHASE_DIFF        402   //

double calculate_phase_diff(uint8_t shift) {

    uint16_t i;
    int16_t diff;
    int32_t sum;
    double avg_phase_diff;
    uint8_t num_diff_sample;

    int16_t phase_one_ant[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
    int16_t reference_one_ant[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];
    int16_t phase_diff[(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4];

    for (i=0;i<3;i++) {
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&phase_one_ant[8*i], &app_vars.phase_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
    }
    for (i=0;i<3;i++) {
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
        memcpy(&reference_one_ant[8*i], &app_vars.reference_data[8*(8+shift+4*i)],sizeof(int16_t)*8);
    }

    num_diff_sample = 0;
    for (i=0;i<(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4;i++) {
        if (shift == 2) {
            diff = phase_one_ant[i] - reference_one_ant[i];
        } else {
            diff = reference_one_ant[i] - phase_one_ant[i];
        }

        //if (diff <= -201) {
        //    diff += 402;
        //} else {
        //    if (diff >= 201) {
        //        diff -= 402;
        //    }
        //}

        if ((diff > (0-VALID_PHASE_DIFF_RANG)) && (diff < VALID_PHASE_DIFF_RANG)) {
            phase_diff[num_diff_sample++] = diff;
        }
    }

    //---- DO NOT calculate angle if the phase diff variate too much
    if (
        variation_check(phase_diff, (NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4) == 0
    ) {
        return INVAILD_ANGLE;
    }

    printf("phase_diff ");

    sum = 0;
    for (i=0;i<num_diff_sample;i++) {
        printf("%d, ",phase_diff[i]);
        sum += phase_diff[i];
    }
    if (num_diff_sample>0) {
        avg_phase_diff = (double)(sum)/num_diff_sample;
    } else {
        avg_phase_diff = MAX_PHASE_DIFF;
    }

    printf("\r\n avg_phase_diff = %f\r\n",avg_phase_diff);

    return avg_phase_diff;
}

#define SPEED_OF_LIGHT          300000000
#define NUM_SAMPLES_SWITCH_SLOT 8
#define ANT_DISTANCE            0.035
#define CENTER_FREQ             (2404)

double calculate_aoa(void) {

    int16_t diff;
    int16_t sum;
    double avg_phase_diff_1;
    double avg_phase_diff_3;
    int16_t reference_data_temp[NUM_SAMPLES+NUM_SAMPLES_REFERENCE];
    
    uint8_t i;
    uint8_t wave_index_start, wave_index_end;
    int16_t step;
    int16_t estimated_phase;
    double   wave_length; // in meter
    uint32_t IF;
    uint32_t frequency;

    uint8_t num_reference_sample;

    double   angle;
    double   acos_x_1;
    double   acos_x_3;
    double   angle_1;
    double   angle_3;

    uint8_t shift;

    wave_index_start = 0;
    wave_index_end   = 0;
    num_reference_sample = 64;
    //==== find a complete wave of the signal
    for (i=0; i<num_reference_sample-1; i++) {
        if (app_vars.phase_data[i] - app_vars.phase_data[i+1] > 300) {
            if (wave_index_start == 0) {
                wave_index_start = i + 1;
            } else {
                wave_index_end   = i + 1;
                break;
            }
        }
    }

    frequency       = CENTER_FREQ;
    if (
        wave_index_start != 0 && 
        wave_index_end   != 0 &&
        wave_index_end - wave_index_start > 15 &&
        wave_index_end - wave_index_start < 49
    ) {
        IF          = 8000000/(wave_index_end-wave_index_start);
        frequency   = frequency*1000000 + IF;
        wave_length = (double)SPEED_OF_LIGHT / frequency;
        step        = app_vars.phase_data[wave_index_end-1] - app_vars.phase_data[wave_index_start];
        step        = step/(wave_index_end-wave_index_start-1);
    } else {
        return INVAILD_ANGLE;
    }

    //===== generate the target phase data when reference antenna is used through CTE)
    memcpy(reference_data_temp,     app_vars.phase_data, sizeof(int16_t)*wave_index_end);
    i = wave_index_end;
    while (i<(NUM_SAMPLES+NUM_SAMPLES_SWITCH_SLOT)){
        estimated_phase        = reference_data_temp[i-1] + step;
        if (estimated_phase > 201) {
            estimated_phase -= 403;
        }
        reference_data_temp[i] = estimated_phase;
        i++;
    }
    printf("step = %d wave_index_end=%d, wave_index_start=%d\r\n", step, wave_index_end, wave_index_start);
    printf("reference_data");
    for (i=0;i<num_reference_sample;i++) {
        printf("%d ",reference_data_temp[i]);
    }
    printf("\r\n");

    memcpy(
        app_vars.reference_data,
        reference_data_temp,
        sizeof(int16_t)*num_reference_sample
    );
    memcpy(
        &app_vars.reference_data[num_reference_sample],
        &reference_data_temp[num_reference_sample+NUM_SAMPLES_SWITCH_SLOT],
        sizeof(int16_t)*(NUM_SAMPLES-num_reference_sample)
    );
    
    //==== generate phase diff between ant_x.2 and ant_x.1; ant_x.3 and ant_x.2

    //---- generate phase diff between ant_x.2 and ant_x.1

    shift = 0;
    avg_phase_diff_1 = calculate_phase_diff(shift);

    //---- generate phase diff between ant_x.3 and ant_x.2

    shift = 2;
    avg_phase_diff_3 = calculate_phase_diff(shift);

    if (avg_phase_diff_1 == MAX_PHASE_DIFF || avg_phase_diff_3 == MAX_PHASE_DIFF) {
        return INVAILD_ANGLE;
    }

    //==== calculate the angle

    acos_x_1  = (avg_phase_diff_1/402.0) * wave_length / ANT_DISTANCE;
    if (acos_x_1 >= -1 && acos_x_1 <= 1) {
        angle_1  = acos(acos_x_1);
    } else {
        angle_1 = INVAILD_ANGLE;
    }

    acos_x_3  = (avg_phase_diff_3/402.0) * wave_length / ANT_DISTANCE;
    if (acos_x_3 >= -1 && acos_x_3 <= 1){
        angle_3  = acos(acos_x_3);
    } else {
        angle_3 = INVAILD_ANGLE;
    }

    if (angle_1 != INVAILD_ANGLE && angle_3 != INVAILD_ANGLE) {
        angle = 2*tan(angle_1)*tan(angle_3)/(tan(angle_1)+tan(angle_3));
        angle = 180 * atan(angle) / M_PI;
    } else {
        if (angle_1 != INVAILD_ANGLE && angle_3 == INVAILD_ANGLE) {
            angle = 180 * angle_1 / M_PI;
        } else {
            if (angle_1 == INVAILD_ANGLE && angle_3 != INVAILD_ANGLE) {
                angle = 180 * angle_3 / M_PI;
            } else {
                return INVAILD_ANGLE;
            }
        }
    }


    if (angle<0) {
        angle = 180.0 + angle;
    }

    return angle;
}
