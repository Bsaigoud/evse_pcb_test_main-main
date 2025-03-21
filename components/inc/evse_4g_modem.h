/**
 * @file evse_4g_modem.h
 * @author S GOPIKRISHNA
 * @brief 
 * @version 0.1
 * @date 2025-02-08
 * 
 * @copyright Copyright (c) EVRE 2025
 * 
 */
#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_system.h"
#include <string.h>
#include <stdio.h>

#define UART_NUM        UART_NUM_1
#define TX_PIN          GPIO_NUM_17
#define RX_PIN          GPIO_NUM_16
#define BAUDRATE        115200
#define BUF_SIZE        1024
#define MAX_RETRIES     5

// Stack protection
#define CHECK_STACK() do { \
    uint32_t stack_size = uxTaskGetStackHighWaterMark(NULL); \
    if (stack_size < 200) { \
        ESP_LOGW(TAG, "Low stack size: %lu", stack_size); \
    } \
} while(0)

esp_err_t init_4gModule(void);