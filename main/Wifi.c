#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "tcpip_adapter.h"
#include "esp_smartconfig.h"
#include "Wifi.h"

#define SmartCfg_TASK_SIZE 4096
#define SmartCfg_TASK_PRIO 1
TaskHandle_t SmartCfg_TASK_Handle = NULL;

/* FreeRTOS event group to signal when we are connected & ready to make a request */
EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;
static const char *TAG = "sc";

void SmartCfg_task(void * parm);

esp_err_t SmartCfg_event_handler(void *ctx, system_event_t *event)
{
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        xTaskCreate(SmartCfg_task, "SmartCfg_task", SmartCfg_TASK_SIZE, NULL, SmartCfg_TASK_PRIO, &SmartCfg_TASK_Handle);
        xEventGroupSetBits(systemstate_event, 0x04);
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(s_wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        esp_wifi_connect();
        xEventGroupClearBits(s_wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

void initialise_wifi(void)
{
	bool auto_connect_flag;
    tcpip_adapter_init();
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(SmartCfg_event_handler, NULL) );

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_get_auto_connect(&auto_connect_flag));
    if (auto_connect_flag == 0)
    	ESP_ERROR_CHECK(esp_wifi_set_auto_connect(1));


}

static void sc_callback(smartconfig_status_t status, void *pdata)
{
    switch (status) {
        case SC_STATUS_WAIT:
            ESP_LOGI(TAG, "SC_STATUS_WAIT");
            break;
        case SC_STATUS_FIND_CHANNEL:
            ESP_LOGI(TAG, "SC_STATUS_FINDING_CHANNEL");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
            ESP_LOGI(TAG, "SC_STATUS_GETTING_SSID_PSWD");
            break;
        case SC_STATUS_LINK:
            ESP_LOGI(TAG, "SC_STATUS_LINK");
            wifi_config_t *wifi_config = pdata;
            ESP_LOGI(TAG, "SSID:%s", wifi_config->sta.ssid);
            ESP_LOGI(TAG, "PASSWORD:%s", wifi_config->sta.password);
            ESP_ERROR_CHECK( esp_wifi_disconnect() );
            ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, wifi_config) );
            ESP_ERROR_CHECK( esp_wifi_connect() );
            break;
        case SC_STATUS_LINK_OVER:
            ESP_LOGI(TAG, "SC_STATUS_LINK_OVER");
            if (pdata != NULL) {
                uint8_t phone_ip[4] = { 0 };
                memcpy(phone_ip, (uint8_t* )pdata, 4);
                ESP_LOGI(TAG, "Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);
            }
            xEventGroupSetBits(s_wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}

void SmartCfg_task(void * parm)
{
    EventBits_t uxBits;
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(s_wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        if(uxBits & CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            xEventGroupClearBits(systemstate_event, 0x04);
            vTaskDelete(NULL);
        }
    }
}

