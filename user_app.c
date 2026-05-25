#include "user_app.h"
#include "io.h"
#include <stdio.h>   
#include <string.h>  
#include "timers.h"

#define LIMIAR_TEMP  50.0 
#define HIST_VAL  3.0f  // Margem de histerese (desliga em LIMIAR_TEMP - HIST_VAL)

void floatToString(float valor, int casas_decimais, char *buffer, int tamanho);

QueueHandle_t xQueueAtuador; // Nova fila para controlar o cooler
QueueHandle_t xQueueTemp;

SemaphoreHandle_t xMutexBuf;
SemaphoreHandle_t xMutexUART;
SemaphoreHandle_t xSemAlarm;

TimerHandle_t xTimerADC;

char buffer[32];

// Declaração da função de callback
void vTimerADC_Callback(TimerHandle_t xTimer);

void init_task(void) {
    ADC_Init();
    UART_Init();
    Atuador_Init();

    xQueueTemp = xQueueCreate(4, sizeof (float));
    xQueueAtuador = xQueueCreate(2, sizeof(atuador_cmd_t));
    
    xMutexBuf = xSemaphoreCreateMutex();
    xMutexUART = xSemaphoreCreateMutex();
    xSemAlarm = xSemaphoreCreateBinary();
    
    // --- CRIAÇÃO E INICIALIZAÇÃO DO TIMER (100ms periódico) ---
    xTimerADC = xTimerCreate(
        "Timer_ADC",               // Nome apenas para debug
        pdMS_TO_TICKS(100),        // Período de 100ms (igual à antiga task)
        pdTRUE,                    // pdTRUE indica que o timer é auto-reload (periódico)
        (void *) 0,                // ID do timer (não precisamos usar)
        vTimerADC_Callback         // A função que será chamada a cada 100ms
    );

    if (xTimerADC != NULL) {
        xTimerStart(xTimerADC, 0); // Inicializa o timer imediatamente
    }
}

/*
void vTaskADC(void *pvParameters) {
    (void) pvParameters;
    float temp;
    while (1) {
        temp = ADC_ReadTemp();

        xQueueSend(xQueueTemp, &temp, portMAX_DELAY); //Coloca o valor no pipe
        vTaskDelay(pdMS_TO_TICKS(100)); //aguarda para proxima leitura
    }
} 
 */

void vTimerADC_Callback(TimerHandle_t xTimer) {
    (void) xTimer;
    float temp;

    temp = ADC_ReadTemp();

    // IMPORTANTE: O terceiro parâmetro DEVE SER 0. Não podemos bloquear aqui!
    xQueueSend(xQueueTemp, &temp, 0); 
}

void vTaskControl(void *pvParameters) {
    (void) pvParameters;
    float temp;
    
    static atuador_cmd_t estado_atual = ATUADOR_OFF; // Mantém o estado da última execução

    while (1) {
        xQueueReceive(xQueueTemp, &temp, portMAX_DELAY); // recebe o valor adc e coloca no temp

        xSemaphoreTake(xMutexBuf, portMAX_DELAY); // segura o buffer para não escrever em cima

        floatToString(temp, 3, buffer, sizeof (buffer));

        xSemaphoreGive(xMutexBuf); // libera o buffer

        // Lógica de Controle com Histerese
        if (temp >= LIMIAR_TEMP && estado_atual == ATUADOR_OFF) {
            estado_atual = ATUADOR_ON;
            xQueueSend(xQueueAtuador, &estado_atual, 0); // Envia comando para LIGAR
            xSemaphoreGive(xSemAlarm);                    // Dispara alarme na UART
        } 
        else if (temp < (LIMIAR_TEMP - HIST_VAL) && estado_atual == ATUADOR_ON) {
            estado_atual = ATUADOR_OFF;
            xQueueSend(xQueueAtuador, &estado_atual, 0); // Envia comando para DESLIGAR
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // aguarda novamente
    }
}

void vTaskUART(void *pvParameters) {
    (void) pvParameters;
    char local[32];

    while (1) {
        xSemaphoreTake(xMutexBuf, portMAX_DELAY); // segura o buffer para poder escrever na uart
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

void vTaskAtuador(void *pvParameters) {
    (void) pvParameters;
    atuador_cmd_t comando;

    while (1) {
        // Fica bloqueada aguardando um comando de LIGAR ou DESLIGAR
        if (xQueueReceive(xQueueAtuador, &comando, portMAX_DELAY) == pdTRUE) {
            if (comando == ATUADOR_ON) {
                GPIO_WriteAtuador(1); // Liga a fan
            } else {
                GPIO_WriteAtuador(0); // Desliga a fan
            }
        }
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