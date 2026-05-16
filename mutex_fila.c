#include "mutex_fila.h"
#include <xc.h>


SemaphoreHandle_t mutex;
QueueHandle_t fila;

void config_mutex_fila_exemplo()
{
    TRISGbits.TRISG0 = 0;
    TRISGbits.TRISG1 = 0;
    TRISFbits.TRISF0 = 0;   
    
    mutex = xSemaphoreCreateMutex();
    fila = xQueueCreate(5, sizeof(int));
}

void task_1()
{
    int i, acao[10] = {1, 1, 0, 0, 0, 0, 0, 1, 1, 0};
    
    i = 0;
    
    while (1) {
        //xSemaphoreTake(mutex, portMAX_DELAY);
        PORTGbits.RG1 = ~PORTGbits.RG1;
        xQueueSend(fila, &acao[i], portMAX_DELAY);
        i = (i+1) % 10;
        //xSemaphoreGive(mutex);
    }
}

void task_2()
{
    int cmd_recebido;
    
    while (1) {        
        PORTGbits.RG0 = ~PORTGbits.RG0;
        
        xQueueReceive(fila, &cmd_recebido, portMAX_DELAY);
        
        if (cmd_recebido == 0) {
            PORTFbits.RF0 = 0;
        }
        else if (cmd_recebido == 1) {
            PORTFbits.RF0 = 1;
        }
        
        //vTaskDelay(5);        
    }
}

