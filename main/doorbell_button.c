#include "doorbell_button.h"


static button_handle_t button_handle = NULL;

void doorbell_button_init(void){
    button_config_t button_cfg = {
        .long_press_time=750,
        .short_press_time=50

    };
    button_adc_config_t button_adc_cfg={
        .min=0,
        .max=10,
        .button_index=0,
        .adc_channel=7,//gpio7
        .unit_id=ADC_UNIT_1//ADC1
    };
    ESP_ERROR_CHECK(iot_button_new_adc_device(&button_cfg,&button_adc_cfg,&button_handle));
}

void doorbell_button_front_register( button_event_t event,button_cb_t cb,void *arg){
    iot_button_register_cb(button_handle,event,NULL,cb,arg);
}

