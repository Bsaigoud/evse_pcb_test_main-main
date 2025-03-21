#include "RFID_READ.h"

static const char *TAG = "rc522";
static TaskHandle_t rfidHandler = NULL;
extern uint32_t Test_case;
volatile bool is_rfid_card_present = false;

static rc522_spi_config_t driver_config = {
    .host_id = SPI3_HOST,
    .bus_config = &(spi_bus_config_t){
        .miso_io_num = RC522_SPI_BUS_GPIO_MISO,
        .mosi_io_num = RC522_SPI_BUS_GPIO_MOSI,
        .sclk_io_num = RC522_SPI_BUS_GPIO_SCLK,
    },
    .dev_config = {
        .spics_io_num = RC522_SPI_SCANNER_GPIO_SDA,
    },
    .rst_io_num = RC522_SCANNER_GPIO_RST,
};

static rc522_driver_handle_t driver;
static rc522_handle_t scanner;

static void on_picc_state_changed(void *arg, esp_event_base_t base, int32_t event_id, void *data)
{
    rc522_picc_state_changed_event_t *event = (rc522_picc_state_changed_event_t *)data;
    rc522_picc_t *picc = event->picc;

    if (picc->state == RC522_PICC_STATE_ACTIVE)
    {
        rc522_picc_print(picc);
        is_rfid_card_present = true;
    }
    else if (picc->state == RC522_PICC_STATE_IDLE && event->old_state >= RC522_PICC_STATE_ACTIVE)
    {
        ESP_LOGI(TAG, "Card has been removed");
    }
}

esp_err_t RC522_Deinit(void)
{
    esp_err_t ret;
    // ret = rc522_spi_uninstall(driver);
    //  if (ret != ESP_OK) {
    //     return ret;  // Return the error code if configuration fails
    //   }

    ret = rc522_driver_uninstall(driver);
    if (ret != ESP_OK)
    {
        return ret; // Return the error code if configuration fails
    }
    ret = rc522_destroy(scanner);
    if (ret != ESP_OK)
    {
    }
    return ESP_OK;
}
#if 0

void Rfid_Task(void *pvParameters) 
{
  

   
  
 


    while (1) 
    {
        // Task logic for PCF85062 (e.g., reading/writing to the RTC)
        // printf("Running Rfid_Task task\n");
         

        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

#endif

esp_err_t RC522_Init(void)
{
    esp_err_t ret;

    ret = rc522_spi_create(&driver_config, &driver);
    if (ret != ESP_OK)
    {
        return ret; // Return the error code if configuration fails
    }

    rc522_driver_install(driver);

    rc522_config_t scanner_config = {
        .driver = driver,
    };
    ret = rc522_create(&scanner_config, &scanner);
    if (ret != ESP_OK)
    {
        return ret; // Return the error code if configuration fails
    }
    rc522_register_events(scanner, RC522_EVENT_PICC_STATE_CHANGED, on_picc_state_changed, NULL);
    ret = rc522_start(scanner);
    if (ret != ESP_OK)
    {
        return ret; // Return the error code if configuration fails
    }
    // xTaskCreate(Rfid_Task, "Rfid_task", 4096, NULL, 2,&rfidHandler);
    return ESP_OK;
}