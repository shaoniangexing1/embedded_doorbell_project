#ifndef __DOORBELL_BUTTON_H__
#define __DOORBELL_BUTTON_H__


#include "button_adc.h"
#include "iot_button.h"
#include "errno.h"

//短按按键初始化
void doorbell_button_init(void);



void doorbell_button_front_register( button_event_t event,button_cb_t cb,void *arg);

void doorbell_button_brack_register(button_event_t event, button_cb_t cb, void *arg);

#endif /* __DOORBELL_BUTTON_H__ */
