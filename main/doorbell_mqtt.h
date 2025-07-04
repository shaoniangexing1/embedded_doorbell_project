#ifndef __DOORBELL_MQTT_H__
#define __DOORBELL_MQTT_H__


#include "esp_log.h"
#include "mqtt_client.h"
#include "doorbell_config.h"
#include "cJSON.h"


typedef struct
{
    char mqtt_cmd[16];                        // mqtt的指令
    void (*mqtt_cmd_callback)(void *arg); // 对应指令的回调函数
    void *mqtt_cmd_arg;                   // 回调函数的参数
} mqtt_cmd_t;

void doorbell_mqtt_init(void);

void doorbell_mqtt_send_message(char *message);
/**
 * @brief 注册mqtt回调指令
 *
 * @param cmd 要注册的指令
 */
void doorbell_mqtt_register_cmd(mqtt_cmd_t *cmd);

#endif /* __DOORBELL_MQTT_H__ */
