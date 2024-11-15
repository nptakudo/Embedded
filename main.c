/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "software_timer.h"
#include "lcd.h"
#include "ds3231.h"
#include "button.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define NORMAL          0
#define MODIFY_TIME     7
#define MODIFY_ALARM    8
#define SET_HOUR        1
#define SET_MIN         2
#define SET_SEC         9
#define SET_DAY         3
#define SET_DATE        4
#define SET_MONTH       5
#define SET_YEAR        6
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
int status = NORMAL;
int statusModifying = SET_HOUR;
int hourTemp = 0,minTemp = 0,secTemp = 0,dayTemp = 1,dateTemp = 1,monthTemp = 1,yearTemp = 0;
int hourAlarm = 25, minAlarm = 0;
int timeBlink = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void DisplayTime();
void UpdateTime();
unsigned char IsButtonMode();
unsigned char IsButtonUp();
unsigned char IsButtonSave();
void setHour();
void setMin();
void setSec();
void setDay();
void setDate();
void setMonth();
void setYear();
void fsm();
void modifyTimeFsm();
void modifyAlarmFsm();
void displayHour(int num, int isBlink);
void displayMin(int num, int isBlink);
void displaySec(int num, int isBlink);
void displayDay(int num, int isBlink);
void convertDay(int num);
void displayDate(int num, int isBlink);
void displayMonth(int num, int isBlink);
void convertMonth(int num);
void displayYear(int num, int isBlink);
void displayScreen();
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_TIM2_Init();
	MX_SPI1_Init();
	MX_FSMC_Init();
	MX_I2C1_Init();
	MX_TIM4_Init();
	/* USER CODE BEGIN 2 */
	system_init();
	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	lcd_clear(BLACK);
	UpdateTime();// update the curret time

	while (1) {
		if (timer2_flag) {
		timer2_flag = 0;
		ds3231_read_time();
        button_scan();
        fsm();
        displayScreen();
		}

    /* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
void system_init() {
	HAL_GPIO_WritePin(OUTPUT_Y0_GPIO_Port, OUTPUT_Y0_Pin, 0);
	HAL_GPIO_WritePin(OUTPUT_Y1_GPIO_Port, OUTPUT_Y1_Pin, 0);
	HAL_GPIO_WritePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin, 0);

	button_init ();
	lcd_init();
	ds3231_init();

	timer2_init();
	timer2_set(50);
}

void UpdateTime() {// update the current time
	ds3231_write(ADDRESS_YEAR, 24);
	ds3231_write(ADDRESS_MONTH, 11);
	ds3231_write(ADDRESS_DATE, 14);
	ds3231_write(ADDRESS_DAY, 5);
	ds3231_write(ADDRESS_HOUR, 13);
	ds3231_write(ADDRESS_MIN, 30);
	ds3231_write(ADDRESS_SEC, 0);
}

void displayHour(int num, int isBlink) {
	//if(status == INIT_SYSTEM) ds3231_read_time();
	if(isBlink){
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(70, 100, "  ", GREEN, BLACK, 24, 0);
	    }
	    else {
		lcd_show_int_num(70, 100, num/10, 1, GREEN, BLACK, 24);
		lcd_show_int_num(83, 100, num%10, 1, GREEN, BLACK, 24);
		//lcd_show_int_num (70 ,100 , num ,2, GREEN ,BLACK ,24) ;
	    }
	}
	else {
		lcd_show_int_num(70, 100, num/10, 1, GREEN, BLACK, 24);
		lcd_show_int_num(83, 100, num%10, 1, GREEN, BLACK, 24);
	}
}

void displayMin(int num, int isBlink) {
	lcd_show_char(96, 100, ':', GREEN, BLACK, 24, 0);
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(110, 100, "  ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	lcd_show_int_num(110, 100, num/10, 1, GREEN, BLACK, 24);
	    	lcd_show_int_num(123, 100, num%10, 1, GREEN, BLACK, 24);
	    }
		//lcd_show_int_num (110 ,100 , num ,2, GREEN ,BLACK ,24) ;
	}
	else {
    	lcd_show_int_num(110, 100, num/10, 1, GREEN, BLACK, 24);
    	lcd_show_int_num(123, 100, num%10, 1, GREEN, BLACK, 24);
	}

}

void displaySec(int num, int isBlink) {
	lcd_show_char(136, 100, ':', GREEN, BLACK, 24, 0);
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(150, 100, "  ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	lcd_show_int_num(150, 100, num/10, 1, GREEN, BLACK, 24);
	    	lcd_show_int_num(163, 100, num%10, 1, GREEN, BLACK, 24);
	    }
	}
	else {
    	lcd_show_int_num(150, 100, num/10, 1, GREEN, BLACK, 24);
    	lcd_show_int_num(163, 100, num%10, 1, GREEN, BLACK, 24);
	}
}

void convertDay(int num) {
	switch(num)
		    {
		        case 1:
		        	lcd_show_string(20, 130, "SUN", YELLOW, BLACK, 24, 0);
		            break;
		        case 2:
		        	lcd_show_string(20, 130, "MON", YELLOW, BLACK, 24, 0);
		            break;
		        case 3:
		        	lcd_show_string(20, 130, "TUE", YELLOW, BLACK, 24, 0);
		            break;
		        case 4:
		        	lcd_show_string(20, 130, "WED", YELLOW, BLACK, 24, 0);
		            break;
		        case 5:
		        	lcd_show_string(20, 130, "THU", YELLOW, BLACK, 24, 0);
		            break;
		        case 6:
		        	lcd_show_string(20, 130, "FRI", YELLOW, BLACK, 24, 0);
		            break;
		        case 7:
		        	lcd_show_string(20, 130, "SAT", YELLOW, BLACK, 24, 0);
		            break;
		    }
}
void displayDay(int num, int isBlink) {
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(20, 130, "   ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	convertDay(num);
	    }
	}
	else convertDay(num);
}

void displayDate(int num, int isBlink) {
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(70, 130, "  ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	lcd_show_int_num(70, 130, num, 2, YELLOW, BLACK, 24);
	    }
	}
	else {
    		lcd_show_int_num(70, 130, num, 2, YELLOW, BLACK, 24);
	}
}

void convertMonth(int num) {
    switch(num)
    {
        case 1:
        	lcd_show_string(105, 130, "JAN", YELLOW, BLACK, 24, 0);
            break;
        case 2:
        	lcd_show_string(105, 130, "FEB", YELLOW, BLACK, 24, 0);
            break;
        case 3:
        	lcd_show_string(105, 130, "MAR", YELLOW, BLACK, 24, 0);
            break;
        case 4:
        	lcd_show_string(105, 130, "APR", YELLOW, BLACK, 24, 0);
            break;
        case 5:
        	lcd_show_string(105, 130, "MAY", YELLOW, BLACK, 24, 0);
            break;
        case 6:
        	lcd_show_string(105, 130, "JUN", YELLOW, BLACK, 24, 0);
            break;
        case 7:
        	lcd_show_string(105, 130, "JUL", YELLOW, BLACK, 24, 0);
            break;
        case 8:
        	lcd_show_string(105, 130, "AUG", YELLOW, BLACK, 24, 0);
            break;
        case 9:
        	lcd_show_string(105, 130, "SEP", YELLOW, BLACK, 24, 0);
            break;
        case 10:
        	lcd_show_string(105, 130, "OCT", YELLOW, BLACK, 24, 0);
            break;
        case 11:
        	lcd_show_string(105, 130, "NOV", YELLOW, BLACK, 24, 0);
            break;
        case 12:
        	lcd_show_string(105, 130, "DEC", YELLOW, BLACK, 24, 0);
            break;
    }
}
void displayMonth(int num, int isBlink) {
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(105, 130, "   ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	convertMonth(num);
	    }
	}
	else {
		convertMonth(num);
	}
}

void displayYear(int num, int isBlink) {
	if (isBlink) {
	    timeBlink = (timeBlink + 1)%10;
	    if(timeBlink < 5) {
	    	lcd_show_string(150, 130, "    ", GREEN, BLACK, 24, 0);
	    }
	    else {
	    	lcd_show_int_num(150, 130, 20, 2, YELLOW, BLACK, 24);
	    	lcd_show_int_num(176, 130, num/10, 1, YELLOW, BLACK, 24);
	    	lcd_show_int_num(189, 130, num%10, 1, YELLOW, BLACK, 24);
	    }
	}
	else {
    	lcd_show_int_num(150, 130, 20, 2, YELLOW, BLACK, 24);
    	lcd_show_int_num(176, 130, num/10, 1, YELLOW, BLACK, 24);
    	lcd_show_int_num(189, 130, num%10, 1, YELLOW, BLACK, 24);
	}
}

void displayScreen()
{
	if (status == NORMAL) {
		displayHour(ds3231_hours,0);
		displayMin(ds3231_min,0);
		displaySec(ds3231_sec,0);
		displayDay(ds3231_day,0);
		displayDate(ds3231_date,0);
		displayMonth(ds3231_month,0);
		displayYear(ds3231_year,0);
	}
	else {
		if (statusModifying == SET_HOUR) {
			displayHour(hourTemp,1);
			displayMin(minTemp,0);
			displaySec(secTemp,0);
			displayDay(dayTemp,0);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_MIN) {
			displayHour(hourTemp,0);
			displayMin(minTemp,1);
			displaySec(secTemp,0);
			displayDay(dayTemp,0);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_SEC) {
			displayHour(hourTemp,0);
			displayMin(minTemp,0);
			displaySec(secTemp,1);
			displayDay(dayTemp,0);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_DAY) {
			displayHour(hourTemp,0);
			displayMin(minTemp,0);
			displaySec(secTemp,0);
			displayDay(dayTemp,1);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_DATE) {
			displayHour(hourTemp,0);
			displayMin(minTemp,0);
			displaySec(secTemp,0);
			displayDay(dayTemp,0);
			displayDate(dateTemp,1);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_MONTH) {
			displayHour(hourTemp,0);
			displayMin(minTemp,0);
			displaySec(secTemp,0);
			displayDay(dayTemp,0);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,1);
			displayYear(yearTemp,0);
		}

		else if (statusModifying == SET_YEAR) {
			displayHour(hourTemp,0);
			displayMin(minTemp,0);
			displaySec(secTemp,0);
			displayDay(dayTemp,0);
			displayDate(dateTemp,0);
			displayMonth(monthTemp,0);
			displayYear(yearTemp,1);
		}
	}
	//displayState
	if (status == NORMAL) {
		lcd_show_string(20, 160, "1.NOR", YELLOW, BLACK, 24, 0);
	}
	else if (status == MODIFY_TIME) {
		lcd_show_string(20, 160, "2.MOD", YELLOW, BLACK, 24, 0);
	}
	else {
		lcd_show_string(20, 160, "3.ALR", YELLOW, BLACK, 24, 0);
	}
	if (ds3231_hours == hourAlarm && ds3231_min == minAlarm) {
		lcd_show_string(20, 190, "DING DONG", RED, BLACK, 24, 0);
	}

}

void modifyTimeFsm() {
	switch(statusModifying)
	    {
	        case SET_HOUR:
	        	setHour();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_HOUR, hourTemp);
	                statusModifying = SET_MIN;
	            }
	            break;

	        case SET_MIN:
	            setMin();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_MIN, minTemp);
	                statusModifying = SET_SEC;
	            }
	            break;

	        case SET_SEC:
	            setSec();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_SEC, secTemp);
	                statusModifying = SET_DAY;
	            }
	            break;

	        case SET_DAY:
	            setDay();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_DAY, dayTemp);
	                statusModifying = SET_DATE;
	            }
	            break;

	        case SET_DATE:
	            setDate();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_DATE, dateTemp);
	                statusModifying = SET_MONTH;
	            }
	            break;

	        case SET_MONTH:
	            setMonth();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_MONTH, monthTemp);
	                statusModifying = SET_YEAR;
	            }
	            break;

	        case SET_YEAR:
	            setYear();
	            if(IsButtonSave()) {
	            	ds3231_write(ADDRESS_YEAR, yearTemp);
	                statusModifying = SET_HOUR;
	            }
	            break;


	        default:
	            statusModifying = SET_HOUR;
	            break;
	    }
}

void modifyAlarmFsm(){
	switch(statusModifying) {
	        case SET_HOUR:
	        	setHour();
	            if(IsButtonSave()) {
	            	hourAlarm = hourTemp;
	                statusModifying = SET_MIN;
	            }
	            break;

	        case SET_MIN:
	            setMin();
	            if(IsButtonSave()) {
	            	minAlarm = minTemp;
	                statusModifying = SET_HOUR;
	            }
	            break;

	        default:
	            statusModifying = SET_HOUR;
	            break;
	}
}

void fsm()
{
    switch(status)
    {
        case NORMAL:
            if(IsButtonMode()) {
                status = MODIFY_TIME;
            	statusModifying = SET_HOUR;
            }
            break;
        case MODIFY_TIME:
            modifyTimeFsm();
            if(IsButtonMode()) {
                status = MODIFY_ALARM;
                statusModifying = SET_HOUR;
            }
            break;
        case MODIFY_ALARM:
            modifyAlarmFsm();
            if(IsButtonMode())
                status = NORMAL;
            break;
        default:
            status = NORMAL;
            break;
    }
}
//button change mode
unsigned char IsButtonMode()
{
    if (button_count[0] == 1)
        return 1;
    else
        return 0;
}

//increase parameter
unsigned char IsButtonUp()
{
	if ((button_count[3] == 1) || (button_count[3] >= 40 && button_count[3] % 4 == 0))
        return 1;
    else
        return 0;
}

//save parameter
unsigned char IsButtonSave()
{
    if (button_count[12] == 1)
        return 1;
    else
        return 0;
}

void setHour()
{
    if(IsButtonUp())
    {
        hourTemp++;
        if(hourTemp > 23)
            hourTemp = 0;
    }
}

void setMin()
{
    if(IsButtonUp())
    {
        minTemp++;
        if(minTemp > 59)
            minTemp = 0;
    }
}

void setSec()
{

    if(IsButtonUp())
    {
        secTemp++;
        if(secTemp > 59)
            secTemp = 0;
    }
}

void setDay()
{

    if(IsButtonUp())
    {
        dayTemp++;
        if(dayTemp > 7)
            dayTemp = 1;
    }
}

void setDate()
{

    if(IsButtonUp())
    {
        dateTemp++;
        if(dateTemp > 31)
            dateTemp = 1;
    }
}

void setMonth()
{

    if(IsButtonUp())
    {
        monthTemp++;
        if(monthTemp > 12)
            monthTemp = 1;
    }
}

void setYear()
{

    if(IsButtonUp())
    {
        yearTemp++;
        if(yearTemp > 99)
            yearTemp = 0;
    }
}



/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
