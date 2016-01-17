#ifndef __EPS_HWMOD_H
#define __EPS_HWMOD_H

#include "hwmods.h"

#include <time.h>

#define N_ACDC 6
#define N_POL 4
#define N_EPS 10

/*EPS Settings*/
enum EPSComponent {
    ACDC_T = 0,
    ACDC_B = 1,
    ACDC_1A = 2,
    ACDC_1B = 3,
    ACDC_2A = 4,
    ACDC_2B = 5,
    POL_1 = 6,
    POL_2 = 7,
    POL_3 = 8,
    POL_4 = 9,
};
/*EPS Settings - @I2C*/
#define EPS_BITRATE 0
static unsigned char eps_addr[N_EPS] = { 0x1, //ACDC_T
                                         0x2, //ACDC_B
                                         0x3, //ACDC_1A
                                         0x4,
                                         0x5,
                                         0x6 };
/*EPS Settings - Pin ennable*/
static unsigned char eps_pinen[N_EPS] = { 1, //ACDC_T
                                          2, //ACDC_B
                                          3, //ACDC_1A
                                          4,
                                          5,
                                          6 };
/*EPS Settings - State*/
enum EPScstat {
    ERROR = -1,
    STOP = 0, //Checked
    ON = 1, //Powered on
    READY = 2, //Initialized
    RUNNING = 3, //Run
};
typedef struct EPSstate {
    EPScstat cstate;
    time_t t;
    MODerror errors;
} POLstatus;

/*Battery*/
/*Batery - Files*/
static char bfile[] = "/sys/class/power_supply/battery/state";
static char bfile_status[] = "/sys/class/power_supply/battery/state";
static char bfile_present[] = "/sys/class/power_supply/battery/state";
static char bfile_voltage_now[] = "/sys/class/power_supply/battery/voltage_now";
static char bfile_temp[] = "/sys/class/power_supply/battery/state";
static char bfile_capacity[] = "/sys/class/power_supply/battery/state";
static char bfile_current_now[] = "/sys/class/power_supply/battery/current_now";
/*Battery - Data*/
typedef struct BATdata {
    time_t t;
    bool charging;
    int level;
    int voltage;
    int current;

} BATdata;

/*ACDC*/
/*ACDC - Config*/
enum ACDC_S_CH { SC1 = 0x0, //Channel Selection
                 SC2 = 0x1,
                 SC3 = 0x2,
                 SC4 = 0x3 };
enum ACDC_S_CM { CCM = 0x0, //Conversion Mode
                 OSCM = 0x1 };
enum ACDC_S_SRS { SPS240B12 = 0x0, //Sample Rate Selection
                  SPS60B14 = 0x1,
                  SPS15B16 = 0x2,
                  SPS3_75B18 = 0x3 };
enum ACDC_S_PGAGS { X1 = 0x0, //PGA Gain Selection
                    X2 = 0x1,
                    X4 = 0x2,
                    X8 = 0x3 };
typedef struct ACDCconfig {
    ACDC_S_CH channel = SC1;
    ACDC_S_CM mode = CCM;
    ACDC_S_SRS rate = SPS240B12;
    ACDC_S_PGAGS gain = X1;
} ACDCconfig;
/*ACDC - Channels*/
enum ACDC_CH {
    ACDC_CH_VOLTAGE = SC1, //CH1
    ACDC_CH_INTENSITI = SC2, //CH2
    ACDC_CH_TEMPERATURE = SC3, //CH3
    ACDC_CH_IRRADIANCE = SC4 //CH4
};
/*ACDC - Data*/
typedef struct ACDCdata {
    EPSComponent ident;
    int voltage;
    char vo_gain;
    time_t vt;
    int intensiti;
    char in_gain;
    time_t it;
    int temperature;
    char te_gain;
    time_t tt;
    int irradiance;
    char ir_gain;
    time_t gt;
} ACDCdata;

/*POL*/
/*POL - Data*/
typedef struct POLdata {
    unsigned short ident;
    time_t t[N_EPS];

} POLdata;

#endif
