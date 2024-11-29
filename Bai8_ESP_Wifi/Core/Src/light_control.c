/*
 * light_control.c
 *
 *  Created on: Nov 15, 2023
 *      Author: phamv
 */
#include "light_control.h"
#include "sensor.h"

uint8_t light_status = 0;

void lightProcess(){
	if(button_count[13] == 1){
		light_status = 1 - light_status;
		if(light_status == 1){
			uart_EspSendBytes("A", 1);
		} else {
			uart_EspSendBytes("a", 1);
		}
	}
	if(light_status == 1){
		HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 1);
	} else {
		HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	}
}

uint16_t elapsedTime = 0;
void test_Esp() {
	elapsedTime += 50;
	if (elapsedTime >= 1000) {
		elapsedTime = 0;
		if (uart_EspCheck() == 0) {
			// Display status on LCD
			lcd_ShowStr(10, 50, "Sending Temperature to ESP", GREEN, BLACK, 24, 0);

			// Get the temperature reading as a float
			sensor_Read();
			float temp = sensor_GetTemperature();

			// Extract the integer and fractional parts
			int tempWhole = (int)temp;                    // Integer part of temperature
			int tempFraction = (int)((temp - tempWhole) * 100); // Fractional part (2 decimal places)

			// Handle negative fractional part (if temp < 0)
			if (tempFraction < 0) {
				tempFraction = -tempFraction;
			}

			// Create the formatted temperature string
			char tempString[20];  // Ensure the buffer is large enough
			sprintf(tempString, "!TEMP:%d.%02d#", tempWhole, tempFraction);

			lcd_ShowStr(10, 100, tempString, GREEN, BLACK, 16, 0);


			// Send the formatted string via UART
			uart_EspSendBytes(tempString, strlen(tempString));
		}
	}
}



