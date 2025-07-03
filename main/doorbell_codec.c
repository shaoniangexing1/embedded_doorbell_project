#include "doorbell_codec.h"
#include "doorbell_config.h"

#include "esp_codec_dev_defaults.h"
#include "driver/i2s_pdm.h"
#include "driver/i2s.h"
#include "driver/i2s_std.h"
#include "driver/i2s_tdm.h"
#include "driver/i2c.h"
#include "es8311_codec.h"

/**
 * @brief 初始化I2C接口用于门铃编解码器
 * 
 * 本函数配置并初始化I2C驱动，以使能门铃编解码器的通信门铃编解码器
 * 通过I2C接口进行数据传输，例如控制命令或音频数据的传输
 * 
 * @return int -1表示参数配置失败，其他值表示驱动安装失败
 */
static void doorbell_codec_i2c_init(void)
{
    // 配置I2C参数
    i2c_config_t i2c_cfg = {
        .mode = I2C_MODE_MASTER, // 设置I2C模式为主模式
        .sda_io_num = CODEC_I2C_SDA_GPIO_PIN, // 设置SDA引脚编号
        .scl_io_num = CODEC_I2C_SCL_GPIO_PIN, // 设置SCL引脚编号
        .sda_pullup_en = GPIO_PULLUP_ENABLE, // 启用SDA引脚的上拉电阻
        .scl_pullup_en = GPIO_PULLUP_ENABLE, // 启用SCL引脚的上拉电阻
        .master.clk_speed = 100000, // 设置I2C时钟频率为100kHz
    };

    // 配置I2C参数
    i2c_param_config(I2C_NUM_0, &i2c_cfg);
    

    // 安装I2C驱动
    i2c_driver_install(I2C_NUM_0, i2c_cfg.mode, 0, 0, 0);
}
static void doorbell_codec_i2s_init(audio_codec_i2s_cfg_t *i2s_cfg)
{
    // 初始化I2S通道配置，设置为默认的主模式
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear_after_cb=true;//不播放数据时候自动清除
    // 配置I2S标准模式：
    // - 时钟配置使用预设的采样率（16000Hz）
    // - 使用Philips标准模式，16位数据宽度，立体声模式
    // - 设置相关GPIO引脚
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(CODEC_I2S_SAMPLE_RATE), // 时钟配置，音频的采样率16000
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_BITS_PER_CHAN_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = CODEC_I2S_MCK_GPIO_PIN,
            .bclk = CODEC_I2S_BCK_GPIO_PIN,
            .ws = CODEC_I2S_WS_GPIO_PIN,
            .dout = CODEC_I2S_DO_GPIO_PIN,
            .din = CODEC_I2S_DI_GPIO_PIN},
    };

    // 创建I2S通道，并将发送和接收句柄保存到i2s_cfg中
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, (i2s_chan_handle_t*)&i2s_cfg->tx_handle, (i2s_chan_handle_t*)&i2s_cfg->rx_handle));

    // 初始化发送通道为标准模式
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(i2s_cfg->tx_handle, &std_cfg));
    // 初始化接收通道为标准模式
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(i2s_cfg->rx_handle, &std_cfg));

    // 启用发送通道
    i2s_channel_enable(i2s_cfg->tx_handle);
    // 启用接收通道
    i2s_channel_enable(i2s_cfg->rx_handle);
}

static esp_codec_dev_handle_t codec_dev_handle = NULL;
// 初始化门铃编解码器
void doorbell_codec_init(void)
{
    // 生成数据通道
    // rx_handle和tx_handle来源于i2s初始化
    audio_codec_i2s_cfg_t i2s_cfg = {
        .rx_handle = NULL,
        .tx_handle = NULL,
    };
    doorbell_codec_i2s_init(&i2s_cfg);
    const audio_codec_data_if_t *data_if = audio_codec_new_i2s_data(&i2s_cfg);
    
    // 生成控制通道
    doorbell_codec_i2c_init();
    audio_codec_i2c_cfg_t i2c_cfg = {
        .port = I2C_NUM_0,
        .addr = ES8311_CODEC_DEFAULT_ADDR,
    };
    const audio_codec_ctrl_if_t *out_ctrl_if = audio_codec_new_i2c_ctrl(&i2c_cfg);
    const audio_codec_gpio_if_t *gpio_if = audio_codec_new_gpio();

    // ES8311编解码器配置
    es8311_codec_cfg_t es8311_cfg = {
        .codec_mode = ESP_CODEC_DEV_WORK_MODE_BOTH,
        .ctrl_if = out_ctrl_if,
        .gpio_if = gpio_if,
        .pa_pin = CODEC_PA_GPIO_PIN,
        .use_mclk = true,
    };
    const audio_codec_if_t *out_codec_if = es8311_codec_new(&es8311_cfg);

    // 生成音频
    esp_codec_dev_cfg_t codec_dev_cfg = {
        .dev_type = ESP_CODEC_DEV_TYPE_IN_OUT,
        .codec_if = out_codec_if,
        .data_if = data_if};

    codec_dev_handle = esp_codec_dev_new(&codec_dev_cfg);

    
}

/**
 * @brief 打开门铃编解码器
 * 
 * 本函数用于初始化并打开门铃编解码器设备。它配置了采样信息结构体，
 * 包括每样本位数、采样率、声道数和声道掩码，然后调用ESP-IDF的编解码器
 * 设备驱动函数来打开编解码器设备。
 */
void doorbell_codec_open(void)
{
    // 设置输出音量
    esp_codec_dev_set_out_vol(codec_dev_handle, 50);
    //声音增益,值小防止炸麦
    esp_codec_dev_set_in_gain(codec_dev_handle,(float)10);
    // 配置编解码器的采样信息
    esp_codec_dev_sample_info_t fs = {
        .bits_per_sample = I2S_BITS_PER_CHAN_16BIT, // 每样本16位
        .sample_rate = CODEC_I2S_SAMPLE_RATE,       // 采样率为预定义的CODEC_I2S_SAMPLE_RATE
        .channel = 1,                                // 单声道
        .channel_mask = ESP_CODEC_DEV_MAKE_CHANNEL_MASK(0) // 选择声道0（左声道）
    };
    // 打开编解码器设备，并检查操作是否成功
    ESP_ERROR_CHECK(esp_codec_dev_open(codec_dev_handle, &fs));
}

/**
 * @brief 关闭门铃编解码器
 * 
 * 本函数用于关闭与门铃相关的音频编解码器设备。在不再需要编解码器时，
 * 调用此函数可以释放编解码器设备占用的资源。
 */
void doorbell_codec_close(void)
{
    // 关闭编解码器设备
    esp_codec_dev_close(codec_dev_handle);
}
/**
 * 从门铃编解码器中读取数据
 * 
 */
void doorbell_codec_read(void *read_buffer, uint16_t len)
{
    ESP_ERROR_CHECK(esp_codec_dev_read(codec_dev_handle, read_buffer, len));
}
/**
 * 从门铃编解码器中写入数据
 * 
 */
void doorbell_codec_write(void *write_buffer, uint16_t len)
{
    ESP_ERROR_CHECK(esp_codec_dev_write(codec_dev_handle, write_buffer, len));
}
