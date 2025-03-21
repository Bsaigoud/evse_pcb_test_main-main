
#include "EnergyMeter.h"

static const char *TAG = "Energy Meter";
spi_device_handle_t spi_EM;
gptimer_handle_t gptimer;
volatile int8_t Em_Test_case = -1;
uint8_t Test_cnt = 1;

static TaskHandle_t energy_meterHandler = NULL;
// Function to initialize SPI bus

void readEnrgyMeterValues(void)
{
    // if (xSemaphoreTake(mutexSpiBus, portMAX_DELAY))
    // {
    uint16_t temp = CommEnergyIC(spi_EM, ATM90E36A_READ, Temp, 0);     // Temp Register
    float voltage = CommEnergyIC(spi_EM, ATM90E36A_READ, UrmsA, 0);    // Temp Register
    // uint16_t current = CommEnergyIC(spi_EM, ATM90E36A_READ, IrmsA, 0); // Temp Register
    // uint16_t power = CommEnergyIC(spi_EM, ATM90E36A_READ, PmeanAF, 0); // Temp Register
    double volt = (double)voltage / 100;
    // xSemaphoreGive(mutexSpiBus);
    ESP_LOGI(TAG, "temp: %u \n", temp);
    // ESP_LOGI(TAG, "Voltage: %f V\n", voltage * 0.01);//
    volt = (volt * 1.539249378) + 0.051283407;
    ESP_LOGI(TAG, "Voltage: %f V\n", volt);
    // uint32_t voltage_chk = (uint32_t)volt;
    
    if (((temp >= 28) && (temp <= 36)) && ((volt >= 200) && (volt <= 250)))
    {
        if((Test_cnt++ % 10) == 0)
        {
            printf("\r\nEm Test Done..\r\n");
            Em_Test_case = 1;
        }
    }
    else
    {
        Em_Test_case = 0;
    }

    // if (temp >= 30 )
    // {

    //     Test_case = 3;
    // }
    //  ESP_LOGI(TAG, "Voltage: %f V\n", voltage *  0.051283407);
    // ((volt * 1.539249378) + 0.051283407)

    // ESP_LOGI(TAG, "current: %f A\n", current * 0.001 * config.CurrentGain1);
    // ESP_LOGI(TAG, "Power Calculated: %f A\n", voltage * 0.01 * current * 0.001 * config.CurrentGain1);
    // ESP_LOGI(TAG, "Power Fundamental: %f A\n", power * 1.0);
    // }
}
// void delayMicroSeconds(gptimer_handle_t gptimer, uint32_t time)
// {
//     uint64_t timer_count = 0;
//     gptimer_set_raw_count(gptimer, 0);
//     gptimer_get_raw_count(gptimer, &timer_count);
//     while (timer_count < time)
//     {
//         gptimer_get_raw_count(gptimer, &timer_count);
//     }
// }

uint16_t CommEnergyIC(spi_device_handle_t spi_EM, uint16_t RW, uint16_t address, uint16_t val)
{
    uint16_t readCommand = address | RW;
    uint8_t tx_data[4] = {(readCommand >> 8) & 0xFF, readCommand & 0xFF, (val >> 8) & 0xFF, val & 0xFF};
    uint8_t rx_data[4] = {0, 0, 0, 0};
    // gpio_set_level(PIN_NUM_EM_CS, 0);

    //  vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EM_ISO_EN_PIN, 1);
    // delayMicroSeconds( 10);
    vTaskDelay(pdMS_TO_TICKS(10));

    spi_transaction_t transaction = {
        .cmd = 0,
        .addr = 0,
        .length = 32,   // 2 bytes (16 bits)
        .rxlength = 32, // 2 bytes (16 bits)
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    esp_err_t ret = spi_device_transmit(spi_EM, &transaction);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "SPI transaction failed, error = %x\n", ret);
    }

    // delayMicroSeconds(gptimer, 10);
    //  delayMicroSeconds(10);
    // gpio_set_level(PIN_NUM_EM_CS, 1);
    //  vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(EM_ISO_EN_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    return ((rx_data[2] << 8) | rx_data[3]);
}

void ATM90E36_Initialise(void)
{
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, SoftReset, 0x789A); // Perform soft reset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, FuncEn0, 0x0000);   // Voltage sag
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, FuncEn1, 0x0000);   // Voltage sag
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, SagTh, 0x0001);     // Voltage sag threshold

    /* SagTh = Vth * 100 * sqrt(2) / (2 * Ugain / 32768) */

    // Set metering config values (CONFIG)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, ConfigStart, 0x5678); // Metering calibration startup
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PLconstH, 0x0861);    // PL Constant MSB (default)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PLconstL, 0xC468);    // PL Constant LSB (default)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, MMode0, 0x1087);      // Mode Config (60 Hz, 3P4W)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, MMode1, 0x1500);      // 0x5555 (x2) // 0x0000 (1x)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PStartTh, 0x0000);    // Active Startup Power Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, QStartTh, 0x0000);    // Reactive Startup Power Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, SStartTh, 0x0000);    // Apparent Startup Power Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PPhaseTh, 0x0000);    // Active Phase Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, QPhaseTh, 0x0000);    // Reactive Phase Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, SPhaseTh, 0x0000);    // Apparent  Phase Threshold
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CSZero, 0x4741);      // Checksum 0

    // Set metering calibration values (CALIBRATION)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CalStart, 0x5678); // Metering calibration startup
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, GainA, 0x0000);    // Line calibration gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PhiA, 0x0000);     // Line calibration angle
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, GainB, 0x0000);    // Line calibration gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PhiB, 0x0000);     // Line calibration angle
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, GainC, 0x0000);    // Line calibration gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PhiC, 0x0000);     // Line calibration angle
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PoffsetA, 0x0000); // A line active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, QoffsetA, 0x0000); // A line reactive power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PoffsetB, 0x0000); // B line active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, QoffsetB, 0x0000); // B line reactive power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PoffsetC, 0x0000); // C line active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, QoffsetC, 0x0000); // C line reactive power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CSOne, 0x0000);    // Checksum 1

    // Set metering calibration values (HARMONIC)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, HarmStart, 0x5678); // Metering calibration startup
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, POffsetAF, 0x0000); // A Fund. active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, POffsetBF, 0x0000); // B Fund. active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, POffsetCF, 0x0000); // C Fund. active power offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PGainAF, 0x0000);   // A Fund. active power gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PGainBF, 0x0000);   // B Fund. active power gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, PGainCF, 0x0000);   // C Fund. active power gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CSTwo, 0x0000);     // Checksum 2

    // Set measurement calibration values (ADJUST)
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, AdjStart, 0x5678); // Measurement calibration
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UgainA, 0x0002);   // A SVoltage rms gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IgainA, 0xFD7F);   // A line current gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UoffsetA, 0x0000); // A Voltage offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IoffsetA, 0x0000); // A line current offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UgainB, 0x0002);   // B Voltage rms gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IgainB, 0xFD7F);   // B line current gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UoffsetB, 0x0000); // B Voltage offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IoffsetB, 0x0000); // B line current offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UgainC, 0x0002);   // C Voltage rms gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IgainC, 0xFD7F);   // C line current gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, UoffsetC, 0x0000); // C Voltage offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IoffsetC, 0x0000); // C line current offset
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, IgainN, 0xFD7F);   // C line current gain
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CSThree, 0x02F6);  // Checksum 3

    // Done with the configuration
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, ConfigStart, 0x5678);
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, CalStart, 0x5678);  // 0x6886 //0x5678 //8765);
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, HarmStart, 0x5678); // 0x6886 //0x5678 //8765);
    CommEnergyIC(spi_EM, ATM90E36A_WRITE, AdjStart, 0x5678);  // 0x6886 //0x5678 //8765);

    CommEnergyIC(spi_EM, ATM90E36A_WRITE, SoftReset, 0x789A); // Perform soft reset
}
void Energy_meter_deinit(void)
{
    Test_cnt = 0;

    // Remove the SPI device
    spi_bus_remove_device(spi_EM);

    // Deinitialize the SPI bus
    spi_bus_free(SPI3_HOST);

    // Delete the task
    vTaskDelete(energy_meterHandler);
}

void init_Energy_meter_Task(void *pvParameters)
{
    while (1)
    {
        // Task logic for PCF85062 (e.g., reading/writing to the RTC)
        printf("Running Energy_meter_Task task\n");
        readEnrgyMeterValues();
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}
esp_err_t Energy_meter_init(void)
{
        gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << EM_ISO_EN_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .intr_type = GPIO_INTR_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(EM_ISO_EN_PIN, 1);
    spi_bus_config_t bus_config = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    spi_device_interface_config_t dev_config = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0,
        .duty_cycle_pos = 128,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 1000000, // Adjust as needed
        .input_delay_ns = 0,
        .spics_io_num = PIN_NUM_EM_CS,
        .flags = 0,
        .queue_size = 1,
        .pre_cb = NULL,
        .post_cb = NULL,
    };

    esp_err_t ret = spi_bus_initialize(SPI3_HOST, &bus_config, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to initialize spi bus, error = %x\n", ret);
        return ret;
    }
    ret = spi_bus_add_device(SPI3_HOST, &dev_config, &spi_EM);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add SPI device, error = %x\n", ret);
        return ret;
    }

    ATM90E36_Initialise();

    xTaskCreate(init_Energy_meter_Task, "Energy_meter_Task", 4096, NULL, 2, &energy_meterHandler);

return ESP_OK;
}