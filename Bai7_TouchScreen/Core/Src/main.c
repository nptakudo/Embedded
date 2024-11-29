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
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <math.h>
#include "software_timer.h"
#include "led_7seg.h"
#include "button.h"
#include "lcd.h"
#include "picture.h"
#include "ds3231.h"
#include "sensor.h"
#include "buzzer.h"
#include "touch.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define INIT 0
#define DRAW 1
#define CLEAR 2

typedef struct {
    uint16_t x1;
    uint16_t y1; 
    uint16_t x2;
    uint16_t y2;
    int16_t dx1;  // Movement delta for x1
    int16_t dy1;  // Movement delta for y1
    int16_t dx2;  // Movement delta for x2
    int16_t dy2;  // Movement delta for y2
    uint16_t color;
} MovingLine;

int draw_Status = INIT;

// Initialize the moving line with multiple segments
#define SNAKE_WIDTH 10  // Width of the snake
#define SNAKE_LENGTH 100  // Length of the snake
#define MAX_DOTS 5     // Maximum number of food dots

MovingLine line = {
    .x1 = 120,           // Starting head position
    .y1 = 160,
    .x2 = 120,           // Starting tail position
    .y2 = 160 + SNAKE_LENGTH,  // Initial length
    .dx1 = 0,
    .dy1 = 0,
    .dx2 = 0,
    .dy2 = 0,
    .color = GREEN
};

// Add these new structures for food dots
typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t active;
} FoodDot;

FoodDot foodDots[MAX_DOTS];
uint32_t lastDotTime = 0;

#define ARROW_WIDTH 40
#define ARROW_HEIGHT 40
#define ARROW_SPACING 10

typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
} Button;
#define MAX_SEGMENTS 50  // Increased for longer snake
typedef struct {
    uint16_t x;
    uint16_t y;
} Point;
typedef struct {
    Point segments[MAX_SEGMENTS];
    int numSegments;
    int16_t dx;
    int16_t dy;
    uint16_t color;
} Snake;

Snake snake = {
    .segments = {{120, 160}},  // Start with head position
    .numSegments = 1,
    .dx = 0,
    .dy = 0,
    .color = GREEN
};

// Define arrow buttons
Button arrowUp = {100, 240, ARROW_WIDTH, ARROW_HEIGHT};
Button arrowDown = {100, 240 + ARROW_HEIGHT + ARROW_SPACING, ARROW_WIDTH, ARROW_HEIGHT};
Button arrowLeft = {100 - ARROW_WIDTH - ARROW_SPACING, 240 + ARROW_HEIGHT + ARROW_SPACING/2, ARROW_WIDTH, ARROW_HEIGHT};
Button arrowRight = {100 + ARROW_WIDTH + ARROW_SPACING, 240 + ARROW_HEIGHT + ARROW_SPACING/2, ARROW_WIDTH, ARROW_HEIGHT};

// Add movement speed
const int16_t MOVE_SPEED = 5;



/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void system_init();
void test_LedDebug();
void touchProcess();
uint8_t isButtonClear();
void drawArrowButton(Button *btn, const char *label, uint16_t color);
uint8_t isButtonPressed(Button *btn, uint16_t x, uint16_t y);
void drawArrowButtons(void);  
void drawThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color);
void createRandomDot(void);
void initFoodDots(void);
void updateMovingLine(void);
bool checkCollision(Point newHead);
void checkFoodCollision(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
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
  MX_TIM13_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
  system_init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  touch_Adjust();
 lcd_Clear(BLACK);
 while (1)
  {
	  //scan touch screen
	  touch_Scan();
	  //check if touch screen is touched
	  // if(touch_IsTouched() && draw_Status == DRAW){
    //         //draw a point at the touch position
		//   lcd_DrawPoint(touch_GetX(), touch_GetY(), RED);
	  // }
	  // 50ms task
	  if(flag_timer2 == 1){
		  flag_timer2 = 0;
		  touchProcess();
		  test_LedDebug();
		  updateMovingLine();
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
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void system_init(){
	  timer_init();
	  button_init();
	  lcd_init();
	  touch_init();
	  setTimer2(50);
	  initFoodDots();
	  lastDotTime = HAL_GetTick();
}

uint8_t count_led_debug = 0;

void test_LedDebug(){
	count_led_debug = (count_led_debug + 1)%20;
	if(count_led_debug == 0){
		HAL_GPIO_TogglePin(DEBUG_LED_GPIO_Port, DEBUG_LED_Pin);
	}
}

uint8_t isButtonClear(){
	if(!touch_IsTouched()) return 0;
	return touch_GetX() > 60 && touch_GetX() < 180 && touch_GetY() > 10 && touch_GetY() < 60;
}

void touchProcess(){
	switch (draw_Status) {
		case INIT:
                // display blue button
			lcd_Fill(60, 10, 180, 60, GBLUE);
			lcd_ShowStr(90, 20, "CLEAR", RED, BLACK, 24, 1);
			drawArrowButtons();
			draw_Status = DRAW;
			break;
		case DRAW:
			if(isButtonClear()){
				draw_Status = CLEAR;
                    // clear board
				lcd_Fill(0, 60, 240, 320, BLACK);
                    // display green button
				lcd_Fill(60, 10, 180, 60, GREEN);
				lcd_ShowStr(90, 20, "CLEAR", RED, BLACK, 24, 1);
				drawArrowButtons();
			}
			break;
		case CLEAR:
			if(!touch_IsTouched()) draw_Status = INIT;
			break;
		default:
			break;
	}
}

void updateMovingLine() {
    // Clear previous snake segments
    for(int i = 0; i < snake.numSegments-1; i++) {
        drawThickLine(snake.segments[i].x, snake.segments[i].y, 
                     snake.segments[i+1].x, snake.segments[i+1].y, 
                     SNAKE_WIDTH, BLACK);
    }
    
    // Handle controls
    if(touch_IsTouched()) {
        uint16_t touchX = touch_GetX();
        uint16_t touchY = touch_GetY();
        
        if(isButtonPressed(&arrowUp, touchX, touchY) && snake.dy >= 0) {
            snake.dy = -MOVE_SPEED;
            snake.dx = 0;
        }
        else if(isButtonPressed(&arrowDown, touchX, touchY) && snake.dy <= 0) {
            snake.dy = MOVE_SPEED;
            snake.dx = 0;
        }
        else if(isButtonPressed(&arrowLeft, touchX, touchY) && snake.dx >= 0) {
            snake.dx = -MOVE_SPEED;
            snake.dy = 0;
        }
        else if(isButtonPressed(&arrowRight, touchX, touchY) && snake.dx <= 0) {
            snake.dx = MOVE_SPEED;
            snake.dy = 0;
        }
    }
    
    // Calculate new head position
    Point newHead = {
        .x = snake.segments[0].x + snake.dx,
        .y = snake.segments[0].y + snake.dy
    };
    
    // Boundary checking
    if (newHead.x <= SNAKE_WIDTH/2) newHead.x = SNAKE_WIDTH/2;
    if (newHead.x >= lcddev.width - SNAKE_WIDTH/2) newHead.x = lcddev.width - SNAKE_WIDTH/2;
    if (newHead.y <= 60 + SNAKE_WIDTH/2) newHead.y = 60 + SNAKE_WIDTH/2;
    if (newHead.y >= lcddev.height - SNAKE_WIDTH/2) newHead.y = lcddev.height - SNAKE_WIDTH/2;
    
    // Move body segments
    for(int i = snake.numSegments-1; i > 0; i--) {
        snake.segments[i] = snake.segments[i-1];
    }
    
    // Update head
    snake.segments[0] = newHead;
    
    // Draw new snake segments
    for(int i = 0; i < snake.numSegments-1; i++) {
        drawThickLine(snake.segments[i].x, snake.segments[i].y,
                     snake.segments[i+1].x, snake.segments[i+1].y,
                     SNAKE_WIDTH, snake.color);
    }
}

void drawArrowButton(Button *btn, const char *label, uint16_t color) {
    lcd_DrawRectangle(btn->x, btn->y, btn->x + btn->width, btn->y + btn->height, color);
    lcd_ShowStr(btn->x + 5, btn->y + 10, (char*)label, BLACK, color, 16, 1);
}

uint8_t isButtonPressed(Button *btn, uint16_t x, uint16_t y) {
    return (x >= btn->x && x <= (btn->x + btn->width) &&
            y >= btn->y && y <= (btn->y + btn->height));
}

void drawArrowButtons() {
    drawArrowButton(&arrowUp, "^", GREEN);
    drawArrowButton(&arrowDown, "v", GREEN);
    drawArrowButton(&arrowLeft, "<", GREEN);
    drawArrowButton(&arrowRight, ">", GREEN);
}
bool checkCollision(Point newHead) {
    // Check wall collision
    if (newHead.x <= SNAKE_WIDTH/2 || 
        newHead.x >= lcddev.width - SNAKE_WIDTH/2 ||
        newHead.y <= 60 + SNAKE_WIDTH/2 || 
        newHead.y >= lcddev.height - SNAKE_WIDTH/2) {
        return true;
    }
    
    // Check self collision (skip head)
    for(int i = 1; i < snake.numSegments; i++) {
        if(abs(newHead.x - snake.segments[i].x) < SNAKE_WIDTH &&
           abs(newHead.y - snake.segments[i].y) < SNAKE_WIDTH) {
            return true;
        }
    }
    return false;
}
void checkFoodCollision() {
    Point head = snake.segments[0];
    
    for(int i = 0; i < MAX_DOTS; i++) {
        if(foodDots[i].active) {
            if(abs(head.x - foodDots[i].x) < SNAKE_WIDTH &&
               abs(head.y - foodDots[i].y) < SNAKE_WIDTH) {
                // Increase score
                // snake.score += 10;
                
                // Deactivate eaten food
                foodDots[i].active = 0;
                
                // Clear food dot from screen
                for(int w = -SNAKE_WIDTH/2; w <= SNAKE_WIDTH/2; w++) {
                    for(int h = -SNAKE_WIDTH/2; h <= SNAKE_WIDTH/2; h++) {
                        lcd_DrawPoint(foodDots[i].x + w, foodDots[i].y + h, BLACK);
                    }
                }
                
                // Don't remove last segment (allows growth)
                return;
            }
        }
    }
    
    // Remove last segment if no food was eaten
    if(snake.numSegments > 1) {
        snake.numSegments--;
    }
}
void drawThickLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color) {
    // Calculate the direction vector
    int dx = x2 - x1;
    int dy = y2 - y1;
    
    // Calculate the perpendicular vector (-dy, dx)
    float len = sqrt(dx*dx + dy*dy);
    if (len == 0) len = 1;  // Avoid division by zero
    
    float ux = -dy / len;
    float uy = dx / len;
    
    // Draw multiple parallel lines
    for(int i = -width/2; i <= width/2; i++) {
        int offset_x = i * ux;
        int offset_y = i * uy;
        lcd_DrawLine(x1 + offset_x, y1 + offset_y, 
                    x2 + offset_x, y2 + offset_y, color);
    }
}

void createRandomDot() {
    for(int i = 0; i < MAX_DOTS; i++) {
        if(!foodDots[i].active) {
            foodDots[i].x = 20 + (rand() % (lcddev.width - 40));  // Keep away from edges
            foodDots[i].y = 80 + (rand() % (lcddev.height - 160)); // Keep away from buttons
            foodDots[i].active = 1;
            // Draw the dot
            for(int w = -SNAKE_WIDTH/2; w <= SNAKE_WIDTH/2; w++) {
                for(int h = -SNAKE_WIDTH/2; h <= SNAKE_WIDTH/2; h++) {
                    lcd_DrawPoint(foodDots[i].x + w, foodDots[i].y + h, RED);
                }
            }
            break;
        }
    }
}

void initFoodDots() {
    for(int i = 0; i < MAX_DOTS; i++) {
        foodDots[i].active = 0;
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
