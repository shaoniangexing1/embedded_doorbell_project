#include "doorbell_wifi.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <wifi_provisioning/manager.h>
#include <wifi_provisioning/scheme_ble.h>
#include "qrcode.h"
#include "esp_system.h"

#define TAG "Wifi"
#define DOORBELL_WIFI_SSID "IQOO12"       // wifi用户名
#define DOORBELL_WIFI_PASSWORD "ykkykk55" // wifi密码

#define PROV_QR_VERSION "v1"
#define PROV_TRANSPORT_BLE "ble"
#define QRCODE_BASE_URL "https://espressif.github.io/esp-jumpstart/qrcode.html"

#define WIFI_CONNECTED_BIT (1 << 0)

static EventGroupHandle_t wifi_event_group;

void doorbell_wifi_nvs_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

static void wifi_prov_print_qr(const char *name, const char *username, const char *pop, const char *transport)
{
    if (!name || !transport)
    {
        ESP_LOGW(TAG, "Cannot generate QR code payload. Data missing.");
        return;
    }
    char payload[150] = {0};
    if (pop)
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"pop\":\"%s\",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, pop, transport);
    }
    else
    {
        snprintf(payload, sizeof(payload), "{\"ver\":\"%s\",\"name\":\"%s\""
                                           ",\"transport\":\"%s\"}",
                 PROV_QR_VERSION, name, transport);
    }
    ESP_LOGI(TAG, "Scan this QR code from the provisioning application for Provisioning.");
    esp_qrcode_config_t cfg = ESP_QRCODE_CONFIG_DEFAULT();
    esp_qrcode_generate(&cfg, payload);
    ESP_LOGI(TAG, "If QR code is not visible, copy paste the below URL in a browser.\n%s?data=%s", QRCODE_BASE_URL, payload);
}

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        // 建立wifi连接
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // 连接失败重新连接
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        // wifi连接成功，设置等待事件组
        ESP_LOGI(TAG, "wifi connect success!!");
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_PROV_END)
    {
        //配网结束，删除二维码
        wifi_prov_mgr_deinit();
    }
}

void doorbell_wifi_init(void)
{
    // 1. 创建一个等待连接完成事件组
    wifi_event_group = xEventGroupCreate();
    // 2. NVS初始化
    doorbell_wifi_nvs_init();
    // 3. wifi初始化
    // 3.1创建一个 LwIP 核心任务，并初始化 LwIP 相关工作
    ESP_ERROR_CHECK(esp_netif_init());
    // 3.2创建一个系统事件任务，并初始化应用程序事件的回调函数。
    // 在此情况下，该回调函数唯一的动作就是将事件中继到应用程序任务中。
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 3.3创建有 TCP/IP 堆栈的默认网络接口实例绑定 station。
    esp_netif_create_default_wifi_sta();
    // 3.4创建 Wi-Fi 驱动程序任务，并初始化 Wi-Fi 驱动程序
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    // 4. 创建回调事件
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_t instance_wifi_prov_end;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        WIFI_PROV_END,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_wifi_prov_end));
    // 新增:wifi配网  替换原来的直接配置固定密码
    wifi_prov_mgr_config_t config = {
        .scheme = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM};
    ESP_ERROR_CHECK(wifi_prov_mgr_init(config));
    // 检查设备是否已经配网
    bool is_provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&is_provisioned));
    if (!is_provisioned)
    {
        ESP_LOGI(TAG, "Starting provisioning");

        char service_name[12];
        //
        get_device_service_name(service_name, sizeof(service_name));
        // 指定wifi安全版本1
        wifi_prov_security_t security = WIFI_PROV_SECURITY_1;

        const char *pop = "abcd1234";

        wifi_prov_security1_params_t *sec_params = pop;
        const char *username = NULL;
        const char *service_key = NULL;

        uint8_t custom_service_uuid[] = {
            /* LSB <---------------------------------------
             * ---------------------------------------> MSB */
            0xb4,
            0xdf,
            0x5a,
            0x1c,
            0x3f,
            0x6b,
            0xf4,
            0xbf,
            0xea,
            0x4a,
            0x82,
            0x03,
            0x04,
            0x90,
            0x1a,
            0x02,
        };
        wifi_prov_scheme_ble_set_service_uuid(custom_service_uuid);
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(security, (const void *)sec_params, service_name, service_key));
        wifi_prov_print_qr(service_name, username, pop, PROV_TRANSPORT_BLE);
    }
    else
    {
        wifi_prov_mgr_deinit();
        // 5.1wifi模式设置为station模式
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        /**
         // 5.2station基本配置
         wifi_config_t wifi_config = {
             .sta = {
                 .ssid = DOORBELL_WIFI_SSID,         // 要连接的wifi用户名
                 .password = DOORBELL_WIFI_PASSWORD, // 要连接的wifi密码
             },
         };
         ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
         */
        // 启动wifi
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "wifi_init_sta finished.");
    }

    // 5. WIFI配置

    // 等待wifi连接成功
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, false, true, portMAX_DELAY);
}

void doorbell_wifi_reset_provisioning(void)
{
    //清除存储的密码与ssid
    ESP_ERROR_CHECK(esp_wifi_restore());
    esp_restart();
}
