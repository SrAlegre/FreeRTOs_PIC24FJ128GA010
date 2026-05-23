#ifndef IO_H
#define IO_H

#include "FreeRTOS.h"
#include "task.h"

void ADC_Init(void);
float ADC_ReadTemp(void);
uint16_t ADC_ReadRaw(void);
void UART_Init(void);
void UART_WriteChar(char c);
void UART_WriteString(const char *str);

#endif