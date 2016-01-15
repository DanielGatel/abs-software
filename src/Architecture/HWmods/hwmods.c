#include "hwmods.h"

#include <netinet/in.h>
#include <mcs.h>
#include <pthread.h>

int i2cbitrate;
pthread_mutex_t lock;

MODerror check_pkt ( MCSPacket *pkt ) {

    if(pkt == NULL) {
        return ME_PKT_RESP_READ;     //Could not read response packet
    } else if(pkt->type != MCS_TYPE_OK && pkt->type != MCS_TYPE_OK_DATA) {
        if(pkt->type == MCS_TYPE_ERR) {
            return ME_PKT_RESP_ERROR;
        } else {
            return ME_PKT_RESP_WRONG;
        }
    } else return ME_FREE;
}

int sdb_connect ( int fd, MCSPacket *pkt, int id ) {

    char welcome[] = "hwmod";
    welcome[3] = id + '0';

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons( SDB_SOCK_PORT );
    addr.sin_addr.s_addr = INADDR_ANY;

    /* SDB connection */
    fd = socket( AF_INET, SOCK_STREAM, 0 );

    if(fd < 0) {
        return ME_SOCKET_CRETE; //Could not create the socket
    }

    if(connect( fd, (struct sockaddr *)&addr, sizeof( addr ) ) < 0) {
        return ME_SOCKET_CONNECT; //Could not connect to the socket
    }

    /* Handshaking */
    /* Remember NULL character! */
    pkt = mcs_create_packet( MCS_MESSAGE_SDB_HANDSHAKE, 0, NULL, strlen( welcome ) + 1, (unsigned char *)welcome );

    if(pkt == NULL) {
        return ME_PKT_CREATE; //Packet generated is NULL
    }

    if(mcs_write_command( pkt, fd ) != 0) {
        return ME_PKT_SEND; //Could not send handshake packet
    }

    mcs_free( pkt );

    pkt = mcs_read_command( fd, fd );

    if(int tmp = check_pkt( pkt ) < 0) return tmp;
    else if(pkt->type == 1) return 0;
    else return -1;
}

int i2c_set ( int fd, MCSPacket *pkt, int br ) {

    if(&lock == NULL) pthread_mutex_init( &lock, NULL );

    pthread_mutex_lock( &lock );

    if(br != i2cbitrate) {
        mcs_free( pkt );
        unsigned char *args[2];
        *args[1] = br;
        pkt = mcs_create_packet( MCS_PAYLOAD_ARDUINO_INIT_I2C, 2, *args, 0, NULL );

        if(mcs_write_command( pkt, fd ) != 0) {
            return ME_PKT_SEND; //Error sending packet
        }

        i2cbitrate = br;

        mcs_free( pkt );
        pkt = mcs_read_command( fd, fd );

        if(int tmp = check_pkt( pkt ) < 0) return tmp;
        else if(pkt->type == 1) return 0;
        else return -1;

    } else return 0;
}

void i2c_leave ( ) {
    pthread_mutex_unlock( &lock );
}
/*
int main ( int argc, char const *argv[] ) {
    pthread_mutex_init( &lock, NULL );
    return 0;
}
*/
