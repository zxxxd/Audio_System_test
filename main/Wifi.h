#ifndef MAIN_WIFI_H_
#define MAIN_WIFI_H_


void initialise_wifi(void);
extern EventGroupHandle_t systemstate_event;	//系统的事件组
extern EventGroupHandle_t s_wifi_event_group;	//wifi的事件组

#endif /* MAIN_WIFI_H_ */
