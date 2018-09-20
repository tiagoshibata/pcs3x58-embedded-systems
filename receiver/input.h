#pragma once

#define SW1 GPIO_PIN_4
#define SW2 GPIO_PIN_0

void input_init();
uint32_t input_get_switches();
