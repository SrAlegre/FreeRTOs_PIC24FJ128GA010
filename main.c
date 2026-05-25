// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT Oscillator mode selected)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = FRCDIV           // Oscillator Select (Fast RC Oscillator with Postscaler (FRCDIV))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Watchdog Timer Window (Windowed Watchdog Timer enabled; FWDTEN must be 1)
#pragma config FWDTEN = OFF              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)
#include <xc.h>
#include "FreeRTOS.h"
#include "task.h"
#include "io.h"
#include "user_app.h"
#define _XTAL_FREQ_ 25000000UL

int main(void) {

    init_task();
    xTaskCreate(vTaskADC, "ADC", 128, NULL, 2, NULL);
    xTaskCreate(vTaskControl, "CTRL", 256, NULL, 3, NULL); //Usa snprintf (pesado)
    xTaskCreate(vTaskUART, "UART", 256, NULL, 1, NULL); // Array local de 32 bytes + funções de string
    xTaskCreate(vTaskAlarm, "ALARM", 128, NULL, 4, NULL);
    xTaskCreate(vTaskAtuador, "ATUADOR", 128, NULL, 4, NULL);

    vTaskStartScheduler();
    while (1);


    return 0;
}