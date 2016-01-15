#ifndef __EPS_HWMOD_H
#define __EPS_HWMOD_H

#include "hwmods.h"

#include <time.h>

#define N_ACDC 6
#define N_POL 4
#define N_EPS 10

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
static char bfile[] = "/sys/class/power_supply/";
/*Settings*/
#define EPS_BITRATE 0
/*Settings - @I2C*/
static unsigned char eps_addr[N_EPS] = { 0x1, //ACDC_T
                                         0x2, //ACDC_B
                                         0x3, //ACDC_1A
                                         0x4,
                                         0x5,
                                         0x6 };
/*Settings - Pin ennable*/
static unsigned char eps_pinen[N_EPS] = { 1, //ACDC_T
                                          2, //ACDC_B
                                          3, //ACDC_1A
                                          4,
                                          5,
                                          6 };
/*Settings - ACDC*/
/*Settings - ACDC - @I2C*/
/*static unsigned char acdc_addr[N_ACDC] = { 0x1,
                                           0x2,
                                           0x3,
                                           0x4,
                                           0x5,
                                           0x6 };
*/
//typedef struct ACDCset {
/*Settings - ACDC - Config settings*/
enum ACDC_S_CH { SC1 = 0x0, //Channel Selection
                 SC2 = 0x1,
                 SC3 = 0x3,
                 SC4 = 0x4 };
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
    ACDC_S_CH channel;
    ACDC_S_CM mode;
    ACDC_S_SRS rate;
    ACDC_S_PGAGS gain;
} ACDCconfig;

//static unsigned char acdc_init = 0x1;

/*Settings - POL*/
/*Settings - POL - @I2C*/
static unsigned char pol_addr[N_POL] = { 0x7,
                                         0x8,
                                         0x9,
                                         0x10 };

/*Data*/
/*Data - ACDC*/
typedef struct ACDCdata {
    unsigned short ident;
    time_t t;
    int voltage;
    char vo_gain;
    int intensiti;
    char in_gain;
    int temperature;
    char te_gain;
    int irradiance;
    char ir_gain;
} ACDCdata;
/*Data - POL*/
typedef struct POLdata {
    unsigned short ident;
    time_t t[N_EPS];

} POLdata;
/*Data - Battery*/
typedef struct BATdata{
    time_t t;
} BATdata;

/*State*/
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

/*check*/
#define STEPS 2
/*Commands to send check*/
/*static const MCSCommand outcmd[STEPS] = { MCS_MESSAGE_SDB_HANDSHAKE,
                                          MCS_PAYLOAD_ARDUINO_INIT_I2C,
                                          //ACDC
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          //POL
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C,
                                          MCS_PAYLOAD_READ_I2C };
   static const unsigned short outnargs[STEPS] =   { 0,
                                                  2 };
   static const unsigned char outargs[STEPS] = { NULL,
                                              serialid,};
   static const unsigned char outdata[STEPS] = { (unsigned char*)welcome,
                                              NULL,
                                              //ACDC
                                              0x2 };
 */
/**/
//MCSPacket *inpkt[STEPS];

#endif
