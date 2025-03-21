#ifndef LCD20X4_H
#define LCD20X4_H

#include <stdint.h>
#include <esp_err.h>
#include <driver/gpio.h>

// LCD I2C Address and Pin Definitions
#define LCD_ADDRESS 0x26
#define DISPLAY_TX_PIN 26
#define DISPLAY_RX_PIN 27
#define COLUMNS 20
#define ROWS 4

// Watchdog Pin Definition
#define WATCH_DOG_PIN 15

// LCD Commands and Instructions
#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_COMMAND 0x00
#define LCD_WRITE 0x01

#define LCD_FUNCTION_RESET 0x30
#define LCD_FUNCTION_SET_4BIT 0x28
#define LCD_ENTRY_MODE 0x06
#define LCD_DISPLAY_ON 0x0C
#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_SET_CURSOR 0x80

// Function Declarations
esp_err_t LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows);
void LCD_task(void *params);
esp_err_t init_lcd(void);
void LCD_setCursor(uint8_t addr, uint8_t col, uint8_t row);
void LCD_home(uint8_t addr);
void LCD_clearScreen(uint8_t addr);
void LCD_writeChar(uint8_t addr, char c);
void LCD_writeStr(uint8_t addr, char* str);
void LCD_printString(uint8_t addr, const char* str, uint8_t col, uint8_t row);
void togglewatchdog(void);
void Lcd_state(void);

void lcd_deinit(void);
// Internal helper functions
static esp_err_t I2C_init(void);
static esp_err_t LCD_writeByte(uint8_t addr, uint8_t data, uint8_t mode);
static esp_err_t LCD_writeNibble(uint8_t addr, uint8_t nibble, uint8_t mode);
static esp_err_t LCD_pulseEnable(uint8_t addr, uint8_t data);
// LCD module defines
#define LCD_LINEONE 0x00   // start of line 1
#define LCD_LINETWO 0x40   // start of line 2
#define LCD_LINETHREE 0x14 // start of line 3
#define LCD_LINEFOUR 0x54  // start of line 4

#define LCD_SET_DDRAM_ADDR 0x80
#define LCD_READ_BF 0x40

#endif // LCD20X4_H









#if 0
#ifndef LCD20X4_H
#define LCD20X4_H

#include <stdint.h>
#define LCD_ADDRESS 0x26
#define DISPLAY_TX_PIN 26
#define DISPLAY_RX_PIN 27
#define COLUMS 20
#define ROWS 4

#define WATCH_DOG_PIN 15

// LCD module defines
#define LCD_LINEONE 0x00   // start of line 1
#define LCD_LINETWO 0x40   // start of line 2
#define LCD_LINETHREE 0x14 // start of line 3
#define LCD_LINEFOUR 0x54  // start of line 4

#define LCD_BACKLIGHT 0x08
#define LCD_ENABLE 0x04
#define LCD_COMMAND 0x00
#define LCD_WRITE 0x01

#define LCD_SET_DDRAM_ADDR 0x80
#define LCD_READ_BF 0x40

// LCD instructions
#define LCD_CLEAR 0x01             // replace all characters with ASCII 'space'
#define LCD_HOME 0x02              // return cursor to first position on first line
#define LCD_ENTRY_MODE 0x06        // shift cursor from left to right on read/write
#define LCD_DISPLAY_OFF 0x08       // turn display off
#define LCD_DISPLAY_ON 0x0C        // display on, cursor off, don't blink character
#define LCD_FUNCTION_RESET 0x30    // reset the LCD
#define LCD_FUNCTION_SET_4BIT 0x28 // 4-bit data, 2-line display, 5 x 7 font
#define LCD_SET_CURSOR 0x80        // set cursor position

void LCD_init(uint8_t addr, uint8_t dataPin, uint8_t clockPin, uint8_t cols, uint8_t rows);
void LCD_setCursor(uint8_t addr, uint8_t col, uint8_t row);
void LCD_home(uint8_t addr);
void LCD_clearScreen(uint8_t addr);
void LCD_writeChar(uint8_t addr, char c);
void LCD_writeStr(uint8_t addr, char* str);
void LCD_printString(uint8_t addr, const char* str, uint8_t col, uint8_t row);
void LCD_task(void* pvParameter);
void  LCD_initialazation();
void togglewatchdog(void);
void Lcd_state(void);




#endif 
#endif
