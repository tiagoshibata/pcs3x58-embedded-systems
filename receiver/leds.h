#pragma once

#include <driverlib/pwm.h>
#include <driverlib/rom.h>
#include <inc/hw_memmap.h>

void leds_init();

///@addtogroup leds
///@{

/**
 * Set red LED.
 * @param[in] brightness red value.
 */
static inline void leds_r(uint32_t brightness) {
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, brightness);
}

/**
 * Set blue LED.
 * @param[in] brightness blue value.
 */
static inline void leds_b(uint32_t brightness) {
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, brightness);
}

/**
 * Set green LED.
 * @param[in] brightness green value.
 */
static inline void leds_g(uint32_t brightness) {
    ROM_PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, brightness);
}

///@}
