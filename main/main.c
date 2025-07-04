#include <stdio.h>

#include "doorbell_button.h"
#include "doorbell_led.h"
#include "doorbell_codec.h"
#include "doorbell_wifi.h"
#include "doorbell_mqtt.h"
#include "doorbell_camera.h"
#include "doorbell_wsclient.h"
#include "freertos/FreeRTOS.h"

#include "string.h"
#include "esp_log.h"

#define TAG "Main"

extern uint8_t dingdong_start[] asm("_binary_output_pcm_start");
extern uint8_t dingdong_end[] asm("_binary_output_pcm_end");

static LED_COLOR led_color = {
    .blue = 0x0005,
    .green = 0x0005,
    .red = 0x0005};

static bool is_streaming=false;


void sound_streaming_task(void *arg){
    void *sound_buf=malloc(2048);
    assert(sound_buf);
    while (is_streaming)
    {
        if (doorbell_wsclient_is_sound_ready())
        {
            doorbell_codec_read(sound_buf,2048);
            doorbell_wsclient_send_message_sound(sound_buf,2048);
            
        }else{
            vTaskDelay(5);
        }
    }
    free(sound_buf);
    vTaskDelete(NULL);
}
void image_streaming_task(void *arg){
    while (is_streaming)
    {
        if (doorbell_wsclient_is_image_ready())
        {
            camera_fb_t *fb=doorbell_camera_capture();
            doorbell_wsclient_send_message_image(fb->buf,fb->len);
            doorbell_camera_release(fb);
            
        }else{
            vTaskDelay(5);
        }
        
    }
    vTaskDelete(NULL);
    
}
static void switch_callback(void *arg){
    if (is_streaming)
    {
        is_streaming=false;
        /* 关闭推流 */
        doorbell_wsclint_stop();
    }else{
        is_streaming=true;
        /* 开启推流 */
        doorbell_wsclient_start();
        xTaskCreate(sound_streaming_task, "sound_task", 4096, NULL, 5, NULL);
        xTaskCreate(image_streaming_task, "image_task", 4096, NULL, 5, NULL);
    }
}

static void upcoming_sound_callback(void *arg,void *data,size_t len){
    if(is_streaming)
        doorbell_codec_write(data,len);
}

static void button_cb(void *button_handle, void *arg)
{
    // ESP_LOGI(TAG, "button event: %s", (char*)arg);
    switch ((BUTTON_STATUS_T)arg)
    {
    case BUTTON_STATUS_SING:
        { /* 发出dingdong声音 */
            doorbell_codec_write((void *)dingdong_start, dingdong_end - dingdong_start);
            ESP_LOGI(TAG, "single click");
        }
        break;
    case BUTTON_STATUS_LONG:
        { /* 长按重新连接wifi */
            doorbell_wifi_reset_provisioning();
            ESP_LOGI(TAG, "long click");
        }

    }
}

void app_main(void)
{

    // 按键模块测试，长按和短按
    doorbell_button_init();

    doorbell_button_front_register(BUTTON_SINGLE_CLICK, button_cb, (void*)BUTTON_STATUS_SING);
    doorbell_button_front_register(BUTTON_LONG_PRESS_START, button_cb, (void*)BUTTON_STATUS_LONG);

    // led灯带测试最多1000个，越多占用空间越大
    doorbell_led_init();

    // 音频编码器测试
    doorbell_codec_init();
    doorbell_codec_open();

    // wifi初始化测试
    doorbell_wifi_init();
    // 编码器收发语音测试
    //  void* buf = malloc(2048);
    //  while (1)
    //  {
    //      doorbell_codec_read(buf,2048);
    //      doorbell_codec_write(buf,2048);
    //  }

    //mqtt测试
    doorbell_mqtt_init();
    // mqtt_cmd_t cmd = {
    //     .mqtt_cmd="on",
    //     .mqtt_cmd_callback=led_on,
    //     NULL};
    // doorbell_mqtt_register_cmd(&cmd);
    // memcpy(cmd.mqtt_cmd,"off",4);
    // cmd.mqtt_cmd_callback=led_off;
    // doorbell_mqtt_register_cmd(&cmd);

    // 摄像头测试
    doorbell_camera_init();
    // for (uint8_t i = 0; i < 5; i++)
    // {
    // camera_fb_t *fb = doorbell_camera_capture();
    //     if (fb)
    //     {
    //         ESP_LOGI(TAG, "Picture capture done:%u",fb->len);
    //         doorbell_camera_release(fb);
    //     }
    // }
    
    doorbell_wsclient_init();

    doorbell_wsclient_register_sound_callback(upcoming_sound_callback,NULL);

    mqtt_cmd_t cmd={
        .mqtt_cmd_arg=NULL,
        .mqtt_cmd="switch",
        .mqtt_cmd_callback=switch_callback
    };
    doorbell_mqtt_register_cmd(&cmd);

}