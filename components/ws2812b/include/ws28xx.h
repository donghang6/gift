/*
 * @Descripttion: 
 * @version: 
 * @Author: donghang
 * @Date: 2019-08-29 08:12:21
 * @LastEditors: donghang
 * @LastEditTime: 2019-08-29 08:31:03
 */
#ifndef WS2811_H_
#define WS2811_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "driver/spi_master.h"
#include "driver/spi_slave.h"
#include "color.h"

/*
 * @brief: The struct of spi.
 * @host: SPI peripheral that controls this bus. Only support HSPI_HOST and VSPI_HOST
 * @bus_config: Pointer to a spi_bus_config_t struct specifying how the host should be initialized
 * @dma_chan: choose dma channel. Either channel 1 or 2, or 0 in the case when no DMA is required.  
 *            Selecting a DMA channel for a SPI bus allows transfers on the bus to have sizes only 
 *            limited by the amount of internal memory. Selecting no DMA channel (by passing the 
 *            value 0) limits the amount of bytes transfered to a maximum of 64. Set to 0 if only 
 *            the SPI flash uses this bus.
 */
struct spi
{
    esp_err_t ret;
    spi_device_handle_t spi_handle; // attach spi and device
    int dma_chan;
    spi_host_device_t host;
};

typedef struct spi spi_t;

void ws28xx_light(spi_t *spi, uint8_t count, uint32_t color);
void ws28xx_init(spi_t *spi);
#endif