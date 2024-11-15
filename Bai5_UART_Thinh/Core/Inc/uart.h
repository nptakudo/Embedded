/*
 * uart.h
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#ifndef INC_UART_H_
#define INC_UART_H_

#include "main.h"

#define RING_BUFFER_SIZE 256

typedef struct {
    uint8_t buffer[RING_BUFFER_SIZE];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} ring_buffer_t;

extern UART_HandleTypeDef huart1;
extern uint8_t receive_buffer1;
extern ring_buffer_t uart_ring_buffer;
extern volatile uint8_t uart_data_ready;

// Function declarations
void uart_init_rs232(void);
void uart_Rs232SendString(const uint8_t* str);
void uart_Rs232SendBytes(uint8_t* bytes, uint16_t size);
void uart_Rs232SendNum(uint32_t num);
void uart_Rs232SendNumPercent(uint32_t num);

// Ring buffer functions
void ring_buffer_init(ring_buffer_t *ring_buffer);
uint8_t ring_buffer_put(ring_buffer_t *ring_buffer, uint8_t data);
uint8_t ring_buffer_get(ring_buffer_t *ring_buffer, uint8_t *data);
uint16_t ring_buffer_available(ring_buffer_t *ring_buffer);

#endif /* INC_UART_H_ */

