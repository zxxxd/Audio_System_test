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
#include "system_state.h"

#define SmartCfg_TASK_SIZE 4096
#define SmartCfg_TASK_PRIO 1
TaskHandle_t SmartCfg_TASK_Handle = NULL;


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;		//置1则为已获取IP
static const int ESPTOUCH_DONE_BIT = BIT1;	//
static const char *TAG = "wifi";

wifi_interface_t wifi_interface;
wifi_config_t wifi_config;

//wifi 状态机运转，根据wifi状态运行相应分支任务
static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	EventBits_t sys_state;
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip:%s",
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
    	sys_state = xEventGroupGetBits(systemstate_event);
    	if ((sys_state & SS_Bit0) == 0)
    	{
    		printf("auto reconnect!\n");
    		esp_wifi_connect();		//掉线自动重连
    	}
    	xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}


esp_err_t initialise_wifi(void)
{
	bool auto_connect_flag;
	int i;		//判断wifi SSID是否为空
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    tcpip_adapter_init();
    //s_wifi_event_group = xEventGroupCreate();
    //ESP_ERROR_CHECK( esp_event_loop_init(SmartCfg_event_handler, NULL) );
    ESP_ERROR_CHECK( esp_wifi_init(&wifi_cfg) );
    ESP_ERROR_CHECK(esp_wifi_get_config(wifi_interface, &wifi_config));
    for (i = 0;i<32;i++)
    {
    	if (wifi_config.sta.ssid[i] != 0)
    		break;
    }
    if (i == 32)
    {
    	printf("wifi SSID is NULL!\n");
    	printf ("i = %d\n",i);
        xTaskCreate(smart_cfg_task, "smartconfig_example_task", 4096, NULL, 3, NULL);
    }
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_get_auto_connect(&auto_connect_flag));
    if (auto_connect_flag == 0)
    	ESP_ERROR_CHECK(esp_wifi_set_auto_connect(1));
    ESP_ERROR_CHECK(esp_wifi_set_config(wifi_interface, &wifi_config));
    ESP_ERROR_CHECK( esp_wifi_start());

    return ESP_OK;
}

static void sc_callback(smartconfig_status_t status, void *pdata);
void smart_cfg_task(void *parm)
{
    EventBits_t uxBits;
    printf("smart_cfg_task runs.\n");
    xEventGroupSetBits(systemstate_event,SS_Bit0);
    ESP_ERROR_CHECK( esp_smartconfig_set_type(SC_TYPE_ESPTOUCH) );
    ESP_ERROR_CHECK( esp_smartconfig_start(sc_callback) );
    while (1) {
        uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
				//等待 wifi_event_group 中两个位中某一个置一，然后清零两个位
        		//等待获取IP或配置完成
        if(uxBits & WIFI_CONNECTED_BIT) {
            ESP_LOGI(TAG, "WiFi Connected to ap");
        }
        if(uxBits & ESPTOUCH_DONE_BIT) {
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            xEventGroupClearBits(systemstate_event,SS_Bit0);
            vTaskDelete(NULL);
        }
    }
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
            xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
            break;
        default:
            break;
    }
}
