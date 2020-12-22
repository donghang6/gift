#ifndef IR_H_
#define IR_H_
#include "driver/rmt.h"
#include "ir_tools.h"
#include "esp_log.h"

void ir_init();
void ir_receive();
#endif