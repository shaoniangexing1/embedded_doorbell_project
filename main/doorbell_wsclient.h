#ifndef __DOORBELL_WSCLIENT_H__
#define __DOORBELL_WSCLIENT_H__

#include "esp_types.h"
#include "esp_log.h"
#include "esp_websocket_client.h"
#include "esp_event.h"
#include "doorbell_config.h"

void doorbell_wsclient_init(void);

// 启动客户端与服务器连接
void doorbell_wsclient_start(void);
// 停止ws客户端与服务器连接
void doorbell_wsclint_stop(void);

bool doorbell_wsclient_is_image_ready(void);

bool doorbell_wsclient_is_sound_ready(void);

void doorbell_wsclient_send_message_image(void *image_message, size_t len);

void doorbell_wsclient_send_message_sound(void *sound_message, size_t len);

void doorbell_wsclient_register_sound_callback(void (*callback)(void *arg, void *data, size_t len), void *arg);

#endif /* __DOORBELL_WSCLIENT_H__ */
