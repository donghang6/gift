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
#include "ws28xx.h"
#include "ir.h"


/*
 * if this constructure couldn't relate to real device(eg. OLED), 
 * it's have no meaning. Because it's different to each device.
 */
void app_main()
{

    // spi_t spi;
    // ws28xx_init(&spi); // initialize spi bus
    ir_init();
    for(;;)
    {
        ir_receive();
        // printf("\n");
        // printf("\nHello World\n");
        // vTaskDelay(1000 / portTICK_RATE_MS );
        // ws28xx_light(&spi, 8, RED);
        // vTaskDelay(1000 / portTICK_RATE_MS );
        // ws28xx_light(&spi, 8, BLUE);
    }
}