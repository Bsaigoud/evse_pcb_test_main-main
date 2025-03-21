/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#include "led_strip.h"
// #include "ocpp_task.h"
// #include "custom_nvs.h"
uint8_t ledId = 1;
// extern uint16_t Test_case;
static TaskHandle_t led_stripHandler = NULL;

// extern ConnectorPrivateData_t ConnectorData[NUM_OF_CONNECTORS];
ledColors_t ledStateColor[NUM_OF_CONNECTORS] = {SOLID_BLACK};
ledColors_t ledStateColor_old[NUM_OF_CONNECTORS];
ledColors_t ledColors;
uint8_t led_loopCount[NUM_OF_CONNECTORS];

static const char *TAG = "LED_STRIP";

static uint8_t led_strip_pixels[EXAMPLE_LED_NUMBERS * 3];
rmt_channel_handle_t led_chan = NULL;
rmt_tx_channel_config_t tx_chan_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT, // select source clock
    .gpio_num = RMT_LED_STRIP_GPIO_V5_PIN,
    .mem_block_symbols = 64, // increase the block size can make the LED less flickering
    .resolution_hz = RMT_LED_STRIP_RESOLUTION_HZ,
    .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
};
rmt_encoder_handle_t led_encoder = NULL;
led_strip_encoder_config_t encoder_config = {
    .resolution = RMT_LED_STRIP_RESOLUTION_HZ,
};
rmt_transmit_config_t tx_config = {
    .loop_count = 0, // no transfer loop
};

void setLEDColor(uint8_t red, uint8_t green, uint8_t blue)
{
    for (uint8_t j = 0; j < EXAMPLE_LED_NUMBERS; j++)
    {
        led_strip_pixels[(j * 3) + 0] = green;
        led_strip_pixels[(j * 3) + 1] = red;
        led_strip_pixels[(j * 3) + 2] = blue;
    }
    ESP_ERROR_CHECK(rmt_transmit(led_chan, led_encoder, led_strip_pixels, sizeof(led_strip_pixels), &tx_config));
    ESP_ERROR_CHECK(rmt_tx_wait_all_done(led_chan, portMAX_DELAY));
    // Test_case = 7;
}
void led_strip_deinit(void)
{
    // ledStateColor[ledId] = SOLID_BLACK;
    vTaskDelete(led_stripHandler);
    vTaskDelay(25 / portTICK_PERIOD_MS);
    setLEDColor(0, 0, 0);
}

void LedTask1(void *params)
{
    // uint8_t ledId = 1;
    // custom_nvs_read_connector_data(1);
    // if (ConnectorData[ledId].CmsAvailable == false)
    // if (false)
    // {
        // ledStateColor[ledId] = OFFLINE_UNAVAILABLE_LED;
    // }
    // else
    // {
    //     ledStateColor[ledId] = OFFLINE_AVAILABLE_LED;
    // }
    while (true)
    {
          printf("Running led_Task task\n");

        if (ledStateColor[ledId] != ledStateColor_old[ledId])
        {
            led_loopCount[ledId] = 0;
        }
        switch (ledStateColor[ledId])
        {
        case SOLID_BLACK:
            setLEDColor(0, 0, 0);
            break;
        case SOLID_GREEN:
            setLEDColor(0, 255, 0);
            break;
        case BLINK_RED:
            if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 0, 0);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case SOLID_RED:
            setLEDColor(255, 0, 0);
            break;
        case BLINK_BLUE:
            if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(0, 0, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case SOLID_BLUE:
            setLEDColor(0, 0, 255);
            break;
        case BLINK_GREEN:
            if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(0, 255, 0);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case BLINK_GREEN_WHITE:
            if (((led_loopCount[ledId] % 10) < 6) && (led_loopCount[ledId] % 2 == 0))
            {
                setLEDColor(0, 255, 0);
            }
            else if (((led_loopCount[ledId] % 10) < 6) && (led_loopCount[ledId] % 2 == 1))
            {
                setLEDColor(0, 0, 0);
            }
            else if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case BLINK_ORANGE:
            if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 0);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case SOLID_ORANGE:
            setLEDColor(255, 255, 0);
            break;
        case BLINK_WHITE:
            if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case SOLID_GREEN_BLINK_WHITE:
            if ((led_loopCount[ledId] % 10) < 6)
            {
                setLEDColor(0, 255, 0);
            }
            else if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case SOLID_BLUE_BLINK_WHITE:
            if ((led_loopCount[ledId] % 10) < 6)
            {
                setLEDColor(0, 0, 255);
            }
            else if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        case BLINK_BLUE_WHITE:
            if (((led_loopCount[ledId] % 10) < 6) && (led_loopCount[ledId] % 2 == 0))
            {
                setLEDColor(0, 0, 255);
            }
            else if (((led_loopCount[ledId] % 10) < 6) && (led_loopCount[ledId] % 2 == 1))
            {
                setLEDColor(0, 0, 0);
            }
            else if (led_loopCount[ledId] % 2 == 0)
            {
                setLEDColor(255, 255, 255);
            }
            else
            {
                setLEDColor(0, 0, 0);
            }
            break;
        default:
            setLEDColor(0, 0, 0);
            break;
        }
        ledStateColor_old[ledId] = ledStateColor[ledId];
        led_loopCount[ledId]++;
        vTaskDelay(BLINK_TIME / portTICK_PERIOD_MS);
    }
}

esp_err_t init_led_Strip(void)
{
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &led_chan));
    esp_err_t ret = rmt_new_led_strip_encoder(&encoder_config, &led_encoder);
    if (ret != ESP_OK) {
        return ret;  // Return the error code if configuration fails
      
        ESP_LOGE(TAG, "configuration is Fail");
    }
    else
    {
       
         ESP_LOGI(TAG, "configuration is successful");
    }
    ESP_ERROR_CHECK(rmt_enable(led_chan));
    xTaskCreate(&LedTask1, "LED_TASK1", 4096, "task led1", 3, &led_stripHandler);
    return ESP_OK;
}
