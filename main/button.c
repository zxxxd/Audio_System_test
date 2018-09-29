
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "button.h"
#include "Wifi.h"
#include "system_state.h"

extern wifi_interface_t wifi_interface;
extern wifi_config_t wifi_config;
//extern wifi_init_config_t cfg;

#define GPIO_MODE_PIN 21
#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_PIN_ISR  (1ULL<<GPIO_MODE_PIN)
static xQueueHandle gpio_evt_queue = NULL;

static BaseType_t pxTaskWorken, xResult;

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
	xResult = xEventGroupSetBitsFromISR(systemstate_event,SS_Bit4, &pxTaskWorken);	//给按键中断位发送标志
	if (xResult == pdFALSE)		//任务切换失败
	{
		portYIELD_FROM_ISR();
	}
}

void button_task(void *parm)
{
	int i = 0;
	EventBits_t uxBits;
	gpio_config_t io_conf;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;	//下降沿中断
	io_conf.pin_bit_mask = GPIO_PIN_ISR;
	io_conf.mode = GPIO_MODE_INPUT;
	gpio_config(&io_conf);

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	//xTaskCreate(gpio_task_example, "gpio_task_example", 1024, NULL, 1, NULL);

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(GPIO_MODE_PIN, gpio_isr_handler, (void*)GPIO_MODE_PIN);

	for(;;) {
			uxBits = xEventGroupWaitBits(systemstate_event,SS_Bit4,pdTRUE,pdTRUE,1000 / portTICK_PERIOD_MS);	//接收按键中断标志
			if ((( uxBits & SS_Bit4 ) != 0) && (!gpio_get_level(GPIO_MODE_PIN)))
			{
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				if (!gpio_get_level(GPIO_MODE_PIN))
				{
					printf("Mode pin: %d\n", i++);		//检测到按键按下
					ESP_ERROR_CHECK(esp_wifi_disconnect());
					ESP_ERROR_CHECK(esp_wifi_stop());
					ESP_ERROR_CHECK(esp_wifi_deinit());		//下面三行必须亚，否则会连接上一次的wifi
					wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
			        ESP_ERROR_CHECK( esp_wifi_init(&wifi_cfg) );
			        xEventGroupSetBits(systemstate_event,SS_Bit0);
			        xTaskCreate(smart_cfg_task, "smartconfig_example_task", 4096, NULL, 2, NULL);
			        ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
			        ESP_ERROR_CHECK(esp_wifi_set_config(wifi_interface, &wifi_config));
			        ESP_ERROR_CHECK( esp_wifi_start());
				}
			}
	}

}
