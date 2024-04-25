#include <Arduino.h>
#include "espRadio.h"
#include "tcu_board.h"

static unsigned long lastReceivedMs = 0;
static unsigned long lastBlinkMs = 0;

void receiverTestSetup(bool firstBoot)
{
    boardInit();
}

void receiverTestLoop()
{
    tEspPacket rData;    
    if (receivePacket(&rData))
    { 
        rData.print();
        Serial.println("-------\n\n");
        lastReceivedMs = millis();
        boardLedOn();
    }
    else 
    {
        if (millis() - lastReceivedMs > SENDER_TEST_INTERVAL_MS * 1.7)
        {            
            if (millis() - lastBlinkMs > SENDER_TEST_INTERVAL_MS * 3)
            {
                boardLedOn();
                delay(50);
                lastBlinkMs = millis();
            }
            boardLedOff();
        }
    }
}
