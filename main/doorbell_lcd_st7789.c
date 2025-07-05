#include "doorbell_lcd_st7789.h"

#define TAG "lcd_st7789"

#define LCD_HOST SPI2_HOST
#define LCD_BK_LIGHT_ON 1
#define LCD_BK_LIGHT_OFF (!LCD_BK_LIGHT_ON)

#define LCD_H_RES_320 320
#define LCD_V_RES_240 240

#define LCD_PARALLEL_LINES 5 // 一次刷新5行

static esp_lcd_panel_handle_t lcd_panel_handle = NULL;

static uint16_t *s_lines[2];

// 初始化门铃的ST7789 LCD驱动
void doorbell_lcd_st7789_init(void)
{
    // 配置LCD背光使用的GPIO
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = 1ULL << LCD_PIN_BG};
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config)); // 初始化GPIO配置
    // 关闭背光以避免在初始化LCD面板驱动程序时显示不可预测的画面（不同的LCD屏幕可能需要不同的电平）
    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BG, LCD_BK_LIGHT_OFF));
    // 配置SPI总线参数
    spi_bus_config_t buscfg = {
        .sclk_io_num = LCD_PIN_SPI_SCLK,
        .mosi_io_num = LCD_PIN_SPI_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_PARALLEL_LINES * LCD_H_RES_320 * 2 + 8};

    ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO)); // 初始化SPI总线

    // 初始化LCD面板的SPI通信配置
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_PIN_DC,
        .cs_gpio_num = LCD_PIN_SPI_CS,
        .pclk_hz = 40000000, // 26MHz或40MHz
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle)); // 将LCD连接到SPI总线

    // 初始化LCD面板配置
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_PIN_RST,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel = 16,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel_handle)); // 初始化LCD配置

    // 重置显示屏
    ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));

    // 初始化LCD面板
    ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));
    // 翻转屏幕
    ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_panel_handle, true));

    // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(lcd_panel_handle, true));

    // 分配像素缓冲区的内存
    for (int i = 0; i < 2; i++)
    {
        s_lines[i] = heap_caps_malloc(LCD_H_RES_320 * LCD_PARALLEL_LINES * sizeof(uint16_t), MALLOC_CAP_DMA);
        assert(s_lines[i] != NULL);
    }
    // //======================================

    // gpio_config_t bk_gpio_config = {
    //     .mode = GPIO_MODE_OUTPUT,
    //     .pin_bit_mask = 1ULL << LCD_PIN_BG};
    // // Initialize the GPIO of backlight
    // ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    // // 关掉背光
    // ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BG, 0));

    // spi_bus_config_t buscfg = {
    //     .sclk_io_num = LCD_PIN_SPI_SCLK,
    //     .mosi_io_num = LCD_PIN_SPI_MOSI,
    //     .miso_io_num = -1,
    //     .quadwp_io_num = -1,
    //     .quadhd_io_num = -1,
    //     .max_transfer_sz = LCD_PARALLEL_LINES * LCD_H_RES_320 * 2 + 8};
    // // Initialize the SPI bus
    // ESP_ERROR_CHECK(spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // esp_lcd_panel_io_handle_t io_handle = NULL;
    // esp_lcd_panel_io_spi_config_t io_config = {
    //     .dc_gpio_num = LCD_PIN_DC,
    //     .cs_gpio_num = LCD_PIN_SPI_CS,
    //     .pclk_hz = 40 * 1000 * 1000,
    //     .lcd_cmd_bits = 8,
    //     .lcd_param_bits = 8,
    //     .spi_mode = 0,
    //     .trans_queue_depth = 10,
    // };

    // // Attach the LCD to the SPI bus
    // ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    // esp_lcd_panel_dev_config_t panel_config = {
    //     .reset_gpio_num = LCD_PIN_RST,
    //     .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
    //     .bits_per_pixel = 16,
    // };
    // // Initialize the LCD configuration
    // ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &lcd_panel_handle));

    // // Reset the display
    // ESP_ERROR_CHECK(esp_lcd_panel_reset(lcd_panel_handle));

    // // Initialize LCD panel
    // ESP_ERROR_CHECK(esp_lcd_panel_init(lcd_panel_handle));

    // ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(lcd_panel_handle, true));

    // // ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));

    // // 申请屏幕缓存
    // for (int i = 0; i < 2; i++)
    // {
    //     s_lines[i] = heap_caps_malloc(LCD_H_RES_320 * LCD_PARALLEL_LINES * sizeof(uint16_t), MALLOC_CAP_DMA);
    //     assert(s_lines[i] != NULL);
    // }
}

void doorbell_lcd_st7789_on(void)
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, true));
    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BG, LCD_BK_LIGHT_ON));
}
void doorbell_lcd_st7789_off(void)
{
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(lcd_panel_handle, false));
    ESP_ERROR_CHECK(gpio_set_level(LCD_PIN_BG, LCD_BK_LIGHT_OFF));
}

/**
 * @brief 显示ST7789 LCD屏幕内容。
 *
 * 该函数负责在ST7789 LCD屏幕上实现特定的显示效果。
 * 使用帧内逐行计算和显示的方法来实现图像旋转效果。
 *
 * @param image LCD面板的句柄，用于与LCD进行交互。
 */
void doorbell_lcd_st7789_display(void *image_data)
{
    uint8_t cuurent_line = 0;
    for (size_t i = 0; i < LCD_V_RES_240; i += LCD_PARALLEL_LINES)
    {
        memcpy(s_lines[cuurent_line], image_data + i * LCD_H_RES_320 * sizeof(uint16_t), LCD_H_RES_320 * LCD_PARALLEL_LINES * sizeof(uint16_t));
        // 发送计算后的数据
        esp_lcd_panel_draw_bitmap(lcd_panel_handle, 0, i, LCD_H_RES_320, i + LCD_PARALLEL_LINES, s_lines[cuurent_line]);
        //实现两个缓冲区切换，避免数据丢失
        cuurent_line = 1-cuurent_line;
    }
    // uint8_t current_line = 0;
    // for (size_t i = 0; i < LCD_V_RES_240; i += LCD_PARALLEL_LINES)
    // {
    //     // 将img_data的数据拷贝到DMA buf
    //     memcpy(s_lines[current_line], image_data + i * LCD_H_RES_320 * sizeof(uint16_t), LCD_H_RES_320 * LCD_PARALLEL_LINES * sizeof(uint16_t));
    //     esp_lcd_panel_draw_bitmap(lcd_panel_handle, 0, i, LCD_H_RES_320, i + LCD_PARALLEL_LINES, s_lines[current_line]);
    //     current_line = 1 - current_line;
    // }
}
