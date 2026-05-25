#include "user_app.h"
#include "io.h"
#include <stdio.h>   
#include <string.h>  
#define LIMIAR_TEMP  50.0 

void floatToString(float valor, int casas_decimais, char *buffer, int tamanho);

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

        xQueueSend(xQueueTemp, &temp, portMAX_DELAY); //Coloca o valor no pipe
        vTaskDelay(pdMS_TO_TICKS(100)); //aguarda para proxima leitura
    }
}

void vTaskControl(void *pvParameters) {
    (void) pvParameters;
    float temp;

    while (1) {
        xQueueReceive(xQueueTemp, &temp, portMAX_DELAY); // recebe o valor adc e coloca no temp

        xSemaphoreTake(xMutexBuf, portMAX_DELAY); // segura o buffer para não escreve em cima

        floatToString(temp, 3, buffer, sizeof (buffer));

        xSemaphoreGive(xMutexBuf); // libera o buffer

        //confere
        if (temp >= LIMIAR_TEMP) {
            xSemaphoreGive(xSemAlarm); // libera o semphoro do alarm
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // aguarda novamente
    }
}

void vTaskUART(void *pvParameters) {
    (void) pvParameters;
    char local[32];

    while (1) {
        xSemaphoreTake(xMutexBuf, portMAX_DELAY); // segura o buffe para poder escrever na uart
        UART_WriteString("\r\n");
        strcpy(local, buffer);
        xSemaphoreGive(xMutexBuf); // libera

        xSemaphoreTake(xMutexUART, portMAX_DELAY); // segura a aguarde para n ser subescrito
        UART_WriteString(local);
        xSemaphoreGive(xMutexUART); //libera

        vTaskDelay(pdMS_TO_TICKS(200)); // tempo para o proxima vez que for exibir algo na tela
    }
}

void vTaskAlarm(void *pvParameters) {
    (void) pvParameters;

    while (1) {
        xSemaphoreTake(xSemAlarm, portMAX_DELAY); // bloqueia a tarefa esperando o sinal de alarme
        xSemaphoreTake(xMutexUART, portMAX_DELAY);
        UART_WriteString("\r\n!! ALARME: temperatura critica !!\r\n"); // escreve
        xSemaphoreGive(xMutexUART); // libera uart
    }
}

void floatToString(float valor, int casas_decimais, char *buffer, int tamanho) {
    // Trata valor negativo
    int negativo = 0;
    if (valor < 0) {
        negativo = 1;
        valor = -valor;
    }

    int inteiro = (int) valor;

    // Calcula multiplicador para as casas decimais (10, 100, 1000...)
    int multiplicador = 1;
    int i;
    for (i = 0; i < casas_decimais; i++) {
        multiplicador *= 10;
    }

    int decimal = (int) ((valor - inteiro) * multiplicador + 0.5f); // +0.5 para arredondar

    // Caso o arredondamento estoure (ex: 0.999 com 2 casas -> decimal = 100)
    if (decimal >= multiplicador) {
        inteiro++;
        decimal = 0;
    }

    if (negativo) {
        snprintf(buffer, tamanho, "-%d.%0*d", inteiro, casas_decimais, decimal);
    } else {
        snprintf(buffer, tamanho, "%d.%0*d", inteiro, casas_decimais, decimal);
    }
}