#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "sdkconfig.h"
#include "led.h"

#define LED_TASK_SIZE 4096
#define LED_TASK_PRIO 1

EventGroupHandle_t systemstate_event;


void app_main()
{
	static uint8_t ucParameterToPass;
	TaskHandle_t LEDTASK_Handle = NULL;
	ESP_ERROR_CHECK( nvs_flash_init() );
	systemstate_event = xEventGroupCreate();
	xEventGroupClearBits(systemstate_event, 0x00);
	ESP_ERROR_CHECK( xTaskCreate(led_task,"led task",LED_TASK_SIZE,
			&ucParameterToPass,LED_TASK_PRIO,&LEDTASK_Handle) );
	while(1)
	{
		if (1)
		{

		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);

	}
}

