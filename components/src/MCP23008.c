#include "mcp23008.h"
static TaskHandle_t mcp23008_task_handle = NULL;

// Helper function to write to a register
static esp_err_t mcp23008_write_register(i2c_port_t i2c_num, uint8_t addr, uint8_t reg, uint8_t data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Helper function to read from a register
static esp_err_t mcp23008_read_register(i2c_port_t i2c_num, uint8_t addr, uint8_t reg, uint8_t *data) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, data, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_num, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

// Initialize MCP23008 (default: all pins input)
esp_err_t mcp23008_init(i2c_port_t i2c_num, uint8_t addr) {
    return mcp23008_write_register(i2c_num, addr, MCP23008_IODIR, 0xFF);
}

// Set pin direction (input/output)
esp_err_t mcp23008_set_pin_direction(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool is_output) {
    uint8_t iodir;
    esp_err_t ret = mcp23008_read_register(i2c_num, addr, MCP23008_IODIR, &iodir);
    if (ret != ESP_OK) return ret;

    if (is_output) {
        iodir &= ~(1 << pin);
    } else {
        iodir |= (1 << pin);
    }

    return mcp23008_write_register(i2c_num, addr, MCP23008_IODIR, iodir);
}

// Write to a pin (high/low)
esp_err_t mcp23008_write_pin(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool level) {
    uint8_t olat;
    esp_err_t ret = mcp23008_read_register(i2c_num, addr, MCP23008_OLAT, &olat);
    if (ret != ESP_OK) return ret;

    if (level) {
        olat |= (1 << pin);
    } else {
        olat &= ~(1 << pin);
    }

    return mcp23008_write_register(i2c_num, addr, MCP23008_OLAT, olat);
}

// Read a pin state
esp_err_t mcp23008_read_pin(i2c_port_t i2c_num, uint8_t addr, uint8_t pin, bool *level) {
    uint8_t gpio;
    esp_err_t ret = mcp23008_read_register(i2c_num, addr, MCP23008_GPIO, &gpio);
    if (ret != ESP_OK) return ret;

    *level = (gpio >> pin) & 0x01;
    return ESP_OK;
}
void mcp23008_deinit(void)
{
    vTaskDelete(mcp23008_task_handle);
}
#if 0
void mcp23008IO_task(void* params)
{
   


    while (1)
    {
        // Delay for a while (e.g., 1 second)
        vTaskDelay(pdMS_TO_TICKS(1000));

    }
    



}

#endif

esp_err_t IO_init(void)
{
    esp_err_t ret;  
    // Configure I2C
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };
    i2c_param_config(I2C_MASTER_NUM, &i2c_config);
    i2c_driver_install(I2C_MASTER_NUM, i2c_config.mode, 0, 0, 0);

    // Initialize MCP23008
     ret = mcp23008_init(I2C_MASTER_NUM, MCP23008_I2C_ADDR);
    if (ret != ESP_OK) {
        printf("Failed to initialize MCP23008: %s\n", esp_err_to_name(ret));
        return ret;
    }

    // Configure pins: 2 and 3 as input, 4 and 6 as output
    mcp23008_set_pin_direction(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EMERGENCY, false); // Pin 2 as input
    mcp23008_set_pin_direction(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EARTH_PRESENCE, false); // Pin 3 as input
    mcp23008_set_pin_direction(I2C_MASTER_NUM, MCP23008_I2C_ADDR, BUZZER, true);  // Pin 4 as output
    mcp23008_set_pin_direction(I2C_MASTER_NUM, MCP23008_I2C_ADDR, RELAY_DROP, true);  // Pin 6 as output
    mcp23008_set_pin_direction(I2C_MASTER_NUM, MCP23008_I2C_ADDR, BATTERY_CONTROLER, true); 


    // xTaskCreate(mcp23008IO_task, "mcp23008IO_task", 2048, NULL, 2, &mcp23008_task_handle);
    return ESP_OK;
}
