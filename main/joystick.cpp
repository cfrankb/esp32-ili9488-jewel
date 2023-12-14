#include "joystick.h"
#include "esphelpers.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_idf_version.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_continuous.h"

static const char *TAG = "joystick";

void initButtons()
{
    esp_err_t ret;
    ret = gpio_set_direction(JOYSTICK_A_BUTTON, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "[A] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    ret = gpio_set_direction(JOYSTICK_B_BUTTON, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "[B] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    ret = gpio_set_direction(JOYSTICK_C_BUTTON, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "[C] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    ret = gpio_set_direction(JOYSTICK_D_BUTTON, GPIO_MODE_INPUT);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "[C] gpio_set_direction Failed (%s)", esp_err_to_name(ret));
    }

    gpio_set_pull_mode(JOYSTICK_A_BUTTON, GPIO_PULLUP_PULLDOWN);
    gpio_set_pull_mode(JOYSTICK_B_BUTTON, GPIO_PULLUP_PULLDOWN);
    gpio_set_pull_mode(JOYSTICK_C_BUTTON, GPIO_PULLUP_PULLDOWN);
    gpio_set_pull_mode(JOYSTICK_D_BUTTON, GPIO_PULLUP_PULLDOWN);
}

uint8_t readButtons()
{
    int a_button = gpio_get_level(JOYSTICK_A_BUTTON);
    int b_button = gpio_get_level(JOYSTICK_B_BUTTON);
    int c_button = gpio_get_level(JOYSTICK_C_BUTTON);
    int d_button = gpio_get_level(JOYSTICK_D_BUTTON);
    // printf("Buttons: %.2x %.2x %.2x %.2x\n", a_button, b_button, c_button, d_button);
    return a_button |
           (b_button << 1) |
           (c_button << 2) |
           (d_button << 3);
}

// #define DEBUG_JOYSTICK

const adc_channel_t ADC_CHANX = static_cast<adc_channel_t>(CONFIG_X_AXIS);
const adc_channel_t ADC_CHANY = static_cast<adc_channel_t>(CONFIG_Y_AXIS);
static adc_oneshot_unit_handle_t adc1_handle;

bool initJoystick()
{
    ESP_LOGI(TAG, "initJoystick(): started");

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 3, 0)
    // enable functionality present in IDF v5.3
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
#else
    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
#endif

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANX, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CHANY, &config));

#ifdef DEBUG_JOYSTICK
    const adc_channel_t channels[] = {
        ADC_CHANNEL_0,
        ADC_CHANNEL_1,
        ADC_CHANNEL_2,
        ADC_CHANNEL_3,
        ADC_CHANNEL_4,
        ADC_CHANNEL_5,
        ADC_CHANNEL_6,
        ADC_CHANNEL_7,
    };

    for (int i = 0; i < 8; ++i)
    {
        const adc_channel_t &chan = channels[i];
        int io_num;
        ESP_ERROR_CHECK(adc_continuous_channel_to_io(ADC_UNIT_1, chan, &io_num));
        ESP_LOGI(TAG, "ADC%d Channel[%d] Mapped to Pin: %d", ADC_UNIT_1 + 1, chan, io_num);
    }
#endif

    ESP_LOGI(TAG, "initJoystick(): ended");
    return true;
}

uint16_t readJoystick()
{
    int adc_vrx = 0;
    int adc_vry = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANX, &adc_vrx));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, ADC_CHANY, &adc_vry));

    if (adc_vry == -1)
        return false;
    if (adc_vrx == -1)
        return false;

    uint16_t joy = JOY_NONE;

#ifdef CONFIG_REVERSE_Y_AXIS_TRUE
    int flipY = JOY_UP | JOY_DOWN;
#else
    int flipY = 0;
#endif

    if (adc_vry < 50)
    {
        joy |= (JOY_DOWN ^ flipY);
    }
    else if (adc_vry > 3000)
    {
        joy |= (JOY_UP ^ flipY);
    }

#ifdef CONFIG_REVERSE_X_AXIS_TRUE
    int flipX = JOY_LEFT | JOY_RIGHT;
#else
    int flipX = 0;
#endif

    if (adc_vrx < 50)
    {
        joy |= (JOY_LEFT ^ flipX);
    }
    else if (adc_vrx > 3000)
    {
        joy |= (JOY_RIGHT ^ flipX);
    }

#ifdef DEBUG_JOYSTICK
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANX, adc_vrx);
    ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, ADC_CHANY, adc_vry);
    if (joy)
    {
        printf("Knob at X:[%d] Y:[%d] [%d]\n", adc_vrx, adc_vry, joy);
    }
#endif

    return joy;
}