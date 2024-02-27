#include <Arduino.h>
#include "espRadio.h"
#include "tcu_board.h"

static uint32_t packCntr = 0;

void senderTestSetup()
{
    boardInit();
}

void senderTestLoop()
{    
    int batMv = boardGetBatteryVoltageMV();
    tEspPacket rData;
    rData.erase();
    rData.addNumeric("packCntr", packCntr);
    rData.addNumeric("millis", (double)millis());
    rData.addString("batMv", String(batMv));
    boardLedOn();
    sendPacket(&rData);
    delay(0.3 * SENDER_TEST_INTERVAL_MS);
    boardLedOff();
    packCntr++;
    Serial.printf("%d packets sent\r\n", packCntr);
    rData.print();
    Serial.println("\n\n\n");
    delay(0.7 * SENDER_TEST_INTERVAL_MS);
}
