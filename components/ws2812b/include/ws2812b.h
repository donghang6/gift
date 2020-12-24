#pragma once

#include "esp_log.h"
#include "led_strip.h"
#include "driver/rmt.h"
#include "color.h"

led_strip_t *ws2812b_init();
void ws2812b_set_pixel(led_strip_t *strip, uint32_t index, uint32_t color);
void ws2812b_luminance(led_strip_t *strip, uint32_t index, uint32_t color, float luminance);