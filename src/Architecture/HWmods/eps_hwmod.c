#include "hwmods.h"
#include "eps_hwmod.h"

#include <time.h>
#include <netinet/in.h>
#include <mcs.h>

int epsfd;
MCSPacket *epspkt;

EPSstate state[N_EPS];

//Send a MCS packet with all EPSstate data
int sdb_send_state ( int fd, MCSPacket *pkt ) {
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
bool eps_on ( int fd, MCSPacket *pkt, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] = eps_pinen[comp];
    arg[1] = 1;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
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
            state[comp].cstate = ON;
            return true;
        }
    }

    return false;

}

//Turn off the component "comp"
bool eps_off ( int fd, MCSPacket *pkt, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] = eps_pinen[comp];
    arg[1] = 0;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
    state[comp].t = time( NULL );

    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );
        state[comp].t = time( NULL );

        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) return true;
    }

    return false;
}

//
void acdc_check ( int fd, MCSPacket *pkt ) {

    bool sw;

    for(size_t i = 0; i < N_ACDC; i++) {

        sw = false;

        if(state[i].cstate <= 1) {
            if(!( sw = eps_on( fd, pkt, (EPSComponent)i ) ) ) goto next;
        }

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[i], 0, NULL );
        state[i].t = time( NULL );

        if(pkt == NULL) {
            state[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            state[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );
            state[i].t = time( NULL );
            state[i].errors = check_pkt( pkt );
        }

next:

        if(sw) {
            eps_off( fd, pkt, (EPSComponent)i );
        }
    }

}
/**/
void pol_check ( int fd, MCSPacket *pkt ) {

    for(size_t i = N_ACDC + 1; i < N_ACDC; i++) {

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[i], 0, NULL );
        state[i].t = time( NULL );

        if(pkt == NULL) {
            state[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            state[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );
            state[i].t = time( NULL );
            state[i].errors = check_pkt( pkt );
        }
    }
}

void check ( ) {

    if(sdb_connect( epsfd, epspkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    //Check
    acdc_check( epsfd, epspkt );
    pol_check( epsfd, epspkt );

    //sdb_send_state( fd, pkt);

error: i2c_leave( );

}

void acdc_config_write ( ACDCconfig c, unsigned char *config ) {
    *config = 0x0;

    *config = (unsigned char)c.channel << 1;
    *config = ( *config << 2 ) + (unsigned char)c.gain;
    *config = ( *config << 1 ) + (unsigned char)c.mode;
    *config = ( *config << 2 ) + (unsigned char)c.rate;

}

void acdc_config ( int fd, MCSPacket *pkt, ACDCconfig c, EPSComponent comp ) {

    unsigned char *config;

    acdc_config_write( c, config );

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
 */         state[comp].cstate = READY;
        } else state[comp].cstate = ERROR;
    }
}

void acdc_init ( int fd, MCSPacket *pkt ) {

    size_t i;

    ACDCconfig configinic;

    for(i = 0; i < N_ACDC; i++) {

        //unsigned char *config;
        /*if(state[i].cstate == 0)*/ eps_on( fd, pkt, (EPSComponent)i );

        /*if(state[i].cstate == 1)*/ acdc_config( fd, pkt, configinic, (EPSComponent)i );

        /*acdc_config_write( SC1, CCM, SPS240B12, X1, config );

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

void pol_init ( int fd, MCSPacket *pkt ) {

    size_t i;

    for(i = 0; i < N_POL; i++) {

/*        unsigned char *config;

   *config = acdc_config_write( SC1 , CCM, SPS240B12, X1);

        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_WRITE_I2C, 1, &acdc_addr[i], 1, config );
        stat[i].t = time( NULL );
 */
        if(pkt == NULL) {
            state[i].errors = ME_PKT_CREATE; //Packet generated is NULL
        } else if(mcs_write_command( pkt, fd ) != 0) {
            state[i].errors = ME_PKT_SEND;
        } else {

            mcs_free( pkt );

            pkt = mcs_read_command( fd, fd );

            state[i].t = time( NULL );

            if( ( state[i].errors = check_pkt( pkt ) ) == 0) state[i].cstate = READY;
            else state[i].cstate = ERROR;
        }
    }

}

void init ( ) {

//    int fd;
//    MCSPacket *pkt;

//    EPSstate state[N_EPS];

    //if(sdb_connect( fd, pkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    acdc_init( epsfd, epspkt );
    pol_init( epsfd, epspkt );

error: i2c_leave( );
}

void bat_data_get ( BATdata *data ) {
    FILE *battery;

    battery = fopen( bfile, "r" );

    data->t = time( NULL );
    //

    fclose( battery );
}

void acdc_config_read(unsigned char cd, ACDCconfig *config){

    config->channel = (ACDC_S_CH)((cd&0x60)>>5);
    config->mode = (ACDC_S_CM)((cd&0x10)>>4);
    config->rate = (ACDC_S_SRS)((cd&0xC)>>2);
    config->gain = (ACDC_S_PGAGS)(cd&0x3);


}
void acdc_data_get (int fd, MCSPacket *pkt, EPSComponent comp, ACDCdata *data) {

    ACDCconfig *conf;

    if(state[comp].cstate != RUNNING) goto error;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[comp], 0, NULL );
    state[comp].t = time( NULL );

    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE; //Packet generated is NULL
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );

        data->t = state[comp].t = time( NULL );

        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) {

            unsigned char dat[pkt->data_size];

            for(size_t i = 1; i < pkt->data_size; i++) {
                dat[i] = *(&(*pkt->data) + i );
            }
            //conf.gain=NULL;
            acdc_config_read(dat[pkt->data_size], conf);
            char mask;
            switch(conf->gain) {
            case SPS240B12:
                mask = 0xF;
                break;
            case SPS60B14:
                mask = 0x3F;
                break;
            case SPS15B16:
                mask = 0xFF;
                break;
            case SPS3_75B18:
                mask = 0x3;
                break;
            }
        } else state[comp].cstate = ERROR;



    }
    error:;
}

void run ( ) {

    BATdata *data;
    ACDCdata *acdcdata;


    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    bat_data_get( data );
EPSComponent comp;
    acdc_data_get( epsfd, epspkt, comp , acdcdata);
//recibir interrupciones

//controlar l bateria y la carga
error:;
}

void acdc_halt ( int fd, MCSPacket *pkt) {

}
void pol_halt ( int fd, MCSPacket *pkt) {

}

void halt ( ) {

//    int fd;
//    MCSPacket *pkt;

//    EPSstate state[N_EPS];

//    if(sdb_connect( fd, pkt, EPS_HWMOD_ID ) < 0) goto error;

    //Check i2c
    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    acdc_halt( epsfd, epspkt);
    pol_halt( epsfd, epspkt);

error: i2c_leave( );

}


void osf ( ) {

}
