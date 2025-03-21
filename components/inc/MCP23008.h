#ifndef MCP23008_H
#define MCP23008_H

#include "esp_err.h"
#include "driver/i2c.h"
#include <stdbool.h> // Include for bool type
#include "driver/i2c.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define I2C_MASTER_SCL_IO    22
#define I2C_MASTER_SDA_IO    21
#define I2C_MASTER_NUM       I2C_NUM_0
#define I2C_MASTER_FREQ_HZ   100000
#define BUZZER 4
#define EMERGENCY 2
#define EARTH_PRESENCE 3
#define RELAY_DROP 6
#define BATTERY_CONTROLER 5
#define HIGH 1
#define LOW 0

// MCP23008 I2C address
#define MCP23008_I2C_ADDR 0x27

// MCP23008 Register Definitions
#define MCP23008_IODIR   0x00
#define MCP23008_GPIO    0x09
#define MCP23008_OLAT    0x0A

// Function Prototypes
esp_err_t mcp23008_init(i2c_port_t i2c_num, uint8_t addr);
esp_err_t mcp23008_set_pin_direction(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool is_output);
esp_err_t mcp23008_write_pin(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool level);
esp_err_t mcp23008_read_pin(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool* level);
void mcp23008IO_task(void* params);
esp_err_t IO_init(void);

#endif // MCP23008_H
