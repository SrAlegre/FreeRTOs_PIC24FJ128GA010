#include "user_app.h"
#include "io.h"
#include <stdio.h>   
#include <string.h>  

QueueHandle_t xQueueTemp;
SemaphoreHandle_t xMutexBuf;
SemaphoreHandle_t xMutexUART;
SemaphoreHandle_t xSemAlarm;
char buffer[32];


void init_task(void) {
    ADC_Init();
    UART_Init();

    xQueueTemp = xQueueCreate(4, sizeof (float));
    xMutexBuf = xSemaphoreCreateMutex();
    xMutexUART = xSemaphoreCreateMutex();
    xSemAlarm = xSemaphoreCreateBinary();
}


void vTaskADC(void *pvParameters) {
    (void) pvParameters;
    float temp;

    while (1) {
        temp = ADC_ReadTemp();
        xQueueSend(xQueueTemp, &temp, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}


void vTaskControl(void *pvParameters) {
    (void) pvParameters;
    float temp;

    while (1) {
        xQueueReceive(xQueueTemp, &temp, portMAX_DELAY);

        xSemaphoreTake(xMutexBuf, portMAX_DELAY);
        sprintf(buffer, "TEMP: %.1f C\r\n", temp);
        xSemaphoreGive(xMutexBuf);

        if (temp >= LIMIAR_TEMP) {
            xSemaphoreGive(xSemAlarm);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


void vTaskUART(void *pvParameters) {
    (void) pvParameters;
    char local[32];

    while(1) {
        xSemaphoreTake(xMutexBuf, portMAX_DELAY);
        strcpy(local, buffer);
        xSemaphoreGive(xMutexBuf);

        xSemaphoreTake(xMutexUART, portMAX_DELAY);
        UART_WriteString(local);
        xSemaphoreGive(xMutexUART);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}


void vTaskAlarm(void *pvParameters) {
    (void) pvParameters;

    while(1) {
        xSemaphoreTake(xSemAlarm, portMAX_DELAY);

        xSemaphoreTake(xMutexUART, portMAX_DELAY);
        UART_WriteString("!! ALARME: temperatura critica !!\r\n");
        xSemaphoreGive(xMutexUART);
    }
}