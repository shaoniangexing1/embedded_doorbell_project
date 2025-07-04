#ifndef __DOORBELL_CONFIG_H__
#define __DOORBELL_CONFIG_H__

// mqtt的UIR配置
#define MQTT_SERVER_IP "192.168.185.86"
#define MQTT_PORT "1883"
#define MQTT_SCHEMA "ws://"

#define LED_STRIP_GPIO_PIN 46

#define CODEC_I2C_SDA_GPIO_PIN 0
#define CODEC_I2C_SCL_GPIO_PIN 1

#define CODEC_I2S_BCK_GPIO_PIN 2
#define CODEC_I2S_MCK_GPIO_PIN 3
#define CODEC_I2S_DI_GPIO_PIN 4
#define CODEC_I2S_WS_GPIO_PIN 5
#define CODEC_I2S_DO_GPIO_PIN 6

#define CODEC_I2S_SAMPLE_RATE 16000

#define CODEC_PA_GPIO_PIN 7

typedef enum BUTTON_STATUS
{
    BUTTON_STATUS_SING,
    BUTTON_STATUS_DOUBLE,
    BUTTON_STATUS_LONG,
} BUTTON_STATUS_T;

// WROVER-KIT PIN Map
#define CAM_PIN_PWDN -1  // power down is not used
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 37
#define CAM_PIN_SIOD 0
#define CAM_PIN_SIOC 1

#define CAM_PIN_D7 38
#define CAM_PIN_D6 36
#define CAM_PIN_D5 35
#define CAM_PIN_D4 33
#define CAM_PIN_D3 10
#define CAM_PIN_D2 12
#define CAM_PIN_D1 11
#define CAM_PIN_D0 9
#define CAM_PIN_VSYNC 41
#define CAM_PIN_HREF 39
#define CAM_PIN_PCLK 34

#endif /* __DOORBELL_CONFIG_H__ */
