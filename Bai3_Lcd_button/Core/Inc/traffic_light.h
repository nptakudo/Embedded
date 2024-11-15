#ifndef TRAFFIC_LIGHT_H
#define TRAFFIC_LIGHT_H

#include "lcd.h"
#include "button.h"

// Modes
typedef enum {
    MODE_NORMAL = 1,
    MODE_RED_MODIFY = 2,
    MODE_GREEN_MODIFY = 3,
    MODE_YELLOW_MODIFY = 4
} TrafficMode;

// States for normal operation
typedef enum {
    STATE_RED_GREEN,
    STATE_RED_YELLOW,
    STATE_GREEN_RED,
    STATE_YELLOW_RED
} TrafficState;

// Traffic light timing structure
typedef struct {
    uint8_t redTime;
    uint8_t greenTime;
    uint8_t yellowTime;
} TrafficTiming;

// Function prototypes
void traffic_Init(void);
void traffic_FSM(void);
void traffic_DrawLights(void);
void traffic_UpdateTimings(void);
void traffic_HandleButtons(void);

#endif
