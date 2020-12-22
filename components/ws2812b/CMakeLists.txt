<<<<<<< HEAD
idf_component_register(SRCS "src/ws28xx.c"
=======
idf_component_register(SRCS "src/led_strip_rmt_ws2812.c" "src/ws2812b.c"
>>>>>>> 8e8d2b5... feature: add ws2812b, ir and freertos
                       INCLUDE_DIRS "include"
                       REQUIRES freertos)