#pragma once

typedef struct {
    int8_t x1;
    int8_t y1;
    int8_t x2;
    uint8_t buttons;
} __attribute__((packed)) gamepad_report_t;

extern gamepad_report_t usb_report;

void usb_init();
void usb_schedule_report();
