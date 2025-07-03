#ifndef __DOORBELL_LED_H__
#define __DOORBELL_LED_H__

#include "led_strip.h"
#include "esp_mac.h"
#include "esp_log.h"
#include "esp_err.h"
#include "doorbell_config.h"


typedef enum{
    LED_ON=0,
    LED_OFF
}LED_STATUS;

typedef struct{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
}LED_COLOR;


void doorbell_led_init(void);


void doorbell_led_on_off(uint8_t led_num,LED_COLOR led_color,LED_STATUS led_status);



#endif /* __DOORBELL_LED_H__ */
