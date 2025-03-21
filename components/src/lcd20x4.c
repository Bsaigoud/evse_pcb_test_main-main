
#include "lcd20x4.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "rom/ets_sys.h"

// extern uint32_t Test_case;

static char tag[] = "LCD Driver";
static uint8_t LCD_addr;
static uint8_t SDA_pin;
static uint8_t SCL_pin;
static uint8_t LCD_cols;
static uint8_t LCD_rows;

static esp_err_t LCD_writeNibble(uint8_t addr, uint8_t nibble, uint8_t mode);
static esp_err_t LCD_writeByte(uint8_t addr, uint8_t data, uint8_t mode);
static esp_err_t LCD_pulseEnable(uint8_t addr, uint8_t nibble);
static esp_err_t I2C_init(void);

// Initialize I2C
static esp_err_t I2C_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_pin,
        .scl_io_num = SCL_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0));
    return ESP_OK;
}

void lcd_deinit(void)
{
    vTaskDelay(3000 / portTICK_PERIOD_MS);

    LCD_printString(LCD_ADDRESS, "                    ", 0, 0);
    LCD_printString(LCD_ADDRESS, "                    ", 0, 1);
    LCD_printString(LCD_ADDRESS, "                    ", 0, 2);
    LCD_printString(LCD_ADDRESS, "                    ", 0, 3);
    i2c_driver_delete(I2C_NUM_0);
}
// Initialize LCD
esp_err_t LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows)
{
    LCD_addr = addr;
    SDA_pin = dataPin;
    SCL_pin = clockPin;
    LCD_cols = cols;
    LCD_rows = rows;

    esp_err_t ret = I2C_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(tag, "I2C initialization failed");
        return ret;
    }

    vTaskDelay(20 / portTICK_PERIOD_MS);

    // Reset the LCD
    if ((ret = LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND)) != ESP_OK)
        return ret;
    vTaskDelay(1 / portTICK_PERIOD_MS);
    if ((ret = LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND)) != ESP_OK)
        return ret;
    ets_delay_us(200);
    if ((ret = LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND)) != ESP_OK)
        return ret;
    if ((ret = LCD_writeNibble(addr, LCD_FUNCTION_SET_4BIT, LCD_COMMAND)) != ESP_OK)
        return ret;
    ets_delay_us(80);

    // Initialize settings
    if ((ret = LCD_writeByte(addr, LCD_FUNCTION_SET_4BIT, LCD_COMMAND)) != ESP_OK)
        return ret;
    ets_delay_us(80);
    if ((ret = LCD_writeByte(addr, LCD_CLEAR, LCD_COMMAND)) != ESP_OK)
        return ret;
    vTaskDelay(2 / portTICK_PERIOD_MS);
    if ((ret = LCD_writeByte(addr, LCD_ENTRY_MODE, LCD_COMMAND)) != ESP_OK)
        return ret;
    ets_delay_us(80);
    if ((ret = LCD_writeByte(addr, LCD_DISPLAY_ON, LCD_COMMAND)) != ESP_OK)
        return ret;

    ESP_LOGI(tag, "LCD initialized successfully");
    return ESP_OK;
}

// Write a Nibble
static esp_err_t LCD_writeNibble(uint8_t addr, uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    if (ret != ESP_OK)
        return ret;
    return LCD_pulseEnable(addr, data);
}

// Write a Byte
static esp_err_t LCD_writeByte(uint8_t addr, uint8_t data, uint8_t mode)
{
    esp_err_t ret;
    if ((ret = LCD_writeNibble(addr, data & 0xF0, mode)) != ESP_OK)
        return ret;
    return LCD_writeNibble(addr, (data << 4) & 0xF0, mode);
}

// Pulse Enable
static esp_err_t LCD_pulseEnable(uint8_t addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data | LCD_ENABLE, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    if (ret != ESP_OK)
        return ret;

    ets_delay_us(1);
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, data & ~LCD_ENABLE, true);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    ets_delay_us(500);
    return ret;
}

#if 0
// LCD Task
void LCD_task(void *params) {
  

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        // togglewatchdog();
        // Lcd_state();
    }
}
#endif

// Start LCD Task
esp_err_t init_lcd(void)
{
    esp_err_t ret = LCD_init(LCD_ADDRESS, DISPLAY_TX_PIN, DISPLAY_RX_PIN, 20, 4);
    if (ret != ESP_OK)
    {
        ESP_LOGE(tag, "Failed to initialize LCD");
        return ret;
    }

    LCD_home(LCD_ADDRESS);
    LCD_clearScreen(LCD_ADDRESS);
    // LCD_printString(LCD_ADDRESS, "ESP32 is ready", 0, 1);  // Print on the second line

    //  xTaskCreate(LCD_task, "LCD_task", 2048, NULL, 2, NULL);
    return ESP_OK;
}

void LCD_clearScreen(uint8_t addr)
{
    LCD_writeByte(addr, LCD_CLEAR, LCD_COMMAND);
    vTaskDelay(200); // This command takes a while to complete
}
void LCD_printString(uint8_t addr, const char *str, uint8_t col, uint8_t row)
{
    // Set the cursor position before printing
    LCD_setCursor(addr, col, row);

    // Write the string to the LCD
    LCD_writeStr(addr, str);
}
void LCD_writeStr(uint8_t addr, char *str)
{
    while (*str)
    {
        LCD_writeChar(addr, *str++);
    }
}
void LCD_setCursor(uint8_t addr, uint8_t col, uint8_t row)
{
    if (row > LCD_rows - 1)
    {
        // ESP_LOGE(tag, "Cannot write to row %d. Please select a row in the range (0, %d)", row, LCD_rows-1);
        row = LCD_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};
    LCD_writeByte(addr, LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

void LCD_writeChar(uint8_t addr, char c)
{
    LCD_writeByte(addr, c, LCD_WRITE); // Write data to DDRAM
}
void LCD_home(uint8_t addr)
{
    LCD_writeByte(addr, LCD_HOME, LCD_COMMAND);
    vTaskDelay(200); // This command takes a while to complete
}

#if 0

#include "lcd20x4.h"
#include <driver/i2c.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include "sdkconfig.h"
#include "rom/ets_sys.h"
#include <esp_log.h>
#include "PCB_TEST.h"
extern  uint32_t Test_case;

static char tag[] = "LCD Driver";
static uint8_t LCD_addr;
static uint8_t SDA_pin;
static uint8_t SCL_pin;
static uint8_t LCD_cols;
static uint8_t LCD_rows;

static void LCD_writeNibble(uint8_t addr, uint8_t nibble, uint8_t mode);
static void LCD_writeByte(uint8_t addr, uint8_t data, uint8_t mode);
static void LCD_pulseEnable(uint8_t addr, uint8_t nibble);
static esp_err_t I2C_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_pin,
        .scl_io_num = SCL_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000};
    i2c_param_config(I2C_NUM_0, &conf);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0);
    return ESP_OK;
}

void LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows)
{
    LCD_addr = addr;
    SDA_pin = dataPin;
    SCL_pin = clockPin;
    LCD_cols = cols;
    LCD_rows = rows;
    I2C_init();
    vTaskDelay(20 / portTICK_PERIOD_MS); // Initial 40 mSec delay

    // Reset the LCD controller
    LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND);    // First part of reset sequence
    vTaskDelay(1 / portTICK_PERIOD_MS);                        // 4.1 mS delay (min)
    LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND);    // second part of reset sequence
    ets_delay_us(200);                                         // 100 uS delay (min)
    LCD_writeNibble(addr, LCD_FUNCTION_RESET, LCD_COMMAND);    // Third time's a charm
    LCD_writeNibble(addr, LCD_FUNCTION_SET_4BIT, LCD_COMMAND); // Activate 4-bit mode
    ets_delay_us(80);                                          // 40 uS delay (min)

    // --- Busy flag now available ---
    // Function Set instruction
    LCD_writeByte(addr, LCD_FUNCTION_SET_4BIT, LCD_COMMAND); // Set mode, lines, and font
    ets_delay_us(80);

    // Clear Display instruction
    LCD_writeByte(addr, LCD_CLEAR, LCD_COMMAND); // clear display RAM
    vTaskDelay(2 / portTICK_PERIOD_MS);          // Clearing memory takes a bit longer

    // Entry Mode Set instruction
    LCD_writeByte(addr, LCD_ENTRY_MODE, LCD_COMMAND); // Set desired shift characteristics
    ets_delay_us(80);

    LCD_writeByte(addr, LCD_DISPLAY_ON, LCD_COMMAND); // Ensure LCD is set to on
}

void LCD_setCursor(uint8_t addr, uint8_t col, uint8_t row)
{
    if (row > LCD_rows - 1)
    {
        // ESP_LOGE(tag, "Cannot write to row %d. Please select a row in the range (0, %d)", row, LCD_rows-1);
        row = LCD_rows - 1;
    }
    uint8_t row_offsets[] = {LCD_LINEONE, LCD_LINETWO, LCD_LINETHREE, LCD_LINEFOUR};
    LCD_writeByte(addr, LCD_SET_DDRAM_ADDR | (col + row_offsets[row]), LCD_COMMAND);
}

void LCD_writeChar(uint8_t addr, char c)
{
    LCD_writeByte(addr, c, LCD_WRITE); // Write data to DDRAM
}

void LCD_writeStr(uint8_t addr, char *str)
{
    while (*str)
    {
        LCD_writeChar(addr, *str++);
    }
}

void LCD_home(uint8_t addr)
{
    LCD_writeByte(addr, LCD_HOME, LCD_COMMAND);
    vTaskDelay(200); // This command takes a while to complete
}

void LCD_clearScreen(uint8_t addr)
{
    LCD_writeByte(addr, LCD_CLEAR, LCD_COMMAND);
    vTaskDelay(200); // This command takes a while to complete
}
void LCD_printString(uint8_t addr, const char *str, uint8_t col, uint8_t row)
{
    // Set the cursor position before printing
    LCD_setCursor(addr, col, row);
    
    // Write the string to the LCD
    LCD_writeStr(addr, str);
}


static void LCD_writeNibble(uint8_t addr, uint8_t nibble, uint8_t mode)
{
    uint8_t data = (nibble & 0xF0) | mode | LCD_BACKLIGHT;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, data, 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);

    LCD_pulseEnable(addr, data); // Clock data into LCD
}

static void LCD_writeByte(uint8_t addr, uint8_t data, uint8_t mode)
{
    LCD_writeNibble(addr, data & 0xF0, mode);
    LCD_writeNibble(addr, (data << 4) & 0xF0, mode);
}

static void LCD_pulseEnable(uint8_t addr, uint8_t data)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, data | LCD_ENABLE, 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    ets_delay_us(1);

    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(cmd, (data & ~LCD_ENABLE), 1);
    i2c_master_stop(cmd);
    i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    ets_delay_us(500);
}
void Lcd_state(void)
{
    switch (Test_case)
    {
    case WI_FI:
      LCD_printString(LCD_ADDRESS, "PCB TEST EVRE", 0, 2);
      break;

    case ENERGY_METER:
    LCD_printString(LCD_ADDRESS, "ENERGY_METER PASS", 0, 2);
    break;

    case RFID:
    LCD_printString(LCD_ADDRESS, "RFID TEST PASS", 0, 2);
    break;

    case RELAY:
        break;

    case LED:
        break;
    case BUZZER_TEST:
        break;
    case EARTH_DIS:
        break;
    case EMERGENCY_TEST:
        break;
    case GFCI:
        break;
    case RTC:
        break;
    case CONTROL_PILET_TEST:
        break;
    case GSM:
        break;
    case EATHERNET:
        break;
    default:
        break;
    }


}

void togglewatchdog(void)
{
    static uint8_t state = 0;

    gpio_set_level(WATCH_DOG_PIN, state);

    if (state == 0)
    {
        state = 1;
    }

    else if (state == 1)
    {
        state = 0;
    }
}


void LCD_task(void* params)
{
     LCD_init(LCD_ADDRESS, DISPLAY_TX_PIN, DISPLAY_RX_PIN, 20, 4);
    LCD_home(LCD_ADDRESS);
    LCD_clearScreen(LCD_ADDRESS);
    gpio_set_direction(WATCH_DOG_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(WATCH_DOG_PIN);
    gpio_pullup_dis(WATCH_DOG_PIN);

  
    while (1)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));

    }
    

}
void  LCD_initialazation()
{

    xTaskCreate(LCD_task, "LCD_task", 2048, NULL, 2, NULL);
    
}

#endif
