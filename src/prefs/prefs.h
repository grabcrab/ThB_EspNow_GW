#ifndef __PREFS_H__
#define __PREFS_H__

#include <Arduino.h>

struct tAppPrefs
{
    String   espSenderName = ESP_SENDER_NAME;       
	uint16_t espSenderIntervalS = ESP_SENDER_INTERVAL_S;	
    void load(void);
};

extern tAppPrefs appPrefs;

#endif