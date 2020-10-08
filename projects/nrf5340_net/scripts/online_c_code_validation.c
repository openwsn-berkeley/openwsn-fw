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

int16_t data[] = {
-139, -125, -114, -100, -87, -75, -62, -49, -36, -25, -12, 1, 15, 29, 43, 56, 68, 81, 93, 106, 119, 132, 144, 156, 168, 180, 194, -193, -181, -168, -156, -144, -132, -119, -106, -94, -81, -70, -56, -43, -30, -17, -6, 7, 20, 31, 44, 56, 69, 80, 94, 106, 120, 131, 146, 158, 171, 185, 199, -189, -179, -173, -174, -188, 166, 180, 195, -191, -179, -168, -159, -152, -145, -137, -124, -100, -32, 24, 45, 57, 67, 78, 92, 110, 124, 134, 136, 135, 131, 129, 129, 131, 136, 142, 151, 162, 174, 187, -199, -185, -173, -162, -153, -145, -137, -129, -117, -93, -26, 32, 52, 63, 72, 81, 94, 111, 126, 137, 139, 137, 134, 131, 131, 133, 136, 143, 150, 160, 173, 186, 200, -186, -174, -163, -154, -144, -136, -128, -115, -90, -25, 32, 53, 65, 75, 86, 102, 119, 136, 147, 154, 159, 161, 166, -138, -51, -44, -38, -32, -24
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

double calculate_phase_diff(uint8_t shift) {

    uint16_t i;
    int16_t diff;
    int32_t sum;
    double avg_phase_diff;

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
    for (i=0;i<(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4;i++) {
        if (shift == 2) {
            diff = phase_one_ant[i] - reference_one_ant[i];
        } else {
            diff = reference_one_ant[i] - phase_one_ant[i];
        }
        printf("diff=%d ",diff);
        if (diff <= -201) {
            diff += 402;
        } else {
            if (diff >= 201) {
                diff -= 402;
            }
        }
        phase_diff[i] = diff;
    }

    printf("%d, %d",reference_one_ant[2], phase_one_ant[2]);

    //---- DO NOT calculate angle if the phase diff variate too much
    if (
        variation_check(phase_diff, (NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4) == 0
    ) {
        return INVAILD_ANGLE;
    }

    sum = 0;
    for (i=0;i<(NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4;i++) {
        printf("%d, ",phase_diff[i]);
        sum += phase_diff[i];
    }
    avg_phase_diff = (double)(sum)/((NUM_SAMPLES-NUM_SAMPLES_REFERENCE)/4);

    printf("\r\n avg_phase_diff = %f\r\n",avg_phase_diff);

    return avg_phase_diff;
}

#define SPEED_OF_LIGHT          300000000
#define NUM_SAMPLES_SWITCH_SLOT 8
#define ANT_DISTANCE            0.035

double calculate_aoa(void) {

    int16_t diff;
    int16_t sum;
    double avg_phase_diff_1;
    double avg_phase_diff_3;
    int16_t reference_data_temp[NUM_SAMPLES+NUM_SAMPLES_REFERENCE];
    
    uint8_t i;
    uint8_t wave_index_start, wave_index_end;
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

    frequency       = 2404;
    if (
        wave_index_start != 0 && 
        wave_index_end   != 0 &&
        wave_index_end - wave_index_start > 15 &&
        wave_index_end - wave_index_start < 49
    ) {
        IF          = 8000000/(wave_index_end-wave_index_start);
        frequency   = frequency*1000000 + IF;
        wave_length = (double)SPEED_OF_LIGHT / frequency;
    } else {
        return INVAILD_ANGLE;
    }

    //===== generate the target phase data when reference antenna is used through CTE)
    memcpy(reference_data_temp,     app_vars.phase_data, sizeof(int16_t)*wave_index_end);
    i = wave_index_end;
    while (i<(NUM_SAMPLES+NUM_SAMPLES_SWITCH_SLOT)){
        memcpy(
            &reference_data_temp[i],
            &app_vars.phase_data[wave_index_start],
            sizeof(int16_t)*(wave_index_end-wave_index_start)
        );
        i += (wave_index_end-wave_index_start);
    }

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
