#ifndef __LEDS_H__
#define __LEDS_H__

#include <Arduino.h>

#define LED_AMBER_PIN   6
#define LED_GREEN_PIN   5

#define LED_ON          true
#define LED_OFF         false

void initLeds(void);
void ledAmber(bool state);
void ledGreen(bool state);

#endif
