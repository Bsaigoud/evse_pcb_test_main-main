#ifndef _RELAY_H
#define _RELAY_H
#include "esp_log.h"
#include "driver/gpio.h"

#define RELAY_GPIO_PIN GPIO_NUM_33  


esp_err_t Relay_init(void);
void relay_deinit(void);
esp_err_t relay_set(int);

#endif