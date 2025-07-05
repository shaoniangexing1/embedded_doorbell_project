#ifndef __DOORBELL_LCD_ST7789_H__
#define __DOORBELL_LCD_ST7789_H__


#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "doorbell_config.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "string.h"
#include "driver/gpio.h"

void doorbell_lcd_st7789_init(void);

void doorbell_lcd_st7789_on(void);
void doorbell_lcd_st7789_off(void);

void doorbell_lcd_st7789_display(void* image_data);


#endif /* __DOORBELL_LCD_ST7789_H__ */
