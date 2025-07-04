#ifndef __DOORBELL_CAMERA_H__
#define __DOORBELL_CAMERA_H__


#include "esp_camera.h"
#include "doorbell_config.h"
#include "jpeg_decoder.h"

void doorbell_camera_init(void);

camera_fb_t*  doorbell_camera_capture(void);

void doorbell_camera_release(camera_fb_t *fb);



#endif /* __DOORBELL_CAMERA_H__ */
