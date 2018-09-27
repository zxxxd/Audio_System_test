/*
 * led.h
 *
 *  Created on: 2018年9月18日
 *      Author: zxd
 */
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "freertos/event_groups.h"
#ifndef MAIN_LED_H_
#define MAIN_LED_H_

void led_task(void * pvParameters);
//extern EventGroupHandle_t systemstate_event;	//系统的事件组

#endif /* MAIN_LED_H_ */
