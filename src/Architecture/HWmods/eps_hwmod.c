#include "hwmods.h"
#include "eps_hwmod.h"

#include <time.h>
#include <netinet/in.h>
#include <mcs.h>
#include <unistd.h>

int epsfd; //file descriptor (socket)
MCSPacket *epspkt; //paket to send

EPSstate state[N_EPS]; //Store all HW states
bool running = true; //control run exit
int rundelay = 1000000; //delay run us

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

//Turn on the component "comp"
bool eps_on ( int fd, MCSPacket *pkt, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] = eps_pinen[comp]; //arg0 = comp pin ennable
    arg[1] = 1; //arg1 = HIGH
    //Send pin HIGH
    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
    //Store time of status change
    state[comp].t = time( NULL );
    //Check intent to send order
    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE;
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {
        mcs_free( pkt );
        //Recive request
        pkt = mcs_read_command( fd, fd );
        //Store time of status change
        state[comp].t = time( NULL );
        //Check and save response
        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) {
            //Change state info to ON
            state[comp].cstate = ON;
            //return success
            return true;
        }
    }
    //Return fail
    return false;

}

//Turn off the component "comp"
bool eps_off ( int fd, MCSPacket *pkt, EPSComponent comp ) {

    unsigned char arg[2];
    arg[0] = eps_pinen[comp]; //arg0 = comp pin ennable
    arg[1] = 0; //LOW
    //Create pkt write pin high
    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_DIGITAL_WRITE, 2, &arg[0], 0, NULL );
    //save time change
    state[comp].t = time( NULL );
    //check and send pkt
    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE;
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {
        mcs_free( pkt );
        //read request
        pkt = mcs_read_command( fd, fd );
        //save time change
        state[comp].t = time( NULL );
        //check request
        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) return true;
    }

    return false;
}

//ACDC check prtocol
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

//POL check protocol
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

void acdc_config_set ( int fd, MCSPacket *pkt, ACDCconfig c, EPSComponent comp ) {

    unsigned char *config = NULL;

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

        eps_on( fd, pkt, (EPSComponent)i );

        acdc_config_set( fd, pkt, configinic, (EPSComponent)i );

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

    //Check i2c
    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    acdc_init( epsfd, epspkt );
    pol_init( epsfd, epspkt );

error: i2c_leave( );
}

void bat_data_get ( BATdata *data ) {
    FILE *f;

    if((f = fopen( bfile, "r" ))!= NULL){
        data->t = time( NULL );
        //fread()
    }
    fclose( f );
}

bool bat_charg_set(){
    FILE *f;

    if((f = fopen( bfile, "w" ))!= NULL){

    }
    fclose( f );

}

void acdc_config_read ( unsigned char cd, ACDCconfig *config ) {

    config->channel = (ACDC_S_CH)( ( cd & 0x60 ) >> 5 );
    config->mode = (ACDC_S_CM)( ( cd & 0x10 ) >> 4 );
    config->rate = (ACDC_S_SRS)( ( cd & 0xC ) >> 2 );
    config->gain = (ACDC_S_PGAGS)( cd & 0x3 );

}

void acdc_data_read(unsigned char vd[], ACDCconfig c, ACDCdata *d){
    int value;
    switch(c.gain) {
    case SPS240B12:
        value = ((vd[0]&0xF)<<8) + vd[1];
        break;
    case SPS60B14:
        value = ((vd[0]&0x3F)<<8) + vd[1];
        break;
    case SPS15B16:
        value = (vd[0]<<8) + vd[1];
        break;
    case SPS3_75B18:
        value = ((vd[0]&0x3)<<16) + (vd[1]<<8) + vd[2];
        break;
    }
    switch((int)c.channel){
        case ACDC_CH_VOLTAGE:
        d->voltage=value;
        d->vo_gain=c.gain;
        break;
        case ACDC_CH_INTENSITI:
        d->intensiti=value;
        d->in_gain=c.gain;
        break;
        case ACDC_CH_TEMPERATURE:
        d->temperature=value;
        d->te_gain = c.gain;
        break;
        case ACDC_CH_IRRADIANCE:
        d->irradiance = value;
        d->ir_gain = c.gain;
        break;
    }
}

void acdc_data_get ( int fd, MCSPacket *pkt, EPSComponent comp, ACDCdata *data ) {

    ACDCconfig *conf = NULL;

    if(state[comp].cstate != RUNNING) goto error;

    pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_READ_I2C, 1, &eps_addr[comp], 0, NULL );
    state[comp].t = time( NULL );

    if(pkt == NULL) {
        state[comp].errors = ME_PKT_CREATE;
    } else if(mcs_write_command( pkt, fd ) != 0) {
        state[comp].errors = ME_PKT_SEND;
    } else {

        mcs_free( pkt );

        pkt = mcs_read_command( fd, fd );

        state[comp].t = time( NULL );

        if( ( state[comp].errors = check_pkt( pkt ) ) == 0) {

            unsigned char dat[pkt->data_size];
            for(size_t i = 0; i < pkt->data_size; i++) {
                dat[i] = *( &( *pkt->data ) + i );
            }

            //conf.gain=NULL;
            acdc_config_read( dat[pkt->data_size], conf);
            acdc_data_read( dat, *conf, data);
            data->ident=comp;

        } else state[comp].cstate = ERROR;

    }

error:;
}

int eps_control(BATdata bd, ACDCdata ad){

    if(bd.charging){
        //if(bd.level)
    }else{

    }
}

void run ( ) {

    while(running){
    BATdata *batteryd = NULL;
    ACDCdata *acdcd[N_ACDC];

    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    bat_data_get( batteryd );

    for (size_t i = 0; i < N_ACDC; i++) {
        acdc_data_get( epsfd, epspkt, (EPSComponent)i , acdcd[i] );
    }
    i2c_leave();
    usleep(rundelay);
    }
    running = true;
    error: i2c_leave();
}

void acdc_halt ( int fd, MCSPacket *pkt ) {

    for(size_t i = 0; i < N_ACDC; i++) {

        eps_off( fd, pkt, (EPSComponent)i );

    }

}
void pol_halt ( int fd, MCSPacket *pkt ) {

}

void halt ( ) {

    running = false;
    //Check i2c
    if(i2c_set( epsfd, epspkt, EPS_BITRATE ) < 0) goto error;

    acdc_halt( epsfd, epspkt );
    pol_halt( epsfd, epspkt );

error: i2c_leave( );

}

void osf_config_set ( ) {



}
