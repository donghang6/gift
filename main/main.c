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
#include "ir.h"
#include "esp_log.h"
#include "ws2812b.h"

static const char *TAG = "main";

static SemaphoreHandle_t lock = NULL;
static QueueHandle_t queue = NULL;

static void ws2812b_task(void *arg)
{
    led_strip_t *strip = (led_strip_t *)arg;
    static uint8_t togglt = false;
    ir result;
    if (xQueueReceive(queue, &result, portMAX_DELAY) == pdPASS) {
        switch (result.cmd)
        {
            case 0xBA45: // power
                printf("power\n");
                    if (!togglt) {
                        ws2812b_set_pixel(strip, 0, RED);
                        ws2812b_set_pixel(strip, 1, RED);
                        ws2812b_set_pixel(strip, 2, RED);
                        ws2812b_set_pixel(strip, 3, RED);
                        ws2812b_set_pixel(strip, 4, RED);
                        ws2812b_set_pixel(strip, 5, RED);
                        ws2812b_set_pixel(strip, 6, RED);
                        ws2812b_set_pixel(strip, 7, RED);
                        ESP_ERROR_CHECK(strip->refresh(strip, 1000));
                    } else {
                        ESP_ERROR_CHECK(strip->clear(strip, 1000));
                    }
                    togglt = !togglt;
                break;
            case 0xB946: // up
                printf("up\n");
                break;
            case 0xB847: // alien
                printf("alien\n");
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
                printf("down\n");
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
    xSemaphoreGive(lock);
    vTaskDelete(NULL);
}

static void ir_receive_task(void *arg)
{
    led_strip_t *strip = (led_strip_t *)arg;
    for(;;) {
        ir receive_result =  ir_receive();
        if (receive_result.items && receive_result.addr != 0 && receive_result.cmd != 0) {
            if (xSemaphoreTake(lock, 0) == pdPASS) {
                if (xQueueSendToBack(queue, &receive_result, 0) != pdPASS) {
                    ESP_LOGE(TAG, "send message to queue failed\n");
                }
                xTaskCreate(ws2812b_task, "ws2812b", 2000, strip, 3, NULL);
            }
        }
    }
    vTaskDelete(NULL);
}

static void initial_task(void *arg)
{
    led_strip_t *strip = ws2812b_init();
    ir_init();
    lock = xSemaphoreCreateBinary();
    if (lock == NULL) {
        ESP_LOGE(TAG, "Create mutex failed!");
    }
    xSemaphoreGive(lock);
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