#include "modbus.h"
#include "crc.h"

unsigned short get_cmd_length(modbReq* reqStruct)
{
    switch(reqStruct->cmdType)
    {
        case CMD_RD_HLD:return 8;
        case CMD_RD_INP:return 8;
        case CMD_RD_COIL:return 8;
        case CMD_RD_DIN:return 8;
    }
    return 0;
}

void fill_cmd_buf(modbReq* reqStruct)
{
    unsigned short tmp;
    switch(reqStruct->cmdType)
    {
        case CMD_RD_HLD:
            reqStruct->tx_ptr[0] = reqStruct->netAddress;
            reqStruct->tx_ptr[1] = 0x03;
            reqStruct->tx_ptr[2] = reqStruct->memAddress >> 8;
            reqStruct->tx_ptr[3] = reqStruct->memAddress & 0xFF;
            reqStruct->tx_ptr[4] = reqStruct->regCount >> 8;
            reqStruct->tx_ptr[5] = reqStruct->regCount & 0xFF;
            tmp = GetCRC16(reqStruct->tx_ptr,6);
            reqStruct->tx_ptr[6] = tmp >> 8;
            reqStruct->tx_ptr[7] = tmp & 0xFF;
        break;
        case CMD_RD_INP:
            reqStruct->tx_ptr[0] = reqStruct->netAddress;
            reqStruct->tx_ptr[1] = 0x04;
            reqStruct->tx_ptr[2] = reqStruct->memAddress >> 8;
            reqStruct->tx_ptr[3] = reqStruct->memAddress & 0xFF;
            reqStruct->tx_ptr[4] = reqStruct->regCount >> 8;
            reqStruct->tx_ptr[5] = reqStruct->regCount & 0xFF;
            tmp = GetCRC16(reqStruct->tx_ptr,6);
            reqStruct->tx_ptr[6] = tmp >> 8;
            reqStruct->tx_ptr[7] = tmp & 0xFF;
        break;
        case CMD_RD_COIL:
            reqStruct->tx_ptr[0] = reqStruct->netAddress;
            reqStruct->tx_ptr[1] = 0x01;
            reqStruct->tx_ptr[2] = reqStruct->memAddress >> 8;
            reqStruct->tx_ptr[3] = reqStruct->memAddress & 0xFF;
            reqStruct->tx_ptr[4] = reqStruct->regCount >> 8;
            reqStruct->tx_ptr[5] = reqStruct->regCount & 0xFF;
            tmp = GetCRC16(reqStruct->tx_ptr,6);
            reqStruct->tx_ptr[6] = tmp >> 8;
            reqStruct->tx_ptr[7] = tmp & 0xFF;
        break;
        case CMD_RD_DIN:
            reqStruct->tx_ptr[0] = reqStruct->netAddress;
            reqStruct->tx_ptr[1] = 0x02;
            reqStruct->tx_ptr[2] = reqStruct->memAddress >> 8;
            reqStruct->tx_ptr[3] = reqStruct->memAddress & 0xFF;
            reqStruct->tx_ptr[4] = reqStruct->regCount >> 8;
            reqStruct->tx_ptr[5] = reqStruct->regCount & 0xFF;
            tmp = GetCRC16(reqStruct->tx_ptr,6);
            reqStruct->tx_ptr[6] = tmp >> 8;
            reqStruct->tx_ptr[7] = tmp & 0xFF;
        break;
    }
}

unsigned char check_answer(modbReq* reqStruct, unsigned short byte_cnt)
{
    unsigned short err_flag = 0;
    unsigned short tmp;

    if(GetCRC16(reqStruct->rx_ptr,byte_cnt)) return 0;

    switch(reqStruct->cmdType)
    {
        case CMD_RD_HLD:
            if(reqStruct->rx_ptr[1] != 0x03) err_flag = 1;
            if(reqStruct->rx_ptr[2] != reqStruct->regCount*2) err_flag = 1;
            if(byte_cnt != 1 + 2 + reqStruct->regCount*2 + 2) err_flag = 1;
            break;
        case CMD_RD_INP:
            if(reqStruct->rx_ptr[1] != 0x04) err_flag = 1;
            if(reqStruct->rx_ptr[2] != reqStruct->regCount*2) err_flag = 1;
            if(byte_cnt != 1 + 2 + reqStruct->regCount*2 + 2) err_flag = 1;
            break;
        case CMD_RD_COIL:
            // amount of bytes with coils' status
            tmp = reqStruct->regCount / 8;
            if(reqStruct->regCount % 8) tmp++;
            if(reqStruct->rx_ptr[1] != 0x01) err_flag = 1;
            if(reqStruct->rx_ptr[2] != tmp) err_flag = 1;
            if(byte_cnt != 1 + 2 + tmp*2 + 2) err_flag = 1;
            break;
        case CMD_RD_DIN:
            // amount of bytes with discrete inputs' status
            tmp = reqStruct->regCount / 8;
            if(reqStruct->regCount % 8) tmp++;
            if(reqStruct->rx_ptr[1] != 0x02) err_flag = 1;
            if(reqStruct->rx_ptr[2] != tmp) err_flag = 1;
            if(byte_cnt != 1 + 2 + tmp*2 + 2) err_flag = 1;
            break;
        default: err_flag = 1; break;
    }
    if(err_flag) return 0;else return 1;
}
