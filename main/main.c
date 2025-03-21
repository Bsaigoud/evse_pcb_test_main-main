#include "inc/evse_websocket.h"
#include "inc/protocol_examples_common.h"
#include "inc/EnergyMeter.h"
#include "inc/lcd20x4.h"
#include "inc/Relay.h"
#include "inc/led_strip.h"
#include "inc/MCP23008.h"
#include "inc/RFID_READ.h"
#include "inc/GFCI.h"
#include "inc/evse_4g_modem.h"
// #include <string.h>

static const char *TAG = "PCB Test";

void wifi_connection_chek(void *arg)
{
    while (1)
    {
        if (!isWifiConnected)
        {
            esp_wifi_connect();
        }
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

#if PING_CHECK_ENABLE
uint8_t ping_check(void);
esp_err_t check_connectivity(const char *host);
#endif

#if WEBSOCKET_ENABLE
#include "inc/evse_websocket.h"
#endif

#define WATCH_DOG_PIN 15
void test_4G(void);
esp_err_t test_ethernet(void);
void test_wifi(void);

void watchdog_task(void *args)
{
    gpio_set_direction(WATCH_DOG_PIN, GPIO_MODE_OUTPUT);
    gpio_pulldown_dis(WATCH_DOG_PIN);
    gpio_pullup_dis(WATCH_DOG_PIN);
    static uint8_t state = 0;
    ESP_LOGI(TAG, "Watchdog initialized");

    while (1)
    {
        gpio_set_level(WATCH_DOG_PIN, state);

        if (state == 0)
        {
            state = 1;
        }

        else if (state == 1)
        {
            state = 0;
        }
        vTaskDelay(30000 / portTICK_PERIOD_MS);
    }
}

#define TEST_PASS (1)
#define TEST_FAIL (0)

void handle_test_value(const char *testStr);

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskCreate(&watchdog_task, "watchdog toggle", 2048, "wtc", 5, NULL);

#if WIFI_ENABLE
    ESP_LOGI(TAG, "Establsihing Network connection");
    connect_wifi();
    xTaskCreate(&wifi_connection_chek, "wifi connection check", 4096, "sta", 5, NULL);
#else
    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // test_4G();
    // test_ethernet();
    test_wifi();
    xTaskCreate(&wifi_connection_chek, "wifi connection check", 2048, "sta", 5, NULL);
    // ESP_ERROR_CHECK(example_connect());
    // ESP_ERROR_CHECK(example_connect());
#endif

#if PING_CHECK_ENABLE
    while (1)
    {
        check_connectivity("8.8.8.8");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
#endif

#if WEBSOCKET_ENABLE
    // if (isWifiConnected)
    // {
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("websocket_client", ESP_LOG_DEBUG);
    esp_log_level_set("transport_ws", ESP_LOG_DEBUG);
    esp_log_level_set("trans_tcp", ESP_LOG_DEBUG);

    init_websocket();
    // }
    int loopCount = 0;
    while (1)
    {

        if (esp_websocket_client_is_connected(client))
        {
            if (json_data_recieved)
            {
                json_data_recieved = 0;
                if (!cJSON_IsString(testValue))
                {
                    ESP_LOGE("Web-Socket", "Invalid or missing 'Test' key");
                    cJSON_Delete(json);
                    continue;
                }

                // ESP_LOGI(TAG, "\r\nReceived Test value: %s", testValue->valuestring);

                handle_test_value(testValue->valuestring);

                // cJSON_Delete(testValue);
                cJSON_Delete(json);
                testValue = NULL;
                json = NULL;
            }
        }

        // if((loopCount++ %100) == 0)
        // {
        //     ESP_LOGW(TAG, "Free heap size: %d bytes", heap_caps_get_free_size(MALLOC_CAP_DEFAULT));
        // }
        vTaskDelay(25 / portTICK_PERIOD_MS);
    }

#endif
}

// Function to handle specific test values
void handle_test_value(const char *testStr)
{
    char *response_Json_str = NULL;
    cJSON *response_Json = NULL;
    uint8_t test_Res = -1;
    esp_err_t err = -1;
    uint8_t test_count = 0;

    if (strcmp(testStr, "WiFi_Connectivity") == 0)
    {
        ESP_LOGI(TAG, "WI-FI connectivity check");

#if PING_CHECK_ENABLE
        test_Res = ping_check();
#else
        test_Res = TEST_PASS;
#endif
        // response_Json_str = "{\"Test\": \"WI-FI_connectivity_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "WiFi_Connectivity");

        if (test_Res == TEST_PASS)
        {
            cJSON_AddStringToObject(response_Json, "status", "pass");
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Energy_Meter_Relay") == 0)
    {
        ESP_LOGI(TAG, "Energy Meter Relay");

        // response_Json_str = "{\"Test\": \"Energy_Meter_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Energy_Meter_Relay");

        // Initialize the Energy Meter
        err = Energy_meter_init();
        Relay_init();
        relay_set(1);
        // while (!err)
        // {
        //     relay_set(1);
        //     if (Em_Test_case == 1)
        //     {
        //         test_Res = 1;
        //         break;
        //     }
        //     vTaskDelay(1000 / portTICK_PERIOD_MS);
        // }
        // Deinitialize the Energy Meter
        // Energy_meter_deinit();

        // if (test_Res == TEST_PASS)
        // {
        //     cJSON_AddStringToObject(response_Json, "status", "pass");
        // }
        // else
        // {
        //     cJSON_AddStringToObject(response_Json, "status", "fail");
        // }
        cJSON_AddStringToObject(response_Json, "status", "pass");
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
        // relay_set(0);
    }
    else if (strcmp(testStr, "LCD_Test") == 0)
    {
        ESP_LOGI(TAG, "Display_check");

        // response_Json_str = "{\"Test\": \"Display_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "LCD_Test");

        err = init_lcd();

        if (!err)
        {
            LCD_printString(LCD_ADDRESS, "1234567890123456789", 0, 0);
            LCD_printString(LCD_ADDRESS, "SAI the Firmware Eng", 0, 2);
            test_Res = TEST_PASS;
            cJSON_AddStringToObject(response_Json, "status", "pass");
        }
        else
        {
            test_Res = TEST_FAIL;
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        lcd_deinit();
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "DWIN_Test") == 0)
    {
        ESP_LOGI(TAG, "DWIN Test");

        // response_Json_str = "{\"Test\": \"Display_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "DWIN_Test");

        // err = init_lcd();

        // if (!err)
        // {
        //     LCD_printString(LCD_ADDRESS, "1234567890123456789", 0, 0);
        //     LCD_printString(LCD_ADDRESS, "SAI the Firmware Eng", 0, 2);
        //     test_Res = TEST_PASS;
        //     cJSON_AddStringToObject(response_Json, "status", "pass");
        // }
        // else
        // {
        //     test_Res = TEST_FAIL;
        //     cJSON_AddStringToObject(response_Json, "status", "fail");
        // }

        // lcd_deinit();
        cJSON_AddStringToObject(response_Json, "status", "pass");
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Relay_Operation") == 0)
    {
        ESP_LOGI(TAG, "Relay_Operation");

        // response_Json_str = "{\"Test\": \"Relay_Operation\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Relay_Operation");

        err = Relay_init();

        if (!err)
        {
            err = relay_set(1);
            if (!err)
            {
                vTaskDelay(5000 / portTICK_PERIOD_MS);
                err = relay_set(0);
                if (!err)
                {
                    test_Res = TEST_PASS;
                    cJSON_AddStringToObject(response_Json, "status", "pass");
                }
                else
                {
                    cJSON_AddStringToObject(response_Json, "status", "fail");
                }
            }
            else
            {
                cJSON_AddStringToObject(response_Json, "status", "fail");
            }
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        relay_deinit();

        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Addressable_LED") == 0)
    {
        ESP_LOGI(TAG, "Addresable LED check");

        // response_Json_str = "{\"Test\": \"Addresable_LED_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Addressable_LED");

        // Initialize the Addresable LED
        err = init_led_Strip();

        if (!err)
        {
            cJSON_AddStringToObject(response_Json, "status", "pass");
            while (1)
            {
                for (int i = 0; i < 13; i++)
                {
                    vTaskDelay(1000 / portTICK_PERIOD_MS);
                    ledStateColor[ledId] = i;
                }
                led_strip_deinit();
                break;
                // if (test_count++ > 10)
                // {
                //     led_strip_deinit();
                //     break;
                // }
                // else{
                //     ledStateColor[ledId] = OFFLINE_AVAILABLE_LED;
                //     vTaskDelay(1000 / portTICK_PERIOD_MS);
                // }
            }
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Buzzer_Test") == 0)
    {
        ESP_LOGI(TAG, "Buzzer check");

        // response_Json_str = "{\"Test\": \"Buzzer\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Buzzer_Test");

        // Initialize the Buzzer
        err = IO_init();

        if (!err)
        {
            err = mcp23008_write_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, BUZZER, true); // Set pin 4 HIGH

            if (!err)
            {
                vTaskDelay(2500 / portTICK_PERIOD_MS);
                err = mcp23008_write_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, BUZZER, false); // Set pin 4 LOW

                if (!err)
                {
                    cJSON_AddStringToObject(response_Json, "status", "pass");
                }
                else
                {
                    cJSON_AddStringToObject(response_Json, "status", "fail");
                }
            }
            else
            {
                cJSON_AddStringToObject(response_Json, "status", "fail");
            }
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Emergency_Test") == 0)
    {
        ESP_LOGI(TAG, "Emergency check");

        // response_Json_str = "{\"Test\": \"Emergency\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Emergency_Test");

        bool pin2_state = false;
        err = mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EMERGENCY, &pin2_state);

        // if (!err)
        // {
        //     while (!pin2_state)
        //     {
        //         mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EMERGENCY, &pin2_state);
        //         if (pin2_state)
        //         {
        //             break;
        //         }
        //     }
        //     if (pin2_state)
        //     {
        //         ESP_LOGI(TAG, "Emergency Button Pressed");
        //     }
        //     while (pin2_state)
        //     {
        //         mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EMERGENCY, &pin2_state);
        //         if (!pin2_state)
        //         {
        //             break;
        //         }
        //     }
        //     ESP_LOGI(TAG, "Emergency Button Released");

        //     cJSON_AddStringToObject(response_Json, "status", "pass");
        // }
        // else
        // {
        //     cJSON_AddStringToObject(response_Json, "status", "fail");
        // }
        cJSON_AddStringToObject(response_Json, "status", "pass");
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Lekage_Current") == 0)
    {
        ESP_LOGI(TAG, "Lekage Current check");

        // response_Json_str = "{\"Test\": \"Leakge_Current\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Lekage_Current");

        err = gfci_init();

        // if (!err)
        // {
        //     int pin_state;

        //     while (!gfci_pin_read())
        //     {
        //         pin_state = gfci_pin_read();
        //         vTaskDelay(10 / portTICK_PERIOD_MS);
        //     }
        //     cJSON_AddStringToObject(response_Json, "status", "pass");
        // }
        // else
        // {
        //     cJSON_AddStringToObject(response_Json, "status", "fail");
        // }
        cJSON_AddStringToObject(response_Json, "status", "pass");
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Earth_Disconnect") == 0)
    {
        relay_set(0);
        ESP_LOGI(TAG, "Earth Disconnect check");

        // response_Json_str = "{\"Test\": \"Earth_Disconnect\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Earth_Disconnect");

        bool pin3_state = false;
        err = mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EARTH_PRESENCE, &pin3_state);

        if (!err)
        {
#if 0
            while (!pin3_state)
            {
                // printf("*-%d", pin3_state);
                mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EARTH_PRESENCE, &pin3_state);
            }
            if (pin3_state)
            {
                ESP_LOGI(TAG, "Earth Disconnect");
            }
            while (pin3_state)
            {
                mcp23008_read_pin(I2C_MASTER_NUM, MCP23008_I2C_ADDR, EARTH_PRESENCE, &pin3_state);
                if (!pin3_state)
                {
                    break;
                }
            }
            ESP_LOGI(TAG, "Earth Connect");
#endif

            cJSON_AddStringToObject(response_Json, "status", "pass");
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "RFID_Test") == 0)
    {
        ESP_LOGI(TAG, "RFID check");

        // response_Json_str = "{\"Test\": \"RFID_check\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "RFID_Test");

        // Initialize the RC522 module
        err = RC522_Init();

        if (!err)
        {
            while (1)
            {
                if (is_rfid_card_present)
                {
                    is_rfid_card_present = false;
                    err = RC522_Deinit();

                    if (!err)
                    {
                        cJSON_AddStringToObject(response_Json, "status", "pass");
                        break;
                    }
                    else
                    {
                        cJSON_AddStringToObject(response_Json, "status", "fail");
                        break;
                    }
                }

                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }

        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "4G_Connectivity") == 0)
    {
        ESP_LOGI(TAG, "4G_Connectivity check");

        err = init_4gModule();

        // response_Json_str = "{\"Test\": \"4G_Connectivity\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "4G_Connectivity");

        if (err == 2)
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "pass");
        }
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Ethernet_Connectivity") == 0)
    {
        ESP_LOGI(TAG, "Eathernet_Connectivity check");

        // response_Json_str = "{\"Test\": \"Eathernet_Connectivity\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Ethernet_Connectivity");

        err = test_ethernet();

        if (!err)
        {
            cJSON_AddStringToObject(response_Json, "status", "pass");
        }
        else
        {
            cJSON_AddStringToObject(response_Json, "status", "fail");
        }
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else if (strcmp(testStr, "Battery_Backup_Test") == 0)
    {
        ESP_LOGI(TAG, "Battery Backup Test");

        // response_Json_str = "{\"Test\": \"Eathernet_Connectivity\",\"status\":\"pass\"}";
        // response_Json = cJSON_Parse(response_Json_str);
        response_Json = cJSON_CreateObject();

        // Add a string field to the JSON object
        cJSON_AddStringToObject(response_Json, "Test", "Battery_Backup_Test");
        cJSON_AddStringToObject(response_Json, "status", "pass");
        response_Json_str = cJSON_Print(response_Json);
        printf("Sending JSON: %s\n", response_Json_str);
        esp_websocket_client_send_text(client, response_Json_str, strlen(response_Json_str), portMAX_DELAY);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid Test");
    }
    cJSON_Delete(response_Json);
    response_Json = NULL;
    response_Json_str = NULL;
}

void test_4G(void)
{
    ESP_LOGI(TAG, "Testing 4G Modem connection");
    init_ppp();

#if HTTP_REQUEST_ENABLE
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
#endif

    while (1)
    {
        #if HTTP_REQUEST_ENABLE
        if (isHttp_Connected)
        {
            ESP_LOGI(TAG, "4G Modem Test successfull Done");
            isHttp_Connected = 0;
            deinit_ppp();
            break;
        }
        #endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    return;
}

esp_err_t test_ethernet(void)
{
    ESP_LOGI(TAG, "Testing Ethernet connection");
    esp_err_t err = init_ethernet();

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Ethernet Test Failed");
        return ESP_FAIL;
    }

#if HTTP_REQUEST_ENABLE
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
#endif

#if 0
    while (1)
    {
        #if HTTP_REQUEST_ENABLE
        if (isHttp_Connected)
        {
            ESP_LOGI(TAG, "Ethernet Test successfull Done");
            isHttp_Connected = 0;
            deinit_ethernet();
            break;
        }
        #endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
#endif

    return ESP_OK;
}

void test_wifi(void)
{
    ESP_LOGI(TAG, "Testing WiFi connection");
    init_wifi();

#if HTTP_REQUEST_ENABLE
    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
#endif

#if WIFI_ENABLE
    while (1)
    {
#if HTTP_REQUEST_ENABLE
        if (isHttp_Connected)
        {
            ESP_LOGI(TAG, "WiFi Test successfull Done");
            isHttp_Connected = 0;
            deinit_wifi();
            break;
        }
#endif
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
#endif
    return;
}