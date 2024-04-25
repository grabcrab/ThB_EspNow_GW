#ifndef __ESP_PACKET_H__
#define __ESP_PACKET_H__

#include <Arduino.h>

#define PARAM_MAX_NUM       4
#define PARAM_NAME_LEN      16
#define PARAM_STRING_LEN    20

#define PARAM_NO_VAL_16     (0xffff)
#define PARAM_NO_VAL_32     (0xffffffff)

enum tParamType
{
    ptNone = 0,
    ptNumeric,
    ptString    
};

struct __attribute__((packed)) tEspParam
{       
    tParamType  paramType = ptNone;
    char        paramName[PARAM_NAME_LEN];
    float       numParam;
    char        strParam[PARAM_STRING_LEN];
    void        setName(String pName);
    void        setString(String pName, String str);
    void        setNumeric(String pName, double val);
    void        print(int num = -1);
};

struct __attribute__((packed)) tEspPacket 
{   
    uint32_t        espProtocolID   = ESP_PROTOCOL_ID;
    uint8_t         paramsCount     = 0;     
    uint16_t        packetID        = 0;
    unsigned long   ms              = PARAM_NO_VAL_32;
    uint16_t        batVCC          = PARAM_NO_VAL_16;
    float           tempC           = PARAM_NO_VAL_16;
    time_t          timestamp       = PARAM_NO_VAL_32;
    tEspParam       params[PARAM_MAX_NUM];
    bool        addString(String pName, String str);
    bool        addNumeric(String pName, double val);
    void        print(void);
    void        erase(void);
};

//int i = sizeof(tEspPacket);
// int j = sizeof(tEspParam);

#endif

