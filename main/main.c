#include <stdio.h>

#include "doorbell_button.h"
#include "doorbell_led.h"
#include "doorbell_codec.h"
#include "doorbell_wifi.h"

#include "string.h"
#include "esp_log.h"

#define TAG "Main"

extern uint8_t dingdong_start[] asm("_binary_output_pcm_start");
extern uint8_t dingdong_end[] asm("_binary_output_pcm_end");

static LED_COLOR led_color = {
    .blue = 0x0005,
    .green = 0x0005,
    .red = 0x0005};

static void button_cb(void *button_handle, void *arg)
{
    // ESP_LOGI(TAG, "button event: %s", (char*)arg);
    switch ((BUTTON_STATUS_T)arg)
    {
    case BUTTON_STATUS_SING:
        { /* 短按开led，发出dingdong声音 */
            doorbell_led_on_off(2, led_color, LED_ON);
            doorbell_codec_write((void *)dingdong_start, dingdong_end - dingdong_start);
            ESP_LOGI(TAG, "single click");
        }
        break;
    case BUTTON_STATUS_DOUBLE:
        { /* 双击关led */
            doorbell_led_on_off(2, led_color, LED_OFF);
            ESP_LOGI(TAG, "double click");
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
    doorbell_button_front_register(BUTTON_PRESS_REPEAT_DONE, button_cb, (void*)BUTTON_STATUS_DOUBLE);

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

    return;
}