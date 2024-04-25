#include <Arduino.h>
#include "espRadio.h"
#include "tcu_board.h"

extern void sendEspPacket(void);

void senderTestSetup(bool firstBoot)
{    
    boardInit();
}

void senderTestLoop()
{    
    boardLedOn();
    sendEspPacket();
    boardLedOff();
    delay(SENDER_TEST_INTERVAL_MS);
}
