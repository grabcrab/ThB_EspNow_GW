#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "espRadio.h"
#include "tcu_board.h"
#include "utils.h"

#define US_FACTOR 1000000
#define SEND_VCC_AFTER_BOOTS    10

tEspPacket rData;
RTC_DATA_ATTR int vccMv = 0;
RTC_DATA_ATTR int vccCntr = SEND_VCC_AFTER_BOOTS;

int getVcc(void)
{
    if (vccCntr < SEND_VCC_AFTER_BOOTS)
    {
        vccCntr++;        
    }
    else 
    {
        vccCntr = 0;
        vccMv = boardGetBatteryVoltageMV();
    }
    return vccMv;
}

void sendEspPacket(void)
{   
    float tempC = tsReadDallas();
    float batVmv = getVcc();
    String vccParamName = String(SENDER_NAME) + "_VCC";
    String vccTempName = String(SENDER_NAME) + "_Temp";    
    rData.erase();
    rData.addNumeric(vccParamName, batVmv);
    rData.addNumeric(vccTempName, tempC);        
    setCpuFrequencyMhz(240);
    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);        
    sendPacket(&rData);        
}


void setup()
{        
    Serial.begin(115200);    
    boardInit();
    boardLedOn();    
    Serial.println(">>>BOOT");    
   
    sendEspPacket();
    Serial.flush();
    Serial.printf("Sent in %lu ms\r\n", millis());
    boardShutdown();
    ESP.deepSleep(SENDER_INTERVAL_S * US_FACTOR);
}

void loop()
{
    Serial.println(millis());
    delay(1000);
}
