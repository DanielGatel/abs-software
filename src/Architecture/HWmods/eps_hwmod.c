#include "hwmods.h"
#include "eps_hwmod.h"

#include <time.h>
#include <netinet/in.h>
#include <mcs.h>

int fd;
MCSPacket *pkt;

EPSstate state[N_EPS];

//Send a MCS packet with all EPSstate data
int sdb_send_state( int fd, MCSPacket *pkt){
/*
    pkt = mcs_create_packet(MCS_EPS_CHECK_DATA, 0, NULL, N_EPS*sizeof(MCSPacket), (unsigned char *)stat);

    if(pkt == NULL) {
        return -1; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        return -2;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );

        return check_pkt( pkt );
    }
*/
return -1;
}

//Turn on the componen "comp"
bool eps_on ( int fd, MCSPacket *pkt, EPSstate *stat, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] =  eps_pinen[comp];
    arg[1] = 1;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
    stat[comp].t = time( NULL );

    if(pkt == NULL) {
        stat[comp].errors = ME_PKT_CREATE; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        stat[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );
        stat[comp].t = time( NULL );

        if( ( stat[comp].errors = check_pkt( pkt ) ) == 0){
            stat[comp].cstate = ON;
            return true;
        }
    }

    return false;

}

//Turn off the component "comp"
bool eps_off ( int fd, MCSPacket *pkt, EPSstate *stat, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] =  eps_pinen[comp];
    arg[1] = 0;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
    stat[comp].t = time( NULL );

    if(pkt == NULL) {
        stat[comp].errors = ME_PKT_CREATE; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        stat[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );
        stat[comp].t = time( NULL );

        if( ( stat[comp].errors = check_pkt( pkt ) ) == 0) return true;
    }

    return false;
}

//
void acdc_check ( int fd, MCSPacket *pkt, EPSstate *stat ) {

    bool sw;

    for(size_t i = 0; i < N_ACDC; i++) {

        sw = false;

        if(stat[i].cstate <= 1) {
            if(!( sw = eps_on( fd, pkt, stat, (EPSComponent)i ) ) ) goto next;
        }

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[i], 0, NULL );
        stat[i].t = time( NULL );

        if(pkt == NULL) {
            stat[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            stat[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );
            stat[i].t = time( NULL );
            stat[i].errors = check_pkt( pkt );
        }

next:        if(sw) {
            eps_off( fd, pkt, stat, (EPSComponent)i );
        }
    }

}
/**/
void pol_check ( int fd, MCSPacket *pkt, EPSstate *stat ) {

    for(size_t i = N_ACDC + 1; i < N_ACDC; i++) {

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[i], 0, NULL );
        stat[i].t = time( NULL );

        if(pkt == NULL) {
            stat[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            stat[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );
            stat[i].t = time( NULL );
            stat[i].errors = check_pkt( pkt );
        }
    }
}

void check ( ) {

    //int fd;
    //MCSPacket *pkt;

    //EPSstate state[N_EPS];

    if(sdb_connect( fd, pkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( fd, pkt, EPS_BITRATE ) < 0) goto error;

    //Check
    acdc_check( fd, pkt, state );
    pol_check( fd, pkt, state );

    //sdb_send_state( fd, pkt, state);

error: i2c_leave( );

}

void acdc_config_create ( ACDCconfig c, unsigned char *config ) {
    *config = 0x0;

    *config = (unsigned char)c.channel << 1;
    *config = ( *config << 2 ) + (unsigned char)c.gain;
    *config = ( *config << 1 ) + (unsigned char)c.mode;
    *config = ( *config << 2 ) + (unsigned char)c.rate;

}

void acdc_config(ACDCconfig c, EPSComponent comp){

    unsigned char *config;

    acdc_config_create( c, config );

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_WRITE_I2C, 1, &eps_addr[comp], 1, config );
    state[comp].t = time( NULL );

    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );

        state[comp].t = time( NULL );

        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) {
/*                for (size_t i = 0; i < pkt->data_size; i++) {
*(pkt->data+i);
            }
*/             state[comp].cstate = READY;
        } else state[comp].cstate = ERROR;
    }
}

void acdc_init ( int fd, MCSPacket *pkt, EPSstate *stat ) {

    size_t i;

    for(i = 0; i < N_ACDC; i++) {

        //unsigned char *config;
if(state[i]== 0 ) eps_on(fd, pkt, comp)
        if(state[i].cstate == 1 )acdc_config(confinic, i);
        /*acdc_config_create( SC1, CCM, SPS240B12, X1, config );

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_WRITE_I2C, 1, &acdc_addr[i], 1, config );
        stat[i].t = time( NULL );

        if(pkt == NULL) {
            stat[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            stat[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );

            stat[i].t = time( NULL );

            if( ( stat[i].errors = check_pkt( pkt ) ) == 0) {
             stat[i].cstate = READY;
            } else stat[i].cstate = ERROR;
        }*/
    }

}

void pol_init ( int fd, MCSPacket *pkt, EPSstate *stat ) {

    size_t i;

    for(i = 0; i < N_POL; i++) {

/*        unsigned char *config;

   *config = acdc_config_create( SC1 , CCM, SPS240B12, X1);

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_WRITE_I2C, 1, &acdc_addr[i], 1, config );
        stat[i].t = time( NULL );
 */
        if(pkt == NULL) {
            stat[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            stat[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );

            stat[i].t = time( NULL );

            if( ( stat[i].errors = check_pkt( pkt ) ) == 0) stat[i].cstate = READY;
            else stat[i].cstate = ERROR;
        }
    }

}

void init ( ) {

//    int fd;
//    MCSPacket *pkt;

//    EPSstate state[N_EPS];

    //if(sdb_connect( fd, pkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( fd, pkt, EPS_BITRATE ) < 0) goto error;

    acdc_init( fd, pkt, state );
    pol_init( fd, pkt, state );

error: i2c_leave( );
}

void bat_read_data (BATdata data ) {
    FILE *battery;

    battery = fopen( bfile, "r" );

    data.t = time(NULL);
    //

    fclose( battery );
}

void eps_read_data(){


}

void run ( ) {

BATdata data;
    if(i2c_set( fd, pkt, EPS_BITRATE ) < 0) goto error;

    bat_read_data(data);

    acdc_read_data( fd, pkt, data);
//recibir interrupciones


//controlar l bateria y la carga
error:;
}

void acdc_halt ( int fd, MCSPacket *pkt, EPSstate *stat ) {

}
void pol_halt ( int fd, MCSPacket *pkt, EPSstate *stat ) {

}

void halt ( ) {

//    int fd;
//    MCSPacket *pkt;

//    EPSstate state[N_EPS];

//    if(sdb_connect( fd, pkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( fd, pkt, EPS_BITRATE ) < 0) goto error;

    acdc_halt( fd, pkt, state );
    pol_halt( fd, pkt, state );

error: i2c_leave( );

}

void eps_get_data(){


}

void osf ( ) {



}
