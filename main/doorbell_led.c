#include "doorbell_led.h"

#define TAG "LED"
static led_strip_handle_t led_strip_handle=NULL;

void doorbell_led_init(void)
{
    led_strip_config_t strip_config={
        .strip_gpio_num = LED_STRIP_GPIO_PIN,//gpio引脚
        .max_leds = 2,//led数量
        .led_model=LED_MODEL_WS2812,//灯的型号
        .color_component_format=LED_STRIP_COLOR_COMPONENT_FMT_GRB,//RGB模式显示
        .flags={
            .invert_out=false,//不翻转输出
        }
    };
    led_strip_rmt_config_t rmt_config={
        .clk_src=RMT_CLK_SRC_DEFAULT,
        .resolution_hz=10*1000*1000,//RMT计数频率为10MHz
        .mem_block_symbols=64,//RMT内存块大小为64个计数单元
        .flags={
            .with_dma=true,//使用DMA
        }
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config,&rmt_config,&led_strip_handle));
}



void doorbell_led_on_off(uint8_t led_num, LED_COLOR led_color, LED_STATUS led_status)
{
    if (led_status==LED_ON)
    {
        for (uint8_t i = 0; i < led_num; i++)
        {
            ESP_ERROR_CHECK(led_strip_set_pixel(led_strip_handle,i,led_color.red,led_color.green,led_color.blue));
        }
        ESP_ERROR_CHECK(led_strip_refresh(led_strip_handle));
        
    }else if (led_status==LED_OFF)
    {
        ESP_ERROR_CHECK(led_strip_clear(led_strip_handle));
    }
    
}
