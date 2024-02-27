#include "espPacket.h"

void tEspParam::setName(String pName)
{
    const byte maxLen = PARAM_NAME_LEN - 1;
    byte  nLen = pName.length();
    if (nLen > maxLen)
    {
        Serial.printf("tEspParam::setName: WARNING! Param name <%s> is too long (%d)\r\n", pName.c_str(), nLen);
        nLen = maxLen;        
    }
    memset(paramName, 0, PARAM_NAME_LEN);
    memcpy(paramName, pName.c_str(), nLen);
}

void tEspParam::setString(String pName, String str)
{
    const byte maxLen = PARAM_STRING_LEN - 1;
    byte  pLen = str.length();
    paramType = ptString;
    setName(pName);
    if (pLen > maxLen)
    {
        Serial.printf("tEspParam::setString: WARNING! Param text <%s> is too long (%d)\r\n", str.c_str(), pLen);
        pLen = maxLen;        
    }
    memset(strParam, 0, PARAM_NAME_LEN);
    memcpy(strParam, str.c_str(), pLen);
}

void tEspParam::setNumeric(String pName, double val)
{
    paramType = ptNumeric;
    setName(pName);
    numParam = val;
}

void tEspParam::print(int num)
{
    String prnStr = "\t";
    
    if (num >= 0)
        prnStr += (String("###") + num);
    
    prnStr += (String(paramName) + " = ");

    if (paramType == ptNumeric)
        prnStr += String(numParam);
    
    if (paramType == ptString)
        prnStr += String(strParam);

    if (paramType == ptNone)
        prnStr += "N/A";
    
    Serial.println(prnStr);
}

bool tEspPacket::addString(String pName, String str)
{
    if (paramsCount >= PARAM_MAX_NUM)
    {
        Serial.println("tEspPacket::addString: ERROR!!! Too many parameters!!!");
        return false;
    }
    params[paramsCount].setString(pName, str);
    paramsCount++;
    return true;
}

bool tEspPacket::addNumeric(String pName, double val)
{
    if (paramsCount >= PARAM_MAX_NUM)
    {
        Serial.println("tEspPacket::addNumeric: ERROR!!! Too many parameters!!!");
        return false;
    }
    params[paramsCount].setNumeric(pName, val);
    paramsCount++;
    return true;
}

void  tEspPacket::print(void)
{
    Serial.printf("ESP packet, %d parameters:\r\n", paramsCount);
    for (auto i = 0; i < paramsCount; i++)
        params[i].print();
}

void  tEspPacket::erase(void)
{
    memset(this, 0, sizeof(tEspPacket));
}