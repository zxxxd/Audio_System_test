
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

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
	BaseType_t xHigherPriorityTaskWoken, xResult;
	printf ("detected button!\n");
	//如果中断程序内没有信号量就会 发生重启！！！
    /*xResult = xEventGroupSetBitsFromISR(systemstate_event,SS_Bit4,&xHigherPriorityTaskWoken);
    // Was the message posted successfully?
          if( xResult == pdFALSE )
          {
              // If xHigherPriorityTaskWoken is now set to pdTRUE then a context
              // switch should be requested.  The macro used is port specific and
              // will be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() -
              // refer to the documentation page for the port being used.
              //portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
        	  printf ("Button set Event failed!!\n");
          }
          else
          {
        	  printf ("Button set Event success!\n");
          }*/
}

void button_task(void *parm)
{
	gpio_config_t io_conf;

	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
	io_conf.pin_bit_mask = GPIO_PIN_ISR;
	io_conf.mode = GPIO_MODE_INPUT;


	gpio_config(&io_conf);

	//install gpio isr service
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
	//hook isr handler for specific gpio pin
	gpio_isr_handler_add(GPIO_MODE_PIN, gpio_isr_handler, (void*)GPIO_MODE_PIN);
	gpio_intr_enable(GPIO_PIN_ISR);


	while(1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		printf("Button task.\n");
	}
}
