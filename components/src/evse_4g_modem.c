/**
 * @file evse_4g_modem.c
 * @author S GOPIKRISHNA
 * @brief 
 * @version 0.1
 * @date 2025-02-08
 * 
 * @copyright Copyright (c) EVRE 2025
 * 
 */
#include "evse_4g_modem.h"

#define TAG             "SIM7600"

static char response[BUF_SIZE]; // Static buffer to avoid dynamic allocation

void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = BAUDRATE,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TX_PIN, RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    ESP_LOGI(TAG, "UART Initialized");
}

void send_at_command(const char *command) {
    uart_write_bytes(UART_NUM, command, strlen(command));
    uart_write_bytes(UART_NUM, "\r\n", 2);
}

bool read_response(int timeout_ms) {
    uint8_t data[BUF_SIZE] = {0};
    int len = uart_read_bytes(UART_NUM, data, BUF_SIZE - 1, timeout_ms / portTICK_PERIOD_MS);
    if (len > 0) {
        data[len] = '\0';
        strcpy(response, (char *)data);
        ESP_LOGI(TAG, "Response: %s", response);
        return true;
    }
    return false;
}

bool send_command_with_retry(const char *command) {
    for (int i = 0; i < MAX_RETRIES; i++) {
        send_at_command(command);
        vTaskDelay(pdMS_TO_TICKS(500));

        if (read_response(1000)) {
            if (strstr(response, "OK") || strstr(response, "+")) {
                return true;
            }
        }
        ESP_LOGW(TAG, "Retrying command: %s (%d/%d)", command, i + 1, MAX_RETRIES);
    }
    ESP_LOGE(TAG, "Command failed after %d attempts: %s", MAX_RETRIES, command);
    return false;
}

esp_err_t reset_module(void) {
    ESP_LOGI(TAG, "Resetting SIM7600...");
    send_at_command("AT+CFUN=1,1");
    vTaskDelay(pdMS_TO_TICKS(5000));

    for (int i = 0; i < 10; i++) {
        send_at_command("AT");
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (read_response(1000) && strstr(response, "OK")) {
            ESP_LOGI(TAG, "Module is ready!");
            return ESP_OK;
        }
    }
    ESP_LOGE(TAG, "Module did not respond after reset.");
    return 2; //reset failed
}

void check_sim7600_mode(void) {
    // Check if the module is in Command Mode
    if (send_command_with_retry("AT")) {
        ESP_LOGI(TAG, "Module is in COMMAND mode");
    } else {
        ESP_LOGW(TAG, "Module might be in DATA mode");
        uart_write_bytes(UART_NUM_1, "+++", 3); // Escape sequence to exit DATA mode
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (send_command_with_retry("AT")) {
            ESP_LOGI(TAG, "Successfully exited DATA mode");
        } else {
            ESP_LOGE(TAG, "Failed to exit DATA mode");
        }
    }
}

void get_4gModule_info(void) {
    // Check the mode first
    check_sim7600_mode();

    // AT Test (Just Checking If Module is Responsive)
    if (send_command_with_retry("AT")) {
        ESP_LOGI(TAG, "Module is responsive");
    }

    // Network Mode
    if (send_command_with_retry("AT+CNMP?")) {
        char network_mode[20] = {0};
        sscanf(response, "+CNMP: %s", network_mode);
        ESP_LOGI(TAG, "Network Mode: %s", network_mode);
    }

    // IMEI
    if (send_command_with_retry("AT+CGSN")) {
        char imei[20] = {0};
        sscanf(response, "%s", imei);
        ESP_LOGI(TAG, "IMEI: %s", imei);
    }

    // IMSI
    if (send_command_with_retry("AT+CIMI")) {
        char imsi[20] = {0};
        sscanf(response, "%s", imsi);
        ESP_LOGI(TAG, "IMSI: %s", imsi);
    }

    // Operator
    if (send_command_with_retry("AT+COPS?")) {
        char operator_name[50] = {0};
        sscanf(response, "+COPS: %*d,%*d,\"%[^\"]\"", operator_name);
        ESP_LOGI(TAG, "Operator: %s", operator_name);
    }

    // Serial Number
    if (send_command_with_retry("AT+GSN")) {
        char serial_number[20] = {0};
        sscanf(response, "%s", serial_number);
        ESP_LOGI(TAG, "Serial Number: %s", serial_number);
    }

    // Signal Strength (RSSI)
    if (send_command_with_retry("AT+CSQ"))
    {
        int rssi = -1; // Default invalid value
        const char *csq_prefix = "+CSQ: ";
        char *csq_pos = strstr(response, csq_prefix);
        
        if (csq_pos) {
            rssi = atoi(csq_pos + strlen(csq_prefix)); // Extract RSSI value
            
            if (rssi == 99) {
                printf("RSSI: Unknown (99)\n");
            } else {
                int rssi_dbm = -113 + (2 * rssi);
                printf("RSSI: %d (Approx %d dBm)\n", rssi, rssi_dbm);
            }
        } else {
            printf("Failed to parse RSSI\n");
        }
    }
}

esp_err_t init_4gModule(void) {
    esp_err_t err = -1;
    uart_init();
    err = reset_module();
    if(err == 2)
    {
        ESP_LOGE(TAG, "Reset Failed");
        return 2;
    }
    vTaskDelay(pdMS_TO_TICKS(2000));
    get_4gModule_info();
return ESP_OK;
}