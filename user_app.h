#ifndef USER_APP_H
#define USER_APP_H   // guard ANTES dos includes

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define LIMIAR_TEMP  50.0 

extern QueueHandle_t     xQueueTemp;
extern SemaphoreHandle_t xMutexBuf;
extern SemaphoreHandle_t xMutexUART;
extern SemaphoreHandle_t xSemAlarm;
extern char              buffer[32];

void init_task(void);
void vTaskADC    (void *pvParameters);
void vTaskControl(void *pvParameters);
void vTaskUART   (void *pvParameters);
void vTaskAlarm  (void *pvParameters);

#endif