/*
 * uart.c
 *
 *  Created on: Sep 26, 2023
 *      Author: HaHuyen
 */
#include "uart.h"
#include <stdio.h>

static uint32_t mypow(uint8_t base, uint8_t exp) {
    uint32_t result = 1;
    for(uint8_t i = 0; i < exp; i++) {
        result *= base;
    }
    return result;
}

uint8_t receive_buffer1 = 0;
uint8_t msg[100];
ring_buffer_t uart_ring_buffer;
volatile uint8_t uart_data_ready = 0;

void ring_buffer_init(ring_buffer_t *ring_buffer) {
    ring_buffer->head = 0;
    ring_buffer->tail = 0;
    ring_buffer->count = 0;
}

uint8_t ring_buffer_put(ring_buffer_t *ring_buffer, uint8_t data) {
    if (ring_buffer->count >= RING_BUFFER_SIZE) {
        return 0; // Buffer full
    }
    
    ring_buffer->buffer[ring_buffer->head] = data;
    ring_buffer->head = (ring_buffer->head + 1) % RING_BUFFER_SIZE;
    ring_buffer->count++;
    return 1;
}

uint8_t ring_buffer_get(ring_buffer_t *ring_buffer, uint8_t *data) {
    if (ring_buffer->count == 0) {
        return 0; // Buffer empty
    }
    
    *data = ring_buffer->buffer[ring_buffer->tail];
    ring_buffer->tail = (ring_buffer->tail + 1) % RING_BUFFER_SIZE;
    ring_buffer->count--;
    return 1;
}

uint16_t ring_buffer_available(ring_buffer_t *ring_buffer) {
    return ring_buffer->count;
}

void uart_init_rs232() {
    ring_buffer_init(&uart_ring_buffer);
    HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
}

void uart_Rs232SendString(const uint8_t* str) {
    HAL_UART_Transmit(&huart1, (void*)msg, sprintf((void*)msg,"%s",str), 10);
}

void uart_Rs232SendBytes(uint8_t* bytes, uint16_t size) {
    HAL_UART_Transmit(&huart1, bytes, size, 10);
}

void uart_Rs232SendNum(uint32_t num) {
    if(num == 0){
        uart_Rs232SendString((uint8_t*)"0");
        return;
    }
    uint8_t num_flag = 0;
    int i;
    if(num < 0) uart_Rs232SendString((uint8_t*)"-");
    for(i = 10; i > 0; i--)
    {
        if((num / mypow(10, i-1)) != 0)
        {
            num_flag = 1;
            sprintf((void*)msg,"%lu",num/mypow(10, i-1));
            uart_Rs232SendString(msg);
        }
        else
        {
            if(num_flag != 0)
                uart_Rs232SendString((uint8_t*)"0");
        }
        num = num % mypow(10, i-1);
    }
}

void uart_Rs232SendNumPercent(uint32_t num) {
    sprintf((void*)msg,"%ld",num/100);
    uart_Rs232SendString((const uint8_t*)msg);
    uart_Rs232SendString((const uint8_t*)".");
    sprintf((void*)msg,"%ld",num%100);
    uart_Rs232SendString((const uint8_t*)msg);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        ring_buffer_put(&uart_ring_buffer, receive_buffer1);
        uart_data_ready = 1;
        HAL_UART_Receive_IT(&huart1, &receive_buffer1, 1);
    }
}


