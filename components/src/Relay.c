#include "Relay.h"
#include <esp_err.h>

static char tag[] = "Relay";

esp_err_t relay_set(int state)
{
    // gpio_set_level(RELAY_GPIO_PIN, state);
    esp_err_t ret = gpio_set_level(RELAY_GPIO_PIN, state);
    if (ret != ESP_OK)
    {
        // Handle error
        printf("Error setting GPIO level: %d\n", ret);
    }
    else
    {
        // GPIO level set successfully
        printf("GPIO level set \"%d\" successful\n", state);
        // Test_case = 6;
    }
return ret;
}
void relay_deinit(void)
{
    //   relay_set(0);
    gpio_reset_pin(RELAY_GPIO_PIN);
}

esp_err_t Relay_init(void)
{
    // Configure the GPIO pin for relay as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RELAY_GPIO_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE};

    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK)
    {
        return ret; // Return the error code if configuration fails

        ESP_LOGE(tag, "configuration is Fail");
    }
    else
    {

        ESP_LOGI(tag, "configuration is successful");
    }

    return ESP_OK; // Return ESP_OK if configuration is successful
}