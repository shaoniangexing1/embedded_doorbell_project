#ifndef __DOORBELL_CODEC_H__
#define __DOORBELL_CODEC_H__

#include "esp_codec_dev.h"

void doorbell_codec_init(void);


void doorbell_codec_open(void);

void doorbell_codec_close(void);


void doorbell_codec_read(void *read_buffer,uint16_t len);

void doorbell_codec_write(void *write_buffer,uint16_t len);

#endif /* __DOORBELL_CODEC_H__ */
