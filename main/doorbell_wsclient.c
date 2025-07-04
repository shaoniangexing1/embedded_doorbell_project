#include "doorbell_wsclient.h"

#define TAG "Wsclient"
static esp_websocket_client_handle_t websocket_client_sound = NULL;
static esp_websocket_client_handle_t websocket_client_image = NULL;

static void (*sound_callback)(void *arg, void *data, size_t len)=NULL;
static void *sound_callback_arg=NULL;

static void doorbell_websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    esp_websocket_client_handle_t client = data->client;

    switch (event_id)
    {
    case WEBSOCKET_EVENT_BEGIN:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_BEGIN:%s", client == websocket_client_image ? "image" : "sound");
        break;
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED:%s", client == websocket_client_image ? "image" : "sound");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED:%s", client == websocket_client_image ? "image" : "sound");
        break;
    case WEBSOCKET_EVENT_DATA:
        // ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        if (client == websocket_client_image )
        {
            // 如果是图片或不是二进制文件则不处理
            break;
        }
        if (data->op_code != 0x02)
        {
            break;
        }
        
        // 接收音频信息
        if (sound_callback)
        {
            sound_callback(sound_callback_arg, (void *)data->data_ptr, data->data_len);
        }

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR:%s", client == websocket_client_image ? "image" : "sound");
        break;
    case WEBSOCKET_EVENT_FINISH:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_FINISH:%s", client == websocket_client_image ? "image" : "sound");
        break;
    }
}

void doorbell_wsclient_init(void)
{
    // 语音websocket初始化
    esp_websocket_client_config_t websocket_cfg = {
        .uri = "ws://" MQTT_SERVER_IP ":" WS_PORT WS_SOUND_PATH,
    };
    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);
    websocket_client_sound = esp_websocket_client_init(&websocket_cfg);
    assert(websocket_client_sound);
    // 图像websocket初始化
    esp_websocket_client_config_t websocket_cfg_image = {
        .uri = "ws://" MQTT_SERVER_IP ":" WS_PORT WS_IMAGE_PATH,
    };
    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg_image.uri);
    websocket_client_image = esp_websocket_client_init(&websocket_cfg_image);
    assert(websocket_client_image);

    // 注册事假回调
    // 音频事件回调
    esp_websocket_register_events(websocket_client_sound, WEBSOCKET_EVENT_ANY, doorbell_websocket_event_handler, (void *)websocket_client_sound);
    // 图片事件回调
    esp_websocket_register_events(websocket_client_image, WEBSOCKET_EVENT_ANY, doorbell_websocket_event_handler, (void *)websocket_client_image);
}

// 启动客户端与服务器连接
void doorbell_wsclient_start(void)
{
    esp_websocket_client_start(websocket_client_sound);
    esp_websocket_client_start(websocket_client_image);
}
// 停止ws客户端与服务器连接
void doorbell_wsclint_stop(void)
{
    ESP_ERROR_CHECK(esp_websocket_client_stop(websocket_client_sound));
    ESP_ERROR_CHECK(esp_websocket_client_stop(websocket_client_image));
}

bool doorbell_wsclient_is_image_ready(void)
{
    return esp_websocket_client_is_connected(websocket_client_image);
}

bool doorbell_wsclient_is_sound_ready(void)
{
    return esp_websocket_client_is_connected(websocket_client_sound);
}

void doorbell_wsclient_send_message_image(void *image_message, size_t len)
{
    esp_websocket_client_send_bin(websocket_client_image, image_message, len, portMAX_DELAY);
}

void doorbell_wsclient_send_message_sound(void *sound_message, size_t len)
{
    esp_websocket_client_send_bin(websocket_client_sound, sound_message, len, portMAX_DELAY);
}

void doorbell_wsclient_register_sound_callback(void (*callback)(void *arg, void *data, size_t len), void *arg)
{
    sound_callback = callback;
    sound_callback_arg = arg;
}
