#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED

#include "tableOfRequests.h"

// form bufers of modbus requests and check answers

void fill_cmd_buf(modbReq* reqStruct);
unsigned short get_cmd_length(modbReq* reqStruct);
unsigned char check_answer(modbReq* reqStruct, unsigned short byte_cnt);

#endif /* MODBUS_H_INCLUDED */
