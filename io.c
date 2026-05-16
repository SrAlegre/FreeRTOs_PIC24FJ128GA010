#include "io.h"
#include <xc.h>

void ADC_Init(void) {
    AD1PCFGbits.PCFG0 = 0; // AN0 analógico

    AD1CON1bits.ADON = 0; // desliga antes de configurar
    AD1CON1bits.SSRC = 0; // conversão manual
    AD1CON1bits.ASAM = 0; // amostragem manual

    AD1CON2bits.VCFG = 0; // Vref = AVdd/AVss
    AD1CON2bits.CSCNA = 0; // canal fixo

    AD1CON3bits.ADCS = 1; // Tad = 2*Tcy
    AD1CON3bits.SAMC = 31; // sample time = 31 Tad

    AD1CHSbits.CH0SA = 0; // AN0

    AD1CON1bits.ADON = 1; // liga ADC
}

float ADC_ReadTemp(void) {
    AD1CON1bits.SAMP = 1;
    vTaskDelay(pdMS_TO_TICKS(10));
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);

    unsigned int raw = ADC1BUF0;
    float tensao = raw * (3.3f / 1023.0f);
    float temp = tensao * 100.0f;
    return temp;
}

void UART_Init(void) {
    U1MODEbits.STSEL = 0;   // 1 stop bit
    U1MODEbits.PDSEL = 0;   // 8 bits, sem paridade
    U1MODEbits.BRGH  = 0;   // standard speed

    U1BRG = 25;             // 9600 baud @ Fcy=4MHz

    U1MODEbits.UARTEN = 1;  // 1º habilita UART
    U1STAbits.UTXEN   = 1;  // 2º habilita TX
}

void UART_WriteChar(char c) {
    while (U1STAbits.UTXBF);
    U1TXREG = c;
}

void UART_WriteString(const char *str) {
    while (*str)
        UART_WriteChar(*str++);
}