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
#include "Wifi.h"
#include "button.h"
//中断里不能放printf，否则会重启

#define LED_TASK_SIZE 4096
#define LED_TASK_PRIO 1
TaskHandle_t LED_TASK_Handle = NULL;
EventGroupHandle_t systemstate_event;


void app_main()
{
	static uint8_t ucParameterToPass;

	ESP_ERROR_CHECK( nvs_flash_init() );
	systemstate_event = xEventGroupCreate();
	xEventGroupClearBits(systemstate_event, 0x00);
	xTaskCreate(led_task,"led task",LED_TASK_SIZE,&ucParameterToPass,LED_TASK_PRIO,&LED_TASK_Handle);
	initialise_wifi();
	xTaskCreate(button_task,"button_task", 2048, NULL,2,NULL);
	//xTaskCreate()
	while(1)
	{
		printf ("hello\n");
		vTaskDelay(5000 / portTICK_RATE_MS);
	}
}



