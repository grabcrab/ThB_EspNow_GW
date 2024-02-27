#ifndef __ESP_PACKET_H__
#define __ESP_PACKET_H__

#include <Arduino.h>

#define PARAM_MAX_NUM       4
#define PARAM_NAME_LEN      16
#define PARAM_STRING_LEN    20

enum tParamType
{
    ptNone = 0,
    ptNumeric,
    ptString    
};

struct tEspParam
{
    tParamType  paramType = ptNone;
    char        paramName[PARAM_NAME_LEN];
    double      numParam;
    char        strParam[PARAM_STRING_LEN];
    void        setName(String pName);
    void        setString(String pName, String str);
    void        setNumeric(String pName, double val);
    void        print(int num = -1);
};

struct tEspPacket 
{   
    uint8_t     paramsCount = 0;     
    tEspParam   params[PARAM_MAX_NUM];
    bool        addString(String pName, String str);
    bool        addNumeric(String pName, double val);
    void        print(void);
    void        erase(void);
};

//int i = sizeof(tEspPacket);

#endif