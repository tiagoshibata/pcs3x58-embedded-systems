#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <inc/hw_sysctl.h>

#include "input.h"
#include "leds.h"
#include "serial.h"
#include "usb.h"

int main() {
    // Use 80MHz
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  // UART1
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);  // USB, analog inputs
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);  // buttons
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // board LEDs and switches
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);  // PWM Module 1

    leds_init();
    usb_init();
    input_init();
    serial_init();

    for (;;) {
        uint32_t buttons = ROM_UARTCharGet(UART1_BASE);
        if (buttons != usb_report.buttons) {
            usb_report.buttons = buttons;
            usb_schedule_report();
        }
    }
}
