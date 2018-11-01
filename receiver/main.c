#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <inc/hw_sysctl.h>

#include "input.h"
#include "leds.h"
#include "serial.h"
#include "usb.h"

#define SIGNATURE       0x55

static inline int8_t get_serial() {
    return (int8_t)ROM_UARTCharGet(UART1_BASE);
}

static inline void get_wireless_data(gamepad_report_t *report) {
    for (;;) {
        while (get_serial() != SIGNATURE) ;
        report->x1 = get_serial();
        report->y1 = get_serial();
        report->x2 = get_serial();
        report->buttons = get_serial();
        // Validate checksum
        int16_t checksum = get_serial();
        checksum |= get_serial() << 8;
        if (checksum == ((((uint8_t)report->x1 << 8) | report->y1) ^ (((uint8_t)report->x2 << 8) | report->buttons)))
            return;
    }
}

static inline void schedule_report_if_changed(gamepad_report_t *report) {
    int8_t has_data = 0;
    int8_t *new_data = (int8_t *)report;
    int8_t *old_data = (int8_t *)&usb_report;
    for (uint8_t i = 0; i < sizeof(usb_report); i++) {
        if (old_data[i] != new_data[i]) {
            old_data[i] = new_data[i];
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
        get_wireless_data(&data);
        schedule_report_if_changed(&data);
    }
}
