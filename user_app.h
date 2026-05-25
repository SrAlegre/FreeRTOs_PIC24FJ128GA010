#ifndef USER_APP_H
#define USER_APP_H   // guard ANTES dos includes

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"


extern QueueHandle_t     xQueueTemp;
extern SemaphoreHandle_t xMutexBuf;
extern SemaphoreHandle_t xMutexUART;
extern SemaphoreHandle_t xSemAlarm;
extern char              buffer[32];

extern QueueHandle_t    xQueueAtuador;
// Estados/Comandos para o atuador
typedef enum {
    ATUADOR_OFF = 0,
    ATUADOR_ON
} atuador_cmd_t;

void init_task(void);
void vTaskADC    (void *pvParameters);
void vTaskControl(void *pvParameters);
void vTaskUART   (void *pvParameters);
void vTaskAlarm  (void *pvParameters);
void vTaskAtuador  (void *pvParameters);

#endif