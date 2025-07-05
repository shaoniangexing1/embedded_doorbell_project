#ifndef __DOORBELL_CAMERA_H__
#define __DOORBELL_CAMERA_H__


#include "esp_camera.h"
#include "doorbell_config.h"
#include "jpeg_decoder.h"

void doorbell_camera_init(void);

camera_fb_t*  doorbell_camera_capture(void);

void doorbell_camera_to_jpeg(camera_fb_t *fb, uint8_t **jpeg_buf, size_t *buf_len);

void doorbell_camera_to_rgb565(camera_fb_t *fb, uint8_t *rgb_buf);

void doorbell_camera_release(camera_fb_t *fb);



#endif /* __DOORBELL_CAMERA_H__ */
