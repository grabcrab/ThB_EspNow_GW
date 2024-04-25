#ifndef __ESP_RADIO_H__
#define __ESP_RADIO_H__
#include  <Arduino.h>

#include "espPacket.h"

#define ENOW_Q_LEN          20
//#define ESP_CHANNEL         9

void sendPacket(tEspPacket *rData, uint16_t batVCC_ = PARAM_NO_VAL_16, float tempC_ = PARAM_NO_VAL_16, time_t timestamp_ = PARAM_NO_VAL_32);
bool receivePacket(tEspPacket *rData);
bool initRadio(void);

void testSender(void);
void testReceiver(void);

#endif