#pragma once

#include "driver/rmt.h"
#include "ir_tools.h"
#include "esp_log.h"
typedef struct {
    rmt_item32_t *items;
    uint32_t addr;
    uint32_t cmd;
} ir;

void ir_init();
ir ir_receive();
