#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#include "prefs/prefs.h"

extern void gwSetup(bool firstBoot);
extern void gwLoop();

extern void senderTestSetup(bool firstBoot);
extern void senderTestLoop();

extern void receiverTestSetup(bool firstBoot);
extern void receiverTestLoop();

extern void senderSetup(bool firstBoot);
extern void senderLoop();

extern void dongleSetup(void);

RTC_DATA_ATTR bool firstBoot = true;


void setup(void)
{
    bool fBoot = false;
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    Serial.begin(115200);
    Serial.println(">>> BOOT");

    if (firstBoot)
    {
        fBoot = firstBoot;
        firstBoot = false;
        appPrefs.load();
    }

    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_protocol( WIFI_IF_STA , WIFI_PROTOCOL_LR);
    esp_wifi_set_max_tx_power(84);
    esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);

    #ifdef ESP_THB_GW
        Serial.println("\n\n>>>>> ThingsBoardGW_C3 configuration\n\n");
        gwSetup(fBoot);
    #endif

    #ifdef ESP_SENDER_TEST
        Serial.println("\n\n>>>>> EspSenderTest_esp32dev configuration\n\n");
        senderTestSetup(fBoot);
    #endif

    #ifdef ESP_RECEIVER_TEST
        Serial.println("\n\n>>>>> EspReceiverTest_esp32dev configuration\n\n");
        receiverTestSetup(fBoot);
    #endif

    #ifdef ESP_SENDER
        Serial.println("\n\n>>>>> EspReceiverTest_esp32dev configuration\n\n");
        senderSetup(fBoot);
    #endif

    #ifdef DONGLE_RECEIVER
        dongleSetup();
    #endif

    
}


void loop(void)
{
    #ifdef ESP_THB_GW
        gwLoop();
    #endif

    #ifdef ESP_SENDER_TEST
        senderTestLoop();
    #endif

    #ifdef ESP_RECEIVER_TEST
        receiverTestLoop();
    #endif

    #ifdef ESP_SENDER

    #endif
}


