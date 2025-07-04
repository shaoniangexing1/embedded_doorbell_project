#include "doorbell_mqtt.h"

#define TAG "Mqtt"
#define MQTT_CMD_MAX_NUM 10

//订阅的其他发布者主题
#define MQTT_TOPIC_SUBSCRIBE_CMD "doorbell/cmd"

//本地发布者主题
#define MQTT_TOPIC_PUBLISH "doorbell/data"

static esp_mqtt_client_handle_t client=NULL;
//订阅的主题发来的指令
static mqtt_cmd_t mqtt_cmd_list[MQTT_CMD_MAX_NUM] = {0};
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    // client = event->client;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        // 建立连接，订阅web服务器主题doorbell/cmd
        esp_mqtt_client_subscribe(client, MQTT_TOPIC_SUBSCRIBE_CMD, 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        // 没有连接成功
        break;
    case MQTT_EVENT_SUBSCRIBED:
        // 订阅成功，尝试发送测试消息
        doorbell_mqtt_send_message("{\"status\":\"ready\"}");
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        // 没有连接成功
        break;
    case MQTT_EVENT_PUBLISHED:
        //发送成功
        break;
    case MQTT_EVENT_DATA:
        // 接收数据
        cJSON *root = cJSON_ParseWithLength(event->data,event->data_len);
        if (root == NULL)
        {
            ESP_LOGI(TAG,"Error: Invalid JSON string.");
            return;
        }
        cJSON *cmd_json=cJSON_GetObjectItem(root,"cmd");
        if (!cJSON_IsString(cmd_json))
        {
            goto FINALLY;
        }
        for (uint8_t i = 0; i < MQTT_CMD_MAX_NUM; i++)
        {
            if (mqtt_cmd_list[i].mqtt_cmd[0]=='\0')
            {
                //没有指令，不接收消息
                break;
                /* code */
            }
            if (strcmp(cmd_json->valuestring,mqtt_cmd_list[i].mqtt_cmd)==0)
            {
                //匹配到对应指令，执行回调
                ESP_LOGI(TAG,"MQTT_CMD: %s",mqtt_cmd_list[i].mqtt_cmd);
                mqtt_cmd_list[i].mqtt_cmd_callback(mqtt_cmd_list[i].mqtt_cmd_arg);
                break;
                /* code */
            }
        }
    FINALLY:
        cJSON_Delete(root);
        break;
    case MQTT_EVENT_ERROR:

        break;
    default:
        break;
    }
}
void doorbell_mqtt_init(void)
{
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_SCHEMA MQTT_SERVER_IP ":" MQTT_PORT,
    };
    // 初始化mqtt客户端
    client = esp_mqtt_client_init(&mqtt_cfg);
    // 检查mqtt初始化结果
    assert(client);
    // 注册mqtt事件
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    // 启动mqtt客户端
    esp_mqtt_client_start(client);
}

void doorbell_mqtt_send_message(char *message)
{
    // qos消息传递的可靠级别有三级 依次是低中高
    //作为发布者，发布数据信息，主题为doorbell/data
    ESP_ERROR_CHECK(esp_mqtt_client_publish(client, MQTT_TOPIC_PUBLISH, message, 0, 0, 0));
}
/**
 * @brief 注册mqtt回调指令
 *
 * @param cmd 要注册的指令
 */
void doorbell_mqtt_register_cmd(mqtt_cmd_t *cmd)
{
    uint8_t i;
    for (i = 0; i < MQTT_CMD_MAX_NUM; i++)
    {
        if (mqtt_cmd_list[i].mqtt_cmd[0] == '\0')
        {
            break;
        }
        else if (strcmp(mqtt_cmd_list[i].mqtt_cmd,cmd->mqtt_cmd) == 0)
        {
            break;
        }
    }
    if (i<MQTT_CMD_MAX_NUM)
    {
        memcpy(&mqtt_cmd_list[i], cmd, sizeof(mqtt_cmd_t));
    }
    
}
