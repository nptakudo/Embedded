#include "traffic_light.h"

static TrafficMode currentMode = MODE_NORMAL;
static TrafficState currentState = STATE_RED_GREEN;
static TrafficTiming timing = {20, 15, 5}; // Default timings
static uint32_t stateTimer = 0;
static uint8_t blinkTimer = 0;
static uint8_t showBlink = 0;

void traffic_Init(void) {
    lcd_Clear(WHITE);
    lcd_StrCenter(0, 2, "Traffic Light Control", BLACK, WHITE, 16, 1);
    traffic_DrawLights();
}

void traffic_FSM(void) {
    static uint32_t lastTick = 0;
    uint32_t currentTick = HAL_GetTick();
    
    // Update every 500ms
    if (currentTick - lastTick >= 500) {
        lastTick = currentTick;
        blinkTimer ^= 1;
        
        if (currentMode == MODE_NORMAL) {
            stateTimer++;
            switch (currentState) {
                case STATE_RED_GREEN:
                    if (stateTimer >= timing.greenTime * 2) {
                        currentState = STATE_RED_YELLOW;
                        stateTimer = 0;
                    }
                    break;
                    
                case STATE_RED_YELLOW:
                    if (stateTimer >= timing.yellowTime * 2) {
                        currentState = STATE_GREEN_RED;
                        stateTimer = 0;
                    }
                    break;
                    
                case STATE_GREEN_RED:
                    if (stateTimer >= timing.greenTime * 2) {
                        currentState = STATE_YELLOW_RED;
                        stateTimer = 0;
                    }
                    break;
                    
                case STATE_YELLOW_RED:
                    if (stateTimer >= timing.yellowTime * 2) {
                        currentState = STATE_RED_GREEN;
                        stateTimer = 0;
                    }
                    break;
            }
        }
        traffic_DrawLights();
    }
}

void traffic_DrawLights(void) {
    // Clear previous lights
    lcd_Fill(20, 50, 220, 200, WHITE);
    
    // Draw traffic light frames
    lcd_DrawRectangle(40, 60, 100, 180, BLACK);
    lcd_DrawRectangle(140, 60, 200, 180, BLACK);
    
    // Draw lights based on current state and mode
    if (currentMode == MODE_NORMAL) {
        // First traffic light
        switch (currentState) {
            case STATE_RED_GREEN:
            case STATE_RED_YELLOW:
                lcd_DrawCircle(70, 90, RED, 15, 1);
                break;
            case STATE_GREEN_RED:
                lcd_DrawCircle(70, 130, GREEN, 15, 1);
                break;
            case STATE_YELLOW_RED:
                lcd_DrawCircle(70, 160, YELLOW, 15, 1);
                break;
        }
        
        // Second traffic light
        switch (currentState) {
            case STATE_RED_GREEN:
                lcd_DrawCircle(170, 130, GREEN, 15, 1);
                break;
            case STATE_RED_YELLOW:
                lcd_DrawCircle(170, 160, YELLOW, 15, 1);
                break;
            case STATE_GREEN_RED:
            case STATE_YELLOW_RED:
                lcd_DrawCircle(170, 90, RED, 15, 1);
                break;
        }
    } else {
        // Modification modes - blink the respective light
        if (blinkTimer) {
            switch (currentMode) {
                case MODE_RED_MODIFY:
                    lcd_DrawCircle(70, 90, RED, 15, 1);
                    lcd_DrawCircle(170, 90, RED, 15, 1);
                    break;
                case MODE_GREEN_MODIFY:
                    lcd_DrawCircle(70, 130, GREEN, 15, 1);
                    lcd_DrawCircle(170, 130, GREEN, 15, 1);
                    break;
                case MODE_YELLOW_MODIFY:
                    lcd_DrawCircle(70, 160, YELLOW, 15, 1);
                    lcd_DrawCircle(170, 160, YELLOW, 15, 1);
                    break;
            }
        }
    }
    
    // Display current mode and timings
    char buf[32];
    sprintf(buf, "Mode: %d", currentMode);
    lcd_ShowStr(10, 200, buf, BLACK, WHITE, 16, 0);
    
    sprintf(buf, "R:%d G:%d Y:%d", timing.redTime, timing.greenTime, timing.yellowTime);
    lcd_ShowStr(10, 220, buf, BLACK, WHITE, 16, 0);
}

void traffic_HandleButtons(void) {
    static uint8_t tempTiming = 0;
    
    // Mode button (assuming button 0)
    if (button_count[0] == 1) {
        currentMode = (currentMode % 4) + 1;
        if (currentMode != MODE_NORMAL) {
            switch (currentMode) {
                case MODE_RED_MODIFY:
                    tempTiming = timing.redTime;
                    break;
                case MODE_GREEN_MODIFY:
                    tempTiming = timing.greenTime;
                    break;
                case MODE_YELLOW_MODIFY:
                    tempTiming = timing.yellowTime;
                    break;
            }
        }
    }
    
    // Increment button (assuming button 1)
    if (currentMode != MODE_NORMAL && button_count[1] == 1) {
        tempTiming = (tempTiming % 99) + 1;
        char buf[32];
        sprintf(buf, "New Value: %d", tempTiming);
        lcd_ShowStr(10, 240, buf, BLACK, WHITE, 16, 0);
    }
    
    // Confirm button (assuming button 2)
    if (currentMode != MODE_NORMAL && button_count[2] == 1) {
        switch (currentMode) {
            case MODE_RED_MODIFY:
                timing.redTime = tempTiming;
                break;
            case MODE_GREEN_MODIFY:
                timing.greenTime = tempTiming;
                break;
            case MODE_YELLOW_MODIFY:
                timing.yellowTime = tempTiming;
                break;
        }
        lcd_Fill(10, 240, 200, 256, WHITE); // Clear "New Value" text
    }
}
