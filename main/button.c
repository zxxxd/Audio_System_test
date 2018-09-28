
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "button.h"
#include "Wifi.h"
#include "system_state.h"

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

static void gpio_task_example(void* arg)
{
	int i = 0;
	EventBits_t uxBits;
	for(;;) {
		uxBits = xEventGroupWaitBits(systemstate_event,SS_Bit4,pdTRUE,pdTRUE,1000 / portTICK_PERIOD_MS);	//接收按键中断标志
		if ((( uxBits & SS_Bit4 ) != 0) && (!gpio_get_level(GPIO_MODE_PIN)))
		{
			vTaskDelay(1000 / portTICK_PERIOD_MS);
			if (!gpio_get_level(GPIO_MODE_PIN))
			{
				printf("Mode pin: %d\n", i++);		//检测到按键按下

			}
		}
	}
}

void button_task(void *parm)
{
	gpio_config_t io_conf;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	io_conf.intr_type = GPIO_INTR_NEGEDGE;	//下降沿中断
	io_conf.pin_bit_mask = GPIO_PIN_ISR;
	io_conf.mode = GPIO_MODE_INPUT;
	gpio_config(&io_conf);

	gpio_evt_queue = xQueueCreate(10, sizeof(uint32_t));

	xTaskCreate(gpio_task_example, "gpio_task_example", 4096, NULL, 1, NULL);

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	gpio_isr_handler_add(GPIO_MODE_PIN, gpio_isr_handler, (void*)GPIO_MODE_PIN);

	while(1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
