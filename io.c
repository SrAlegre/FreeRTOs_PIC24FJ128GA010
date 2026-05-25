#include "io.h"
#include "FreeRTOS.h"
#include "task.h"
#include <xc.h>


void ADC_Init(void) {
    AD1PCFGbits.PCFG0 = 0; // AN0 como analógico
    TRISBbits.TRISB0 = 1; // RB0 como entrada (CORRIGIDO)

    AD1CON1 = 0x0000;
    AD1CHS = 0x0000; // Canal AN0
    AD1CSSL = 0;
    AD1CON2 = 0x0000;
    AD1CON2bits.VCFG = 0b000; // Referência AVDD/AVSS
    AD1CON3 = 0x1F02; // Sample time = 31 TAD

    AD1CON1bits.ADON = 1;
}

float ADC_ReadTemp(void) {
    AD1CON1bits.SAMP = 1;
    vTaskDelay(pdMS_TO_TICKS(5));
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);

    uint16_t raw = (uint16_t)ADC1BUF0;

    float tensao = (raw / 1023.0f) * 3.3f;
    float temp   = tensao * 100.0f;

    return temp;
}
uint16_t ADC_ReadRaw(void) {
    AD1CON1bits.SAMP = 1;         // Inicia amostragem
    vTaskDelay(pdMS_TO_TICKS(5)); // Aguarda estabilizar
    AD1CON1bits.SAMP = 0;         // Para amostragem, inicia conversão
    while (!AD1CON1bits.DONE);    // Aguarda conversão
    return (uint16_t)ADC1BUF0;
}

void UART_Init(void) {


    TRISFbits.TRISF3 = 0;
    U1MODE = 0x0000;
    U1MODEbits.STSEL = 0;
    U1MODEbits.PDSEL = 0;
    U1MODEbits.BRGH = 0;

    U1BRG = 25;
    U1STA = 0x0000;

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN = 1;

}

void UART_WriteChar(char c) {
    while (U1STAbits.UTXBF);
    U1TXREG = c;
}

void UART_WriteString(const char *str) {
    while (*str) {
        UART_WriteChar(*str++);
    }
}

void Atuador_Init(void) {
    
    TRISDbits.TRISD0 = 0; // Configura RD0 como SAÍDA
    LATDbits.LATD0 = 0; // Garante que o cooler inicia DESLIGADO
}

void GPIO_WriteAtuador(int estado) {
    // Escreve no registrador LATCH do PORTD
    if (estado) {
        LATDbits.LATD0 = 1; // Liga o pino
    } else {
        LATDbits.LATD0 = 0; // Desliga o pino
    }
}