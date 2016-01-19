#ifndef __EPS_HWMOD_H
#define __EPS_HWMOD_H

#include "hwmods.h"

#include <time.h>

/*EPS Settings*/
#define N_ACDC 6 //Number of ACDCs
#define N_POL 4 //Number of POLs
#define N_EPS 10 //Total
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
#define EPS_BITRATE 100 //I2C bitrate
static unsigned char eps_addr[N_EPS] = { 0x1, //@ACDC_T
                                         0x2, //@ACDC_B
                                         0x3, //@ACDC_1A
                                         0x4, //@ACDC_1B
                                         0x5, //@ACDC_2A
                                         0x6, //@POL_1
                                         0x7, //@POL_2
                                         0x8, //@POL_3
                                         0x9 };//@POL_4
/*EPS Settings - Pin enable*/
static unsigned char eps_pinen[N_EPS] = { 42, //Pin enable ACDC_T
                                         43, //Pin enable ACDC_B
                                         44, //Pin enable ACDC_1A
                                         45, //Pin enable ACDC_1B
                                         46, //Pin enable ACDC_2A
                                         6, //Pin enable POL_1
                                         7, //Pin enable POL_2
                                         8, //Pin enable POL_3
                                         9 };//Pin enable POL_4
/*EPS Settings - State*/
enum EPScstat {
    ERROR = -1, //Unknow state
    STOP = 0, //Checked
    ON = 1, //Powered on
    READY = 2, //Initialized
    RUNNING = 3, //Run
};
typedef struct EPSstate {
    EPScstat cstate; //Current state
    time_t t; //Time of checked
    MODerror errors; //Store errors
} EPSstate;

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
typedef struct ACDCconfig {//struct to define ACDC configuration
    ACDC_S_CH channel;// = SC1;
    ACDC_S_CM mode;// = CCM;
    ACDC_S_SRS rate;// = SPS240B12;
    ACDC_S_PGAGS gain;// = X1;
} ACDCconfig;
/*ACDC - Channels*/
enum ACDC_CH { //Define type of information of each channel
    ACDC_CH_VOLTAGE = SC1, //CH1
    ACDC_CH_INTENSITI = SC2, //CH2
    ACDC_CH_TEMPERATURE = SC3, //CH3
    ACDC_CH_IRRADIANCE = SC4 //CH4
};
/*ACDC - Data*/
typedef struct ACDCdata { //Store ACDC data
    EPSComponent ident; //Name of the component
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

/*Battery*/
/*Batery - Files*/
static char bchargefile[] = "/sys/class/power_supply/battery/state"; //File to change switch charge
static char bfile_status[] = "/sys/class/power_supply/battery/state";
static char bfile_present[] = "/sys/class/power_supply/battery/state";
static char bfile_voltage[] = "/sys/class/power_supply/battery/voltage_now";
static char bfile_temp[] = "/sys/class/power_supply/battery/state";
static char bfile_capacity[] = "/sys/class/power_supply/battery/state";
static char bfile_current[] = "/sys/class/power_supply/battery/current_now";
/*Battery - Data*/
typedef struct BATdata {
    time_t t;
    bool charging;
    int level;
    int voltage;
    int current;

} BATdata;

#endif
