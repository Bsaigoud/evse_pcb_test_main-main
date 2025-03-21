#ifndef GFCI_H
#define GFCI_H

// #include "driver/gpio.h"
#include <driver/gpio.h>

// Define the GPIO pin to be used for input
#define GFCI_GPIO_PIN GPIO_NUM_34 // Replace with your desired GPIO pin

// Function to initialize the GPIO pin as input
esp_err_t gfci_init(void);

// Function to read the GPIO pin state
int gfci_pin_read(void);

#endif // GPIO_HANDLER_H
