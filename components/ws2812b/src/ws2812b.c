#include "ws2812b.h"

static const char *TAG = "ws2812b";

#define RMT_TX_CHANNEL RMT_CHANNEL_0

#define EXAMPLE_CHASE_SPEED_MS (10)

led_strip_t * ws2812b_init()
{
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(18, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(8, (led_strip_dev_t)config.channel);
    led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    ESP_ERROR_CHECK(strip->clear(strip, 100));
    return strip;
}

void ws2812b_set_pixel(led_strip_t *strip, uint32_t index, uint32_t color)
{
    ws2812b_luminance(strip, index, color, 0.5);
}

void ws2812b_luminance(led_strip_t *strip, uint32_t index, uint32_t color, float luminance)
{
    uint32_t red   = (color >> 16 & 0xFF) * luminance;
    uint32_t green = (color >> 8 & 0xFF) * luminance;
    uint32_t blue  = (color & 0xFF) * luminance;
    ESP_ERROR_CHECK(strip->set_pixel(strip, index, red, green, blue));
}