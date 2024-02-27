#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

extern void gwSetup();
extern void gwLoop();

extern void senderTestSetup();
extern void senderTestLoop();


void setup(void)
{
    Serial.begin(115200);
    WiFi.mode(WIFI_AP_STA);
    esp_wifi_set_protocol( WIFI_IF_STA , WIFI_PROTOCOL_LR);
    esp_wifi_set_max_tx_power(84);
    esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);
    #ifdef ESP_THB_GW
        Serial.println("\n\n>>>>> ThingsBoardGW_C3 configuration\n\n");
        gwSetup();
    #endif

    #ifdef ESP_SENDER_TEST
        Serial.println("\n\n>>>>> EspSenderTest_esp32dev configuration\n\n");
        senderTestSetup();
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
}


