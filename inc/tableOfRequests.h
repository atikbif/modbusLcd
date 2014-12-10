#ifndef TABLEOFREQUESTS_H_INCLUDED
#define TABLEOFREQUESTS_H_INCLUDED

    #define CMD_RD_HLD  1
    #define CMD_RD_INP  2
    #define CMD_RD_COIL 3
    #define CMD_RD_DIN  4

    // round of mdbus requests

    typedef struct
    {
        unsigned char netAddress;
        unsigned short memAddress;
        unsigned short regCount;
        unsigned char cmdType;
        unsigned char* tx_ptr;
        unsigned char* rx_ptr;
    }modbReq;
    modbReq* getRequest(unsigned short num);



#endif /* TABLEOFREQUESTS_H_INCLUDED */
