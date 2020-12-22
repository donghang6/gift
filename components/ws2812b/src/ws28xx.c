/*
 * @Descripttion: 
 * @version: 
 * @Author: donghang
 * @Date: 2019-08-29 08:12:27
 * @LastEditors: donghang
 * @LastEditTime: 2019-08-29 22:10:41
 */
#include "ws28xx.h"

#define CODE0 0xF000
#define CODE1 0xFF00

#define ONE_BYTE_LENGTH 8
#define TOTAL_DATA_LENGTH(length) length*ONE_BYTE_LENGTH

/**
 * @brief setting the spi bus
 */
static spi_bus_config_t buscfg = {
    .miso_io_num = 25, // 25
    .mosi_io_num = 23, // 23
    .sclk_io_num = 19, // 19
    .quadwp_io_num = -1,
    .quadhd_io_num = -1,
    .max_transfer_sz = 100,
};

/**
 * @brief: the ws28xx interface config
 */
static spi_device_interface_config_t ws28xx_cfg  = {
    .clock_speed_hz = 13000000,
    .mode = 0,
    .spics_io_num = 22,
    .queue_size = 7,
};


/**
 * @brief: write buffer into ws28xx 
 * @param: `spi` the handle of SPI
 *         `buffer` the buffer of data
 *         `len` the length of buffer
 * @return: ESP_OK on success
 *          ESP_FAIL on fail
 */
static void ws28xx_write_buf(spi_t *spi, uint8_t *buffer, size_t len)
{
    if (len == 0) {
        printf("the len of buffer is zero: %s %d", __func__, __LINE__);
    }
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.length = TOTAL_DATA_LENGTH(len);
    t.tx_buffer = buffer;
    t.rx_buffer = NULL;
    t.user = NULL;
    spi->ret = spi_device_polling_transmit(spi->spi_handle, &t); // Transmit! Transaction are atomic.
    if (spi->ret != ESP_OK) {
        printf("spi device polling transmit failed: %s %d: ", __func__, __LINE__);
    }
}

/**
 * @brief: reset time between two order
 * @param: `spi` the handle of SPI 
 * @return: ESP_OK success
 *          ESP_FAIL fail
 */
static esp_err_t reset(spi_t *spi)
{
    uint8_t *reset_buffer = malloc(sizeof(uint8_t)*(16+488+16)); // 16 0xFF 488 0 16 0xFF 300um
    memset(reset_buffer, 0xFF, 16);
    memset(reset_buffer+16, 0, 488);
    memset(reset_buffer+16+488, 0xFF, 16);
    ws28xx_write_buf(spi, reset_buffer, 16+488+16);
    return spi->ret;
}

void ws28xx_init(spi_t *spi)
{
    spi->host = HSPI_HOST;
    spi->dma_chan = 1;
    spi->ret = spi_bus_initialize(spi->host, &buscfg, spi->dma_chan);
    if (spi->ret != ESP_OK) {
        printf("spi bus initialize fail: %s %d: ", __func__, __LINE__);
    }
    spi->ret = spi_bus_add_device(spi->host, &ws28xx_cfg, &spi->spi_handle);
    if (spi->ret != ESP_OK) {
        printf("spi bus add device fail: %s %d: ", __func__, __LINE__);
    }
}

/**
 * @brief: convert color(for eg. 0x00FF00) to buffer(for eg. CODE0 CODE1 ……)
 * @param: `converted` the buffer has been converted 
 *         `count` the count of light
 *         `color` the color want to show 
 * @return: none
 */
static void convert_color(uint16_t* converted, uint8_t count, uint32_t color)
{
    for(int i = 0; i < 24*count; i++) {
        converted[i] = CODE0;
    }
    uint32_t red   = color & 0xFF0000;
    uint32_t green = color & 0x00FF00;
    uint32_t blue  = color & 0x0000FF;
    uint32_t temp_color = (green << 8) | (red >> 8) | blue;
    for(int cur_light = 0; cur_light < count; cur_light++) {
        uint32_t shift = 0x800000;
        for(int bit = 0; bit < 24; bit++) {
            if(temp_color & shift) {
                converted[cur_light*24+bit] = CODE1;
            }
            shift >>= 1;
        }
        shift = 0x800000;
    }
}

/**
 * @brief: light the ws28xx of count, the order is GRB
 * @param: `spi` the handle of SPI
 *         `count` the count of ws28xx
 *         `buffer` the color buffer
 * @return: none
 */
void ws28xx_light(spi_t *spi, uint8_t count, uint32_t color)
{
    uint16_t* buffer = malloc(sizeof(uint16_t)*count*24);
    uint8_t* line = malloc(sizeof(uint16_t)*count*24);
    convert_color(buffer, count, color);
    for(int i = 0; i < count*24; i++) {
        line[2*i] = (buffer[i] >> 8) & 0xFF;
        line[2*i+1] = buffer[i] & 0xFF;
    }
    ws28xx_write_buf(spi, line, count*24*2);
    reset(spi);
    free(buffer);
}

