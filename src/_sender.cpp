#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

#include "espRadio.h"
#include "tcu_board.h"
#include "utils.h"
#include "prefs/prefs.h"

#define US_FACTOR 1000000
#define SEND_VCC_AFTER_BOOTS    10

static tEspPacket rData;
RTC_DATA_ATTR int vccMv = 0;
RTC_DATA_ATTR int vccCntr = SEND_VCC_AFTER_BOOTS;
RTC_DATA_ATTR uint32_t bootCount = 0;
RTC_DATA_ATTR uint32_t tempErrCount = 0;

int getVcc(void)
{
    unsigned long ms = millis();
    if (vccCntr < SEND_VCC_AFTER_BOOTS)
    {
        vccCntr++;        
    }
    else 
    {
        vccCntr = 0;
        vccMv = boardGetBatteryVoltageMV();
    }
    Serial.printf(">>>getVcc completed in %d ms\r\n", millis() - ms);
    return vccMv;
}

float getTemp(void)
{
    unsigned long ms = millis();
    float res = tsReadDallas(tempErrCount);
    Serial.printf(">>>getTemp completed in %d ms\r\n", millis() - ms);
    return res;
}

void sendEspPacket(void)
{   
    float tempC = getTemp();
    float batVmv = getVcc();  
    unsigned long ms = millis();
    rData.erase();    
    rData.addString("deviceName", appPrefs.espSenderName);
    //rData.addNumeric("bootCount", bootCount);
    rData.addNumeric("tempErrCount", tempErrCount);
    // WiFi.mode(WIFI_AP_STA);
    // esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);        
    sendPacket(&rData, batVmv, tempC);       
    Serial.printf(">>>sendEspPacket completed in %d ms\r\n", millis() - ms);     
}


void senderSetup(bool firstBoot)
{        
    Serial.begin(115200);    
    boardInit();
    boardLedOn();    
    if (firstBoot)
    {
        Serial.println("senderSetup >>> FIRST BOOT");
        delay(2000);
    }
    Serial.println(">>>BOOT");        
   
    sendEspPacket();
    Serial.flush();
    Serial.printf("Sent in %lu ms.\tbootCount = %lu\ttempErrCount = %lu\r\n", millis(), bootCount, tempErrCount);
    boardShutdown();
    ESP.deepSleep(appPrefs.espSenderIntervalS * US_FACTOR);
}

void senderLoop()
{
    Serial.println(millis());
    delay(1000);
}
