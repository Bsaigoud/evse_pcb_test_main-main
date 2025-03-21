#pragma once

#include <esp_log.h>
#include "rc522.h"
#include "rc522_spi.h"
#include "rc522_picc.h"

#define RC522_SPI_BUS_GPIO_MISO (12)
#define RC522_SPI_BUS_GPIO_MOSI (13)
#define RC522_SPI_BUS_GPIO_SCLK (14)
#define RC522_SPI_SCANNER_GPIO_SDA (0)
#define RC522_SCANNER_GPIO_RST (-1) // soft-reset

esp_err_t RC522_Init(void);
esp_err_t RC522_Deinit(void);

extern volatile bool is_rfid_card_present;