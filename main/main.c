#include <stdio.h>

#include "doorbell_button.h"
#include "doorbell_led.h"
#include "doorbell_codec.h"
#include "doorbell_wifi.h"
#include "doorbell_mqtt.h"
#include "doorbell_camera.h"
#include "doorbell_wsclient.h"
#include "doorbell_lcd_st7789.h"
#include "doorbell_ota.h"
#include "freertos/FreeRTOS.h"

#include "string.h"
#include "esp_log.h"
#include "doorbell_config.h"
#define TAG "Main"

extern uint8_t dingdong_start[] asm("_binary_output_pcm_start");
extern uint8_t dingdong_end[] asm("_binary_output_pcm_end");

static LED_COLOR led_color = {
    .blue = 0x0005,
    .green = 0x0005,
    .red = 0x0005};
// 按键事件判断
static bool is_streaming = false;
static bool is_lcd_streaming = false;
// 定义一个指向camera_fb_t类型的指针fb，用于存储相机帧缓冲区的地址
static camera_fb_t *fb = NULL;

// 定义一个用于获取帧缓冲区的信号量句柄fb_get_sem
static SemaphoreHandle_t fb_get_sem = NULL;

// 定义一个用于释放帧缓冲区的信号量句柄fb_release_sem
static SemaphoreHandle_t fb_release_sem = NULL;

static uint8_t *img_buf = NULL;

static void lcd_streaming_task(void *arg)
{
    while (1)
    {
        if (is_lcd_streaming || is_streaming)
        {
            fb = doorbell_camera_capture();
            xSemaphoreGive(fb_get_sem);
            if (is_lcd_streaming)
            {
                doorbell_camera_to_rgb565(fb, img_buf);
                doorbell_lcd_st7789_display(img_buf);
            }
            if (is_streaming)
            {
                xSemaphoreTake(fb_release_sem, portMAX_DELAY);
            }
            doorbell_camera_release(fb);
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
static void sound_streaming_task(void *arg)
{
     void *buf = malloc(2048);
    while (is_streaming)
    {
        if (doorbell_wsclient_is_sound_ready())
        {
            doorbell_codec_read(buf, 2048);
            doorbell_wsclient_send_message_sound(buf, 2048);
            
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    free(buf);
    vTaskDelete(NULL);
}
void image_streaming_task(void *arg)
{
    while (is_streaming)
    {
        if (doorbell_wsclient_is_image_ready())
        {
            xSemaphoreTake(fb_get_sem,portMAX_DELAY);
            doorbell_wsclient_send_message_image(fb->buf, fb->len);
            xSemaphoreGive(fb_release_sem);
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
    vTaskDelete(NULL);
}
static void switch_callback(void *arg)
{
    if (is_streaming)
    {
        is_streaming = false;
        /* 关闭推流 */
        doorbell_wsclint_stop();
    }
    else
    {
        is_streaming = true;
        /* 开启推流 */
        doorbell_wsclient_start();
        xTaskCreate(sound_streaming_task, "sound_task", 4096, NULL, 5, NULL);
        xTaskCreate(image_streaming_task, "image_task", 4096, NULL, 5, NULL);
    }
}
static void ota_callback(void *arg)
{
    doorbell_ota_process();
}

static void upcoming_sound_callback(void *arg, void *data, size_t len)
{
    if (is_streaming)
        doorbell_codec_write(data, len);
}

// lcd面板下方按键回调函数
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

// lcd背面按键回调
static void button_back_cb(void *button_handle, void *arg)
{
    if (is_lcd_streaming)
    {
        is_lcd_streaming = false;
        // 关闭背光
        vTaskDelay(50);
        doorbell_lcd_st7789_off();
        doorbell_led_on_off(0, led_color, LED_OFF);
    }
    else
    {
        // 打开背光
        doorbell_lcd_st7789_on();
        doorbell_led_on_off(0, led_color, LED_OFF);
        is_lcd_streaming = true;
    }
}

void app_main(void)
{

    // 按键模块测试，长按和短按
    doorbell_button_init();

    doorbell_button_front_register(BUTTON_SINGLE_CLICK, button_cb, (void *)BUTTON_STATUS_SING);
    doorbell_button_front_register(BUTTON_LONG_PRESS_START, button_cb, (void *)BUTTON_STATUS_LONG);
    doorbell_button_brack_register(BUTTON_SINGLE_CLICK, button_back_cb, (void *)BUTTON_STATUS_SING);

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

    // mqtt测试
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

    img_buf = (uint8_t *)heap_caps_malloc(320 * 240 * 2, MALLOC_CAP_SPIRAM);
    assert(img_buf);
    doorbell_lcd_st7789_init();
    fb_get_sem = xSemaphoreCreateBinary();
    fb_release_sem = xSemaphoreCreateBinary();
    xTaskCreate(lcd_streaming_task, "lcd_streaming_task", 4096, NULL, 5, NULL);

    doorbell_wsclient_register_sound_callback(upcoming_sound_callback, NULL);

    mqtt_cmd_t cmd = {
        .mqtt_cmd_arg = NULL,
        .mqtt_cmd = "switch",
        .mqtt_cmd_callback = switch_callback};
    doorbell_mqtt_register_cmd(&cmd);
    memcpy(cmd.mqtt_cmd, "ota", 4);
    cmd.mqtt_cmd_callback = ota_callback;
    doorbell_mqtt_register_cmd(&cmd);
    ESP_LOGI(TAG, "Current version: %s", OTA_VER);
    // doorbell_lcd_st7789_init();

    // doorbell_lcd_st7789_on();
    // while (1)
    // {
    //     camera_fb_t *fb=doorbell_camera_capture();
    //     doorbell_lcd_st7789_display(fb->buf);
    //     doorbell_camera_release(fb);

    // }
}