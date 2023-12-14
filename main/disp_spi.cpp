#include "disp_spi.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"

#if CONFIG_SPI2_HOST
#define HOST_ID SPI2_HOST // HSPI_HOST
#elif CONFIG_SPI3_HOST
#define HOST_ID SPI3_HOST // VSPI_HOST
#endif

static spi_device_handle_t spi;
static volatile bool spi_trans_in_progress;
static volatile bool spi_color_sent;
static const char *TAG = "disp_spi";

static void IRAM_ATTR spi_ready(spi_transaction_t *trans)
{
    spi_trans_in_progress = false;

    // if (spi_color_sent)
    //   lv_flush_ready();
}

void disp_spi_init(void)
{
    // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/spi_master.html

    esp_err_t ret;

    ESP_LOGI(TAG, "[I] ILI9488 MOSI GPIO = %d", DISP_SPI_MOSI);
    ESP_LOGI(TAG, "[I] ILI9488 CLK GPIO = %d", DISP_SPI_CLK);
    ESP_LOGI(TAG, "[I] ILI9488 CS GPIO = %d", DISP_SPI_CS);

    spi_bus_config_t buscfg = {
        .mosi_io_num = DISP_SPI_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = DISP_SPI_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = 0,
        .data5_io_num = 0,
        .data6_io_num = 0,
        .data7_io_num = 0,
        .max_transfer_sz = 0,
        .flags = 0,
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
        .isr_cpu_id = ESP_INTR_CPU_AFFINITY_AUTO,
#else
        .isr_cpu_id = static_cast<intr_cpu_id_t>(0),
#endif
        .intr_flags = 0,
    };

    spi_device_interface_config_t devcfg = {
        .command_bits = 0,
        .address_bits = 0,
        .dummy_bits = 0,
        .mode = 0, // SPI mode 0
        .clock_source = SPI_CLK_SRC_DEFAULT,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = SPI_MASTER_FREQ_40M, // Clock out at 40 MHz
        .input_delay_ns = 0,
        .spics_io_num = DISP_SPI_CS, // CS pin
        .flags = SPI_DEVICE_NO_DUMMY,
        .queue_size = 7,
        .pre_cb = NULL,
        .post_cb = spi_ready,
    };

    // Initialize the SPI bus
    ret = spi_bus_initialize(HOST_ID, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);

    // Attach the LCD to the SPI bus
    ret = spi_bus_add_device(HOST_ID, &devcfg, &spi);
    assert(ret == ESP_OK);
}

void disp_spi_send_data(uint8_t *data, uint16_t length)
{
    if (length == 0)
        return; // no need to send anything

    while (spi_trans_in_progress)
        ;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Zero out the transaction
    t.length = length * 8;    // Length is in bytes, transaction length is in bits.
    t.tx_buffer = data;       // Data
    spi_trans_in_progress = true;
    spi_color_sent = false; // Mark the "lv_flush_ready" NOT needs to be called in "spi_ready"
    spi_device_queue_trans(spi, &t, portMAX_DELAY);
}

void disp_spi_send_colors(uint8_t *data, uint16_t length)
{
    if (length == 0)
        return; // no need to send anything

    while (spi_trans_in_progress)
        ;

    spi_transaction_t t;
    memset(&t, 0, sizeof(t)); // Zero out the transaction
    t.length = length * 8;    // Length is in bytes, transaction length is in bits.
    t.tx_buffer = data;       // Data
    spi_trans_in_progress = true;
    spi_color_sent = true; // Mark the "lv_flush_ready" needs to be called in "spi_ready"
    spi_device_queue_trans(spi, &t, portMAX_DELAY);
    // spi_device_transmit(spi, &t);
}
