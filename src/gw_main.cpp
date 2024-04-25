#ifdef ESP_THB_GW
#include <Arduino.h>
#include <WiFi.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>
#include <esp_wifi.h>

#include "espRadio.h"
#include "utils.h"



#define TB_LED_BUILTIN LC_PIN

constexpr char WIFI_SSID[] = "Yam-Yam";
constexpr char WIFI_PASSWORD[] = "runner1978";
constexpr char TOKEN[] = "OxXb0e5fqqmDZqLPr2hh";
constexpr char THINGSBOARD_SERVER[] = "mqtt.thingsboard.cloud";
constexpr uint32_t MAX_MESSAGE_SIZE = 1024U;
constexpr uint16_t THINGSBOARD_PORT = 1883U;

WiFiClient wifiClient;
Arduino_MQTT_Client mqttClient(wifiClient);
ThingsBoard tb(mqttClient, MAX_MESSAGE_SIZE);

constexpr char BLINKING_INTERVAL_ATTR[] = "blinkingInterval";
constexpr char LED_MODE_ATTR[] = "ledMode";
constexpr char LED_STATE_ATTR[] = "ledState";

volatile bool attributesChanged = false;

volatile int ledMode = 0;

volatile bool ledState = false;

constexpr uint16_t BLINKING_INTERVAL_MS_MIN = 10U;
constexpr uint16_t BLINKING_INTERVAL_MS_MAX = 60000U;
volatile uint16_t blinkingInterval = 1000U;

uint32_t previousStateChange;

constexpr int16_t telemetrySendInterval = 10000U;
uint32_t previousDataSend;

constexpr std::array<const char *, 2U> SHARED_ATTRIBUTES_LIST = 
{
    LED_STATE_ATTR,
    BLINKING_INTERVAL_ATTR
};

constexpr std::array<const char *, 1U> CLIENT_ATTRIBUTES_LIST = 
{
  LED_MODE_ATTR
};

RPC_Response processSetLedMode(const RPC_Data &data);
const std::array<RPC_Callback, 1U> callbacks = {
  RPC_Callback{ "setLedMode", processSetLedMode }
};

uint32_t espPackCntr = 0;

void InitWiFi(void) 
{
    int cntr = 0;
    Serial.printf("Connecting to AP [%s][%s] ...", WIFI_SSID, WIFI_PASSWORD);
    // Attempting to establish a connection to the given WiFi network
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    ledBlue(lsOn);
    while (WiFi.status() != WL_CONNECTED) 
    {
        // Delay 500ms until a connection has been successfully established
        delay(500);
        Serial.print(".");
        cntr++;
        if (cntr > 30) 
        {
            WiFi.disconnect();
            delay(1);
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
            cntr = 0;
            Serial.println();
        }
    }
    ledGreen(lsOn);
    Serial.println("Connected to AP");
}

const bool reconnect(void) 
{    
    const wl_status_t status = WiFi.status();
    if (status == WL_CONNECTED) 
    {
        ledGreen(lsOn);
        return true;
    }

    // If we aren't establish a new connection to the given WiFi network
    InitWiFi();
    return true;
}

RPC_Response processSetLedMode(const RPC_Data &data) 
{
    Serial.println("Received the set led state RPC method");

    // Process data
    int new_mode = data;

    Serial.print("Mode to change: ");
    Serial.println(new_mode);

    if (new_mode != 0 && new_mode != 1) {
        return RPC_Response("error", "Unknown mode!");
    }

    ledMode = new_mode;

    attributesChanged = true;

    // Returning current mode
    return RPC_Response("newMode", (int)ledMode);
}

void processSharedAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), BLINKING_INTERVAL_ATTR) == 0) {
      const uint16_t new_interval = it->value().as<uint16_t>();
      if (new_interval >= BLINKING_INTERVAL_MS_MIN && new_interval <= BLINKING_INTERVAL_MS_MAX) {
        blinkingInterval = new_interval;
        Serial.print("Blinking interval is set to: ");
        Serial.println(new_interval);
      }
    } else if (strcmp(it->key().c_str(), LED_STATE_ATTR) == 0) {
      ledState = it->value().as<bool>();
      digitalWrite(TB_LED_BUILTIN, ledState);
      
      Serial.print("LED state is set to: ");
      Serial.println(ledState);
    }
  }
  attributesChanged = true;
}

void processClientAttributes(const Shared_Attribute_Data &data) {
  for (auto it = data.begin(); it != data.end(); ++it) {
    if (strcmp(it->key().c_str(), LED_MODE_ATTR) == 0) {
      const uint16_t new_mode = it->value().as<uint16_t>();
      ledMode = new_mode;
    }
  }
}

const Shared_Attribute_Callback attributes_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_shared_request_callback(&processSharedAttributes, SHARED_ATTRIBUTES_LIST.cbegin(), SHARED_ATTRIBUTES_LIST.cend());
const Attribute_Request_Callback attribute_client_request_callback(&processClientAttributes, CLIENT_ATTRIBUTES_LIST.cbegin(), CLIENT_ATTRIBUTES_LIST.cend());


void gwSetup(bool firstBoot)
{
    Serial.begin(115200); 
    Serial.println("\n\n>>>BOOT");     
    #ifdef TEST_ESP_SENDER
        WiFi.mode(WIFI_STA);
        esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);
        Serial.println("ESP SENDER TEST");
        testSender();
        while(true);
    #endif

    #ifdef TEST_ESP_RCVR
        Serial.println("ESP RECEIVER TEST");
        WiFi.mode(WIFI_AP_STA);
        esp_wifi_set_channel(ESP_CHANNEL, WIFI_SECOND_CHAN_NONE);
        InitWiFi();
        testReceiver();
        while(true);
    #endif     

    pinMode(TB_LED_BUILTIN, OUTPUT);
    ledInit();
    ledHello();
    InitWiFi();

}

void gwLoop(void)
{
    delay(10);

    if (!reconnect()) 
    {
        return;
    }

    if (!tb.connected()) 
    {
        // Connect to the ThingsBoard
        Serial.print("Connecting to: ");
        Serial.print(THINGSBOARD_SERVER);
        Serial.print(" with token ");
        Serial.println(TOKEN);
        if (!tb.connect(THINGSBOARD_SERVER, TOKEN, THINGSBOARD_PORT)) 
        {
            Serial.println("Failed to connect");
            return;
        }
        // Sending a MAC address as an attribute
        tb.sendAttributeData("macAddress", WiFi.macAddress().c_str());

        // Serial.println("Subscribing for RPC...");
        // // Perform a subscription. All consequent data processing will happen in
        // // processSetLedState() and processSetLedMode() functions,
        // // as denoted by callbacks array.
        // if (!tb.RPC_Subscribe(callbacks.cbegin(), callbacks.cend())) 
        // {
        //     Serial.println("Failed to subscribe for RPC");
        //     return;
        // }

        // if (!tb.Shared_Attributes_Subscribe(attributes_callback)) 
        // {
        //     Serial.println("Failed to subscribe for shared attribute updates");
        //     return;
        // }

        // Serial.println("Subscribe done");

        // // Request current states of shared attributes
        // if (!tb.Shared_Attributes_Request(attribute_shared_request_callback)) 
        // {
        //     Serial.println("Failed to request for shared attributes");
        //     return;
        // }

        // // Request current states of client attributes
        // if (!tb.Client_Attributes_Request(attribute_client_request_callback)) 
        // {
        //     Serial.println("Failed to request for client attributes");
        //     return;
        // }
    }

    // if (attributesChanged) 
    // {
    //     attributesChanged = false;
    //     if (ledMode == 0) 
    //     {
    //         previousStateChange = millis();
    //     }
    //     tb.sendTelemetryData(LED_MODE_ATTR, ledMode);
    //     tb.sendTelemetryData(LED_STATE_ATTR, ledState);
    //     tb.sendAttributeData(LED_MODE_ATTR, ledMode);
    //     tb.sendAttributeData(LED_STATE_ATTR, ledState);
    // }

    // if (ledMode == 1 && millis() - previousStateChange > blinkingInterval) 
    // {
    //     previousStateChange = millis();
    //     ledState = !ledState;
    //     tb.sendTelemetryData(LED_STATE_ATTR, ledState);
    //     tb.sendAttributeData(LED_STATE_ATTR, ledState);
    //     //if (LED_BUILTIN == 99) 
    //     {
    //         Serial.print("LED state changed to: ");
    //         Serial.println(ledState);
    //     } 
    //     //else 
    //     {
    //         digitalWrite(TB_LED_BUILTIN, ledState);
    //     }
    // }
    
    if (millis() - previousDataSend > telemetrySendInterval) 
    {
        previousDataSend = millis();
//        tb.sendTelemetryData("temperature", random(10, 20));
        tb.sendAttributeData("rssi", WiFi.RSSI());
        tb.sendAttributeData("channel", WiFi.channel());
        tb.sendAttributeData("bssid", WiFi.BSSIDstr().c_str());
        tb.sendAttributeData("localIp", WiFi.localIP().toString().c_str());
        tb.sendAttributeData("ssid", WiFi.SSID().c_str());
    }


    #ifdef ENOW_GW_MODE
    tEspPacket rData;
    if (receivePacket(&rData))
    {
        espPackCntr++;
        Serial.printf("%d packets received\r\n", espPackCntr);
        rData.print();
        for (auto i = 0; i < rData.paramsCount; i++)
        {
            switch(rData.params[i].paramType)
            {
                case ptNumeric:
                    tb.sendAttributeData(rData.params[i].paramName, rData.params[i].numParam);
                break;

                case ptString:
                    tb.sendAttributeData(rData.params[i].paramName, rData.params[i].strParam);
                break;
            }
        }
    }

    #endif

    tb.loop();
}
#endif


