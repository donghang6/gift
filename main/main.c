/*
 * @Descripttion: 
 * @version: 
 * @Author: donghang
 * @Date: 2019-08-04 21:56:33
 * @LastEditors: donghang
 * @LastEditTime: 2019-08-29 22:38:00
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "ir.h"
#include "esp_log.h"
#include "ws2812b.h"

#define MAX_COUNT 8

static const char *TAG = "main";

static QueueHandle_t queue = NULL;
static TimerHandle_t timer = NULL;

typedef struct
{
    ir receive_result;
    led_strip_t *strip;
}ws2812b_task_info;

static void ws2812b_task(void *arg)
{
    led_strip_t *strip = (led_strip_t *)arg;
    static uint8_t togglt = false;
    static float luminance = 0.5;
    ir result;
    if (xQueueReceive(queue, &result, portMAX_DELAY) == pdPASS) {
        switch (result.cmd)
        {
            case 0xBA45: // power
                if (!togglt) {
                    for (int i = 0; i < MAX_COUNT; i++) {
                        ws2812b_set_pixel(strip, i, RED);
                    }
                    ESP_ERROR_CHECK(strip->refresh(strip, 1000));
                    ESP_LOGI(TAG, "power on\n");
                } else {
                    ESP_ERROR_CHECK(strip->clear(strip, 1000));
                    ESP_LOGI(TAG, "power off\n");
                    luminance = 0.5;
                }
                togglt = !togglt;
                break;
            case 0xB946: // up
                ESP_LOGI(TAG, "up\n");
                if (togglt) {
                    if (luminance + 0.05 < 1.01) {
                        luminance += 0.05;
                        ESP_LOGI(TAG, "luminance up: %.2f\n", luminance);
                        for (int i = 0; i < MAX_COUNT; i++) {
                            ws2812b_luminance(strip, i, RED, luminance);
                        }
                    } else {
                        ESP_LOGI(TAG, "maximum luminance\n");
                    }
                    ESP_ERROR_CHECK(strip->refresh(strip, 1000));
                }
                break;
            case 0xB847: // alien
                ESP_LOGI(TAG, "alien\n");
                if (togglt) {
                    while (luminance - 0.01 > -0.005) {
                        luminance -= 0.01;
                        ESP_LOGI(TAG, "luminance down: %.2f\n", luminance);
                        for (int i = 0; i < MAX_COUNT; i++) {
                            ws2812b_luminance(strip, i, RED, luminance);
                        }
                        ESP_ERROR_CHECK(strip->refresh(strip, 1000));
                        vTaskDelay(pdMS_TO_TICKS(10));
                    }
                }
                break;
            case 0xBB44: // prefore
                printf("prefore\n");
                break;
            case 0xBF40: // pause
                printf("pause\n");
                break;
            case 0xBC43: // next
                printf("next\n");
                break;
            case 0xF807: // vol-
                printf("vol-\n");
                break;
            case 0xEA15: // down
                ESP_LOGI(TAG, "down\n");
                if (togglt) {
                    if (luminance - 0.05 > -0.01) {
                        luminance -= 0.05;
                        ESP_LOGI(TAG, "luminance down: %.2f\n", luminance);
                        for (int i = 0; i < MAX_COUNT; i++) {
                            ws2812b_luminance(strip, i, RED, luminance);
                        }
                    } else {
                        ESP_LOGI(TAG, "minimum luminance\n");
                    }
                    ESP_ERROR_CHECK(strip->refresh(strip, 1000));
                }
                break;
            case 0xF609: // vol-
                printf("vol-\n");
                break;
            case 0xE916: // 1
                printf("1\n");
                break;
            case 0xE619: // 2
                printf("2\n");
                break;
            case 0xF20D: // 3
                printf("3\n");
                break;
            case 0xF30C: // 4
                printf("4\n");
                break;
            case 0xE718: // 5
                printf("5\n");
                break;
            case 0xA15E: // 6
                printf("6\n");
                break;
            case 0xF708: // 7
                printf("7\n");
                break;
            case 0xE31C: // 8
                printf("8\n");
                break;
            case 0xA55A: // 9
                printf("9\n");
                break;
            case 0xBD42: // 0
                printf("0\n");
                break;
            case 0xB54A: // back
                printf("back\n");
                break;
            default:
                printf("the key is not supported.\n");
                break;
        }
    }
    vTaskDelete(NULL);
}

void vTimerCallback(TimerHandle_t xTimer)
{
    printf("timer expired\n");
    ws2812b_task_info *info = pvTimerGetTimerID(timer);
    if (xQueueSendToBack(queue, &(info->receive_result), 0) != pdPASS) {
        ESP_LOGE(TAG, "send message to queue failed\n");
    }
    xTaskCreate(ws2812b_task, "ws2812b", 2000, info->strip, 3, NULL);
}

static void ir_receive_task(void *arg)
{
    led_strip_t *strip = (led_strip_t *)arg;
    ir receive_result = {
        .items = NULL,
        .cmd = 0,
        .addr = 0
    };
    ws2812b_task_info info = {
        .receive_result = receive_result,
        .strip = strip
    };
    timer = xTimerCreate("Timer", pdMS_TO_TICKS(300), pdFALSE, &info, vTimerCallback);
    if (timer == NULL) {
        ESP_LOGE(TAG, "create timer failed\n");
    }
    for(;;) {
        receive_result =  ir_receive();
        if (receive_result.items && receive_result.addr != 0 && receive_result.cmd != 0) {
            info.receive_result = receive_result;
            vTimerSetTimerID(timer, &info);
            if (xTimerReset(timer, pdMS_TO_TICKS(100)) == pdFAIL) {
                ESP_LOGE(TAG, "reset timer failed\n");
            } else {
                ESP_LOGI(TAG, "reset timer\n");
            }
        }
    }
    vTaskDelete(NULL);
}

static void initial_task(void *arg)
{
    led_strip_t *strip = ws2812b_init();
    ir_init();
    queue = xQueueCreate(1, sizeof(ir));
    if (queue == NULL) {
        ESP_LOGE(TAG, "create queue failed\n");
    }
    xTaskCreate(ir_receive_task, "ir receive", 2000, strip, 2, NULL);
    vTaskDelete(NULL);
}

void app_main()
{
    xTaskCreate(initial_task, "initial", 2000, NULL, 3, NULL);
}