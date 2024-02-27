#include <esp_now.h>
#include <WiFi.h>


#include "espRadio.h"

uint8_t broadcastAddress[] = {0xFF, 0xFF,0xFF,0xFF,0xFF,0xFF};

bool wasRadioInit = false;
bool receiverWasStarted = false;

QueueHandle_t radioQ;
/////////////////
bool initRadio(void) 
{
  if (wasRadioInit) return true;
  //WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println(WiFi.macAddress());
    
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return false;
  } else Serial.println("ESPNow init OK");
  

  // register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
   
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = ESP_CHANNEL;  
  peerInfo.encrypt = false;
  
  esp_err_t addStatus = esp_now_add_peer(&peerInfo);         
  
  if (addStatus == ESP_OK) {
        // Pair success
        Serial.println("Pair success");
        return true;
    }
    else if (addStatus == ESP_ERR_ESPNOW_NOT_INIT) {
        // How did we get so far!!
        Serial.println("ESPNOW Not Init");
        return false;
    }
    else if (addStatus == ESP_ERR_ESPNOW_ARG) {
        Serial.println("Invalid Argument");
        return false;
    }
    else if (addStatus == ESP_ERR_ESPNOW_FULL) {
        Serial.println("Peer list full");
        return false;
    }
    else if (addStatus == ESP_ERR_ESPNOW_NO_MEM) {
        Serial.println("Out of memory");
        return false;
    }
    else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
        Serial.println("Peer Exists");
        return true;
    }
    else {
        Serial.println("Not sure what happened");
        return false;
    }
    
 return true;
 
}
/////////////////
void sendPacket(tEspPacket *rData)
{
   wasRadioInit = initRadio();

   if (!wasRadioInit)
    {
        return;
    }
    
  esp_err_t result = esp_now_send(broadcastAddress, (byte*)rData, sizeof(tEspPacket));
     
  if (result == ESP_OK) 
  {
    Serial.print("Packet sent: ");
    rData->print();
    Serial.println();    
  }
  else {
    Serial.println("Error sending the pakcet!!!");
  }

}
/////////////////
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) 
{
  tEspPacket rData;
  memcpy(&rData, incomingData, sizeof(tEspPacket));
  xQueueSend(radioQ, &rData, 1000);
}
/////////////////
bool startReceiver(void)
{
    if (receiverWasStarted) return true;
    radioQ = xQueueCreate(ENOW_Q_LEN, sizeof(tEspPacket));

    if (esp_now_register_recv_cb(OnDataRecv) == ESP_OK)
    {   
        Serial.println("ESP receiver started");
        return true;
    }
    else 
    {
        Serial.println("ESP receiver FAILED!!!");        
    }
    return false;
}
/////////////////
bool receivePacket(tEspPacket *rData)
{
   wasRadioInit = initRadio();
   if (!wasRadioInit)
    {
        return false;
    }
    receiverWasStarted = startReceiver();
    if (!receiverWasStarted)
    {
        return false;
    }
    BaseType_t res = xQueueReceive(radioQ, rData, 100);

    if (res == pdTRUE) return true;

    return false;
}
/////////////////
void testSender(void)
{
    uint32_t packCntr = 0;
    while(true)
    {
        tEspPacket rData;
        rData.erase();
        rData.addString("stringParam_1", String("stringParam_") + packCntr);
        rData.addString("stringParam_2", String("stringParam_") + millis());
        rData.addNumeric("packCntr", packCntr);
        rData.addNumeric("millis", (double)millis() + 0.01);
        sendPacket(&rData);        
        packCntr++;
        Serial.printf("%d packets sent\r\n", packCntr);
        rData.print();
        Serial.println("\n\n\n");
        delay(3000);
    }
}
/////////////////
void testReceiver(void)
{
    uint32_t packCntr = 0;
    tEspPacket rData;
    while(true)
    {
        tEspPacket rData;
        if (receivePacket(&rData))
        {
            packCntr++;
            Serial.printf("%d packets received\r\n", packCntr);
            rData.print();
        }
        else delay(5000);
    }
}