#include "GFCI.h"
#include "esp_log.h"

static const char* TAG = "gfci";

// Initialize the GPIO pin as input

esp_err_t gfci_init(void) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GFCI_GPIO_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_down_en = GPIO_PULLDOWN_ENABLE, // Enable pull-down resistor if needed
        .pull_up_en = GPIO_PULLUP_DISABLE,    // Disable pull-up resistor
        .intr_type = GPIO_INTR_DISABLE        // No interrupt required
    };

    esp_err_t ret = gpio_config(&io_conf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "GFCI pin initialized successfully  .");
    } else {
        ESP_LOGE(TAG, "GFCI pin initialization failed: %d", ret);
    }
    return ret;
}


// Read the GPIO pin state
int gfci_pin_read(void) {
    return gpio_get_level(GFCI_GPIO_PIN); // Returns 0 or 1
}
