#include "leds.h"

#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>
#include <driverlib/rom.h>

///@defgroup leds LEDs
///@{

/**
 * Initialize LEDs.
 * Initialize M1PWM5, M1PWM6, M1PWM7 (Tiva RGB LED). Period is set to 4096.
 */
void leds_init() {
    // Configure PF1, PF2, PF3 (RGB LED)
    ROM_GPIOPinConfigure(GPIO_PF1_M1PWM5);
    ROM_GPIOPinConfigure(GPIO_PF2_M1PWM6);
    ROM_GPIOPinConfigure(GPIO_PF3_M1PWM7);
    ROM_GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

    // PWM_GEN_2 covers M1PWM4 and M1PWM5
    // PWM_GEN_3 covers M1PWM6 and M1PWM7
    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC |
        PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_DB_NO_SYNC| PWM_GEN_MODE_FAULT_UNLATCHED |
        PWM_GEN_MODE_FAULT_NO_MINPER);
    ROM_PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC |
        PWM_GEN_MODE_DBG_RUN | PWM_GEN_MODE_DB_NO_SYNC | PWM_GEN_MODE_FAULT_UNLATCHED |
        PWM_GEN_MODE_FAULT_NO_MINPER);

    // Set the Period (in clock ticks)
    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 4096);
    ROM_PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 4096);

    // Enable the PWM generator
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    ROM_PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    // Enable PWM
    ROM_PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);
}

///@}
