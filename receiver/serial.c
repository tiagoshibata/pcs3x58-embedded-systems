#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <driverlib/uart.h>
#include <inc/hw_memmap.h>
#include <inc/hw_ints.h>

#include "usb.h"
#include "leds.h"  // FIXME

void serial_init() {
    ROM_GPIOPinConfigure(GPIO_PB0_U1RX);
    ROM_GPIOPinConfigure(GPIO_PB1_U1TX);

    ROM_GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    ROM_UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 115200,
        UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_ODD);

#ifdef UART_LOOPBACK  // useful for testing e.g. interrupts
    UARTLoopbackEnable(UART1_BASE);
#endif
    // ROM_IntEnable(INT_UART1);
    // ROM_UARTIntEnable(UART1_BASE, UART_INT_RX | UART_INT_RT);
}

// If using UART interrupts:
// void UART1IntHandler() {
//     uint32_t ui32Status = ROM_UARTIntStatus(UART1_BASE, true);
//     ROM_UARTIntClear(UART1_BASE, ui32Status);
//     while (ROM_UARTCharsAvail(UART1_BASE)) {
//         uint32_t buttons = ROM_UARTCharGetNonBlocking(UART1_BASE);
//         if (buttons != usb_report.buttons) {
//             usb_report.buttons = buttons;
//             usb_schedule_report();
//         }
//     }
// }
