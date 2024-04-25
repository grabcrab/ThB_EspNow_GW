#include "leds.h"

void initLeds(void)
{
    pinMode(LED_AMBER_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
}

void ledAmber(bool state)
{
  if (state) digitalWrite(LED_AMBER_PIN, HIGH);
  else digitalWrite(LED_AMBER_PIN, LOW);
}

void ledGreen(bool state)
{
  if (state) digitalWrite(LED_GREEN_PIN, HIGH);
  else digitalWrite(LED_GREEN_PIN, LOW);
}
