#include "doorbell_camera.h"

#define TAG "CAMERA"

void doorbell_camera_init(void)
{
    camera_config_t camera_config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = -1,
        .pin_sccb_scl = -1,
        .sccb_i2c_port = 0,

        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,

        .xclk_freq_hz = 20000000, // EXPERIMENTAL: Set to 16MHz on ESP32-S2 or ESP32-S3 to enable EDMA mode
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,

        .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
        .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA, For ESP32, do not use sizes above QVGA when not JPEG. The performance of the ESP32-S series has improved a lot, but JPEG mode always gives better frame rates.

        .jpeg_quality = 12,                 // 0-63, for OV series camera sensors, lower number means higher quality
        .fb_count = 2,                      // When jpeg mode is used, if fb_count more than one, the driver will work in continuous mode.
        .fb_location = CAMERA_FB_IN_PSRAM,  // 需要初始化psram
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY // CAMERA_GRAB_LATEST. Sets when buffers should be filled

    };
    // initialize the camera
    ESP_ERROR_CHECK(esp_camera_init(&camera_config));
}

camera_fb_t *doorbell_camera_capture(void)
{
    return esp_camera_fb_get();
}
void doorbell_camera_to_jpeg(camera_fb_t *fb, uint8_t **jpeg_buf, size_t *buf_len)
{
    assert(fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 80, jpeg_buf, buf_len) == true);
}

void doorbell_camera_to_rgb565(camera_fb_t *fb, uint8_t *rgb_buf)
{
    esp_jpeg_image_cfg_t jpeg_cfg = {
        .indata = (uint8_t *)fb->buf,
        .indata_size = fb->len,
        .outbuf = rgb_buf,
        .outbuf_size = 320 * 240 * sizeof(uint16_t),
        .out_format = JPEG_IMAGE_FORMAT_RGB565,
        .out_scale = JPEG_IMAGE_SCALE_0,
        .flags = {
            .swap_color_bytes = 1,
        }};

    // JPEG decode
    esp_jpeg_image_output_t outimg;
    esp_jpeg_decode(&jpeg_cfg, &outimg);
}

void doorbell_camera_release(camera_fb_t *fb)
{
    esp_camera_fb_return(fb);
}
