#include "ir.h"

static const char *TAG = "ir";
static RingbufHandle_t rb = NULL;
static ir_parser_t *ir_parser = NULL;
void ir_init()
{
    rmt_config_t rmt_rx_config = RMT_DEFAULT_CONFIG_RX(19, RMT_CHANNEL_1);
    rmt_config(&rmt_rx_config);
    rmt_driver_install(RMT_CHANNEL_1, 1000, 0);
    ir_parser_config_t ir_parser_config = IR_PARSER_DEFAULT_CONFIG((ir_dev_t)RMT_CHANNEL_1);
    ir_parser_config.flags |= IR_TOOLS_FLAGS_PROTO_EXT;
    ir_parser = ir_parser_rmt_new_nec(&ir_parser_config);
    rmt_get_ringbuf_handle(RMT_CHANNEL_1, &rb);
    rmt_rx_start(RMT_CHANNEL_1, true);
}

ir ir_receive()
{
    ir result = {
        .items = NULL,
        .addr = 0,
        .cmd = 0
    };
    bool repeat = false;
    rmt_item32_t *items = NULL;
    uint32_t length = 0;
    uint32_t addr = 0;
    uint32_t cmd = 0;
    items = (rmt_item32_t *) xRingbufferReceive(rb, &length, 1000);
    if (items) {
        result.items = items;
        length /= 4; // one RMT = 4 Bytes
        if (ir_parser->input(ir_parser, items, length) == ESP_OK) {
            if (ir_parser->get_scan_code(ir_parser, &addr, &cmd, &repeat) == ESP_OK) {
                result.addr = addr;
                result.cmd = cmd;
                ESP_LOGI(TAG, "Scan Code --- addr: 0x%04x cmd: 0x%04x", addr, cmd);
            }
        }
        //after parsing the data, return spaces to ringbuffer.
        vRingbufferReturnItem(rb, (void *) items);
    }
    return result;
}