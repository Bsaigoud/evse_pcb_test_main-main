#ifndef LED_STRIP_H
#define LED_STRIP_H

#include <stdint.h>
#include "esp_err.h"
#include "led_strip_encoder.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt_tx.h"
#include "esp_log.h"
// #include "ProjectSettings.h"

#define RMT_LED_STRIP_RESOLUTION_HZ 10000000 // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define BLINK_TIME 500
#define RMT_LED_STRIP_GPIO_V5_PIN 25
#define EXAMPLE_LED_NUMBERS 10
#define TASK_DELAY_TIME 1000
#define NUM_OF_CONNECTORS 4
#define AVAILABLE_LED SOLID_GREEN
#define EMERGENCY_FAULT_LED BLINK_RED
#define GFCI_FAULT_LED SOLID_RED
#define EARTH_DISCONNECT_FAULT_LED SOLID_RED
#define POWER_LOSS_FAULT_LED SOLID_RED
#define OVER_VOLTAGE_FAULT_LED SOLID_RED
#define UNDER_VOLTAGE_FAULT_LED SOLID_RED
#define OVER_CURRENT_FAULT_LED SOLID_RED
#define HIGH_TEMP_FAULT_LED SOLID_RED
#define METER_FAILURE_LED SOLID_RED
#define PREPARING_LED SOLID_BLUE
#define OFFLINE_PREPARING_LED SOLID_BLUE_BLINK_WHITE
#define CHARGING_LED BLINK_GREEN
#define UNAVAILABLE_LED SOLID_BLUE
#define OFFLINE_AVAILABLE_LED SOLID_GREEN_BLINK_WHITE
#define OTA_LED SOLID_ORANGE
#define FINISHING_LED SOLID_BLUE
#define OFFLINE_FINISHING_LED SOLID_BLUE_BLINK_WHITE
#define COMMISSIONING_LED SOLID_ORANGE
#define RESERVATION_LED SOLID_BLUE
#define OFFLINE_RESERVATION_LED SOLID_BLUE_BLINK_WHITE
#define OFFLINE_CHARGING_LED BLINK_GREEN_WHITE
#define OFFLINE_UNAVAILABLE_LED BLINK_WHITE
#define RFID_TAPPED_LED BLINK_BLUE
#define SUSPENDED_LED BLINK_BLUE
#define OFFLINE_SUSPENDED_LED BLINK_BLUE_WHITE

#define OFFLINE_ONLY_AVAILABLE_LED SOLID_GREEN
#define OFFLINE_ONLY_PREPARING_LED SOLID_BLUE
#define OFFLINE_ONLY_CHARGING_LED BLINK_GREEN
#define OFFLINE_ONLY_FINISHING_LED SOLID_BLUE
#define OFFLINE_ONLY_SUSPENDED_LED BLINK_BLUE
// void LED_Strip(void);
esp_err_t init_led_Strip(void);

void led_strip_deinit(void);

typedef enum
{
    SOLID_BLACK = 0,         // stable Black
    SOLID_GREEN,             // stable Green
    BLINK_RED,               // blinking Red
    SOLID_RED,               // stable Red
    BLINK_BLUE,              // blinking Blue
    SOLID_BLUE,              // stable Blue
    BLINK_GREEN,             // blinking Green
    BLINK_GREEN_WHITE,       // blinking Green 3 seconds and blinking White 2 seconds
    BLINK_ORANGE,            // blinking Orange
    SOLID_ORANGE,            // stable Orange
    BLINK_WHITE,             // blinking White
    SOLID_GREEN_BLINK_WHITE, // stable Green 3 seconds and blinking White 2 seconds
    SOLID_BLUE_BLINK_WHITE,  // stable Blue 3 seconds and blinking White 2 seconds
    BLINK_BLUE_WHITE         // blinking Blue 3 seconds and blinking White 2 seconds
} ledColors_t;

extern uint8_t ledId;
extern ledColors_t ledStateColor[NUM_OF_CONNECTORS];
#endif