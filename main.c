#include <xc.h>
#include "FreeRTOS.h"
#include "task.h"
#include "io.h"

void vTaskHello(void *pvParameters) {
    (void)pvParameters;
    for (;;) {
        UART_WriteString("Hello World!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int main(void) {
    UART_Init();

    xTaskCreate(vTaskHello, "HELLO", 128, NULL, 1, NULL);

    vTaskStartScheduler();

    while (1);
    return 0;
}