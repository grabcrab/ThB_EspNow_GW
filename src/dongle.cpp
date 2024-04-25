#ifdef DONGLE_RECEIVER
#include <Arduino.h>
#include <Arduino_MQTT_Client.h>
#include <esp_wifi.h>

#include "espRadio.h"
#include "utils.h"

#include "s3_dongle/pin_config.h"
#include "TFT_eSPI.h" 
#include <FastLED.h> // https://github.com/FastLED/FastLED


TFT_eSPI tft = TFT_eSPI();
CRGB leds;

#define NO_TEMP -400

#define TOP_NAME "Balcony2"
#define TOP_RED_C       -10
#define TOP_YELLOW_C    -15

#define BOT_NAME "Balcony3"
#define BOT_RED_C       7
#define BOT_YELLOW_C    5

#define TEMP_LIFE_TIME_MS   15*60000 //!!!

#define HISTORY_SIZE    100

float topHistory[HISTORY_SIZE] = {NO_TEMP};
float botHistory[HISTORY_SIZE] = {NO_TEMP};

unsigned long lastPressedMs = 0;
bool tftActive = true;
unsigned long lastScreenUpdatedMs = 0;
float topTemp = NO_TEMP;
unsigned long lastTopRcvd = 0;
float topV = 0;

float botTemp = NO_TEMP;
unsigned long lastBotRcvd = 0;
float botV = 0;

bool ledStatus = true;
uint8_t rOn = 10; uint8_t gOn = 10; uint8_t bOn = 10;
uint8_t rOff = 0; uint8_t gOff = 1; uint8_t bOff = 0;

uint32_t topFontColor = TFT_GREEN;
uint32_t botFontColor = TFT_GREEN;

void addToHistory(float historyArr[HISTORY_SIZE], float temp)
{
    for (int i = 1; i < HISTORY_SIZE; i++)
    {
        historyArr[i-1] = historyArr[i];
    }
    historyArr[HISTORY_SIZE - 1] = temp;
}

void addToTopHistory(float temp)
{
    addToHistory(topHistory, temp);
}

void addToBotHistory(float temp)
{
    addToHistory(botHistory, temp);
}

void findInHistory(float historyArr[HISTORY_SIZE], float &minTemp, float &maxTemp)
{
    minTemp = NO_TEMP;
    maxTemp = NO_TEMP;

    for (int i = 0; i < HISTORY_SIZE; i++)
    {
        if (historyArr[i] == NO_TEMP)
            continue;

        if (minTemp == NO_TEMP)
            minTemp = historyArr[i];
        else
        {
            if (minTemp >  historyArr[i])
                minTemp = historyArr[i];
        }
        
        if (maxTemp == NO_TEMP)
            maxTemp = historyArr[i];
        else
        {
            if (maxTemp <  historyArr[i])
                maxTemp = historyArr[i];
        }
        
    }
}

void resetHistory(void)
{
    for (int i = 0; i < HISTORY_SIZE; i++)    
        topHistory[i] = botHistory[i] = NO_TEMP;

}

void findInTopHistory(float &minTemp, float &maxTemp)
{
    findInHistory(topHistory, minTemp, maxTemp);
}

void findInBotHistory(float &minTemp, float &maxTemp)
{
    findInHistory(botHistory, minTemp, maxTemp);
}

void ledOn(void)
{
    leds = CRGB(rOn, gOn, bOn);
    FastLED.show();
}

void ledOff(void)
{
    leds = CRGB(rOff, gOff, bOff);
    FastLED.show();
}

void ledOk(void)
{
    rOn = 0; gOn = 50; bOn = 0;
    rOff = 0; gOff = 1; bOff = 0; 
}

void ledNoData(void)
{
    rOn = 0; gOn = 0; bOn = 50;
    rOff = 0; gOff = 0; bOff = 5; 
}

void ledRedZone(void)
{
    rOn = 55; gOn = 0; bOn = 0;
    rOff = 255; gOff = 0; bOff = 0;  
}

void ledYellowZone(void)
{
    rOn = 55; gOn = 55; bOn = 0;
    rOff = 5; gOff = 5; bOff = 0;  
}

void updateLed(void)
{
    

    if (ledStatus)
        ledOn();
    else 
        ledOff();
}

void processTempIndication(void)
{
    ledOk();
    if ((topTemp == NO_TEMP) || (botTemp == NO_TEMP))
        ledNoData();

    if ((topTemp > TOP_YELLOW_C) || (botTemp > BOT_YELLOW_C))
        ledYellowZone();

    if ((topTemp > TOP_RED_C) || (botTemp > BOT_RED_C))
        ledRedZone();

    topFontColor = TFT_DARKGREEN;
    botFontColor = TFT_DARKGREEN;

    if ((topTemp == NO_TEMP) || (botTemp == NO_TEMP))
    {
        topFontColor = TFT_BLUE;
        botFontColor = TFT_BLUE;
    }


    if (topTemp > TOP_YELLOW_C)
        topFontColor = TFT_YELLOW;
    
    if (botTemp > BOT_YELLOW_C)
        botFontColor = TFT_YELLOW;

    if (topTemp > TOP_RED_C)
        topFontColor = TFT_RED;

    if (botTemp > BOT_RED_C)
        botFontColor = TFT_RED;
}   


bool getPressed(void)
{
    return (digitalRead(BTN_PIN) == LOW);
}

void processButton(uint16_t intMs = 15000)
{    
    if (tftActive)
    {
        if ((millis() - lastPressedMs > intMs)||(getPressed()))
        {
            digitalWrite(TFT_LEDA_PIN, 1);
            Serial.println("<<< TFT OFF >>>");
            tftActive = false;
            ledStatus = false;
            
            tftActive = false;
            lastPressedMs = millis();
            while (millis() - lastPressedMs < 3000)
                if (getPressed())  delay(10);
                    else break;
            delay(100);
        }
    }
    else 
    {
        if (getPressed())
        {
            digitalWrite(TFT_LEDA_PIN, 0);
            Serial.println("<<< PRESSED >>>");
            //leds = CHSV(0, 0XFF, 100);
            ledStatus = true;
            tftActive = true;            
            lastPressedMs = millis();
            while (millis() - lastPressedMs < 3000)
                if (getPressed())  delay(10);
                    else break;
            delay(100);
        }   
    }
    
}

void updateTemp(void)
{
    if (millis() - lastTopRcvd > TEMP_LIFE_TIME_MS)
        topTemp = NO_TEMP;

    if (millis() - lastBotRcvd > TEMP_LIFE_TIME_MS)
        botTemp = NO_TEMP;
}


void updateScreen(void)
{
    if (millis() - lastScreenUpdatedMs < 3000)
        return;

    char buf1[64];
    char buf2[64];
    float minT, maxT;
    String minS = "N/A", maxS = "N/A", minmaxS;

    String  topAgoMnS = String((millis() - lastTopRcvd) / 60000) + "m ago";
    String  botAgoMnS = String((millis() - lastBotRcvd) / 60000) + "m ago";

    sprintf(buf1, "%.1f", topTemp);
    if (topTemp == NO_TEMP)
    {
        strcpy(buf1, "N/A");
        //topAgoMnS = "no data";
    }
    sprintf(buf2, "%.1f", botTemp);
    if (botTemp == NO_TEMP)
    {
        strcpy(buf2, "N/A");
        //botAgoMnS = "no data";
    }

    lastScreenUpdatedMs = millis();    
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(topFontColor, TFT_BLACK);
    tft.drawString(buf1, 0, 0, 2);      
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString(topAgoMnS.c_str(), 80, 0, 1);      
    tft.setCursor(130, 0, 1);
    tft.printf("%.1fV", topV);    
    findInTopHistory(minT, maxT);
    tft.setCursor(80, 20, 1);  
    if (minT != NO_TEMP)
        tft.printf("%.1f..", minT);
    else 
        tft.print("N/A..");

    if (maxT != NO_TEMP)
        tft.printf("%.1f", maxT);
    else 
        tft.print("N/A");

    tft.setTextSize(2);
    tft.setTextColor(botFontColor, TFT_BLACK);
    tft.drawString(buf2, 0, 48, 2);    
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLUE, TFT_BLACK);
    tft.drawString(botAgoMnS.c_str(), 80, 48, 1);      
    tft.setCursor(130, 48, 1);
    tft.printf("%.1fV", botV);
    findInBotHistory(minT, maxT);
    tft.setCursor(80, 48+20, 1);  
    if (minT != NO_TEMP)
        tft.printf("%.1f..", minT);
    else 
        tft.print("N/A..");

    if (maxT != NO_TEMP)
        tft.printf("%.1f", maxT);
    else 
        tft.print("N/A");

}

void processReceiver(void)
{
    tEspPacket rData;    
    if (receivePacket(&rData))
    { 
        rData.print();
        //Serial.println("-------\n\n");
        for (auto i = 0; i < rData.paramsCount; i++)
        {
            switch(rData.params[i].paramType)
            {                                
                case ptString:          
                    //Serial.print("***"); rData.params[i].print();                          
                    if (String(rData.params[i].paramName) == "deviceName")
                    {
                        //Serial.print("==="); rData.params[i].print();
                        if (String(rData.params[i].strParam) == TOP_NAME)    
                        {
                            Serial.printf(">>>> %s: t = %.1f\r\n",  TOP_NAME, rData.tempC);
                            topTemp = rData.tempC;
                            lastTopRcvd = millis();
                            addToTopHistory(topTemp);
                            topV = (float) rData.batVCC / 1000;
                        }
                        if (String(rData.params[i].strParam) == BOT_NAME)    
                        {
                            Serial.printf(">>>> %s: t = %.1f\r\n",  BOT_NAME, rData.tempC);
                            botTemp = rData.tempC;
                            lastBotRcvd = millis();
                            addToBotHistory(botTemp);
                            botV = (float) rData.batVCC / 1000;
                        }
                    }        

                break;
            }
        }
    
    }
}

void dongleSetup(void)
{    
    pinMode(TFT_LEDA_PIN, OUTPUT);  
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    digitalWrite(TFT_LEDA_PIN, 0);
    tft.setTextFont(1);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    leds = CRGB(10, 10, 10);
    FastLED.show();

    FastLED.addLeds<APA102, LED_DI_PIN, LED_CI_PIN, BGR>(&leds, 1);

    tft.drawString(" !\"#$%&'()*+,-./0123456", 0, 0, 2);
    tft.drawString("789:;<=>?@ABCDEFGHIJKL", 0, 16, 2);    
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("MNOPQRSTUVWXYZ[\\]^_`", 0, 32, 2);
    tft.drawString("abcdefghijklmnopqrstuvw", 0, 48, 2);

    resetHistory();

    while(true)
    {
        updateTemp();
        processTempIndication();
        processButton();
        updateLed();        
        updateScreen();
        processReceiver();
        delay(30);
    }
}


#endif //#ifdef DONGLE_RECEIVER

