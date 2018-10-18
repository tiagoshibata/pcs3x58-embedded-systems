#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <inc/hw_sysctl.h>

#include "input.h"
#include "leds.h"
#include "serial.h"
#include "usb.h"

static inline void get_wireless_data(int8_t *buffer) {
    while (ROM_UARTCharGet(UART1_BASE) != 0xaa) ;
    for (int i = 0; i < 4; i++)
        buffer[i] = (int8_t)ROM_UARTCharGet(UART1_BASE);
}

static inline void schedule_report_if_changed(gamepad_report_t *report) {
    int8_t has_data = 0;
    int8_t *new_data = (int8_t *)report;
    int8_t *old_data = (int8_t *)&usb_report;
    for (uint8_t i = 0; i < sizeof(usb_report); i++) {
        if (*old_data != *new_data) {
            *old_data = *new_data;
            has_data = 1;
        }
    }
    if (has_data)
        usb_schedule_report();
}

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
        gamepad_report_t data;
        get_wireless_data((int8_t *)&data);
        schedule_report_if_changed(&data);
    }
}
