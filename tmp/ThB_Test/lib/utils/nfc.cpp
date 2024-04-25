#include "nfc.h"

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#include "leds.h"

#define     MAX_UID_SIZE            16
#define     AFTER_ERROR_DELAY_MS    3000
#define     AFTER_READ_DELAY        1000
#define     NFC_READ_TIMEOUT_MS     100


PN532_I2C   pn532i2c(Wire);
PN532       nfc(pn532i2c);
static      bool vPrinted   = false;

static void printUID(uint8_t *uid)
{
  Serial.printf("TAG %02x %02x %02x %02x %02x %02x %02x\r\n", uid[0], uid[1], uid[2], uid[3], uid[4], uid[5], uid[6]);    
}

bool nfcConnectReader(void) 
{  
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata)
    {
        Serial.println("PN53x card not found!");
        return false;
    }
    if (!vPrinted)
    {
        Serial.printf("Found chip PN5%X\r\n", (versiondata >> 24) & 0xFF); 
        Serial.printf("Firmware version: %d.%d\r\n", (versiondata >> 16) & 0xFF,(versiondata >> 8) & 0xFF);         
        vPrinted = true;
    }
    nfc.setPassiveActivationRetries(0xFF);
    nfc.SAMConfig();
  
    return true;
}

bool nfcProcess(void)
{  
    uint8_t uidLength;
    uint8_t uid[MAX_UID_SIZE]   = {0};

    bool    wasProcessed = false;
    bool    wasConnected  = false;

    wasConnected = nfcConnectReader();

    if (!wasConnected) 
    {
        ledGreen(LED_ON);
        delay(100);
        ledGreen(LED_OFF);
        delay(100);
        ledGreen(LED_ON);
        delay(100);
        ledGreen(LED_OFF);
        return false;
    }

    wasProcessed = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, NFC_READ_TIMEOUT_MS);

    if (wasProcessed)
    {
        printUID(uid);    
        ledAmber(LED_ON);
        delay(AFTER_READ_DELAY);
        ledAmber(LED_OFF);                
    }

    return true; //true when reader is connected
}