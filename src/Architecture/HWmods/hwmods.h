#ifndef __HWMODS_H
#define __HWMODS_H

#include <mcs.h>

void check ( );
void init ( );
void run ( );
void check ( );

#define EPS_HWMOD_ID 0
#define COM_HWMOD_ID 1

enum MODerror {
    ME_FREE = 0,
    /*pkt*/
    ME_PKT_CREATE = -11,////Packet generated is NULL
    ME_PKT_SEND = -12, //Error sending packet
    ME_PKT_RESP_READ = -13, //Could not read response packet
    ME_PKT_RESP_ERROR = -14, //Wrong response packet. Error
    ME_PKT_RESP_WRONG = -15,  //Wrong reponse packet.
    /*Socket*/
    ME_SOCKET_CRETE = -21, //Could not create the socket
    ME_SOCKET_CONNECT = -22,//Could not connect to the socket

};

MODerror check_pkt(MCSPacket *pkt);
int sdb_connect ( int fd, MCSPacket *pkt, int id );
int i2c_set ( int fd, MCSPacket *pkt, int br );
void i2c_leave ( );

#endif
