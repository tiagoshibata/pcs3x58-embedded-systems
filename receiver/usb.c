#include "usb.h"

#include <inc/hw_memmap.h>
#include <driverlib/gpio.h>
#include <driverlib/rom.h>
#include <driverlib/sysctl.h>
#include <usblib/usblib.h>
#include <usblib/usbhid.h>
#include <usblib/usb-ids.h>
#include <usblib/device/usbdevice.h>
#include <usblib/device/usbdhid.h>
#include <usblib/device/usbdhidgamepad.h>
#include <stdlib.h>

#include "leds.h"

gamepad_report_t usb_report;

static const uint8_t usb_language_descriptor[] = {
    4,
    USB_DTYPE_STRING,
    USBShort(USB_LANG_EN_US)
};

static const uint8_t usb_manufacturer_string[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    'H', 0, 'o', 0, 'm', 0, 'e', 0, 'm', 0, 'a', 0, 'd', 0, 'e', 0
};

static const uint8_t usb_product_string[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    'P', 0, 'i', 0, 'm', 0, 'p', 0, 'o', 0, 'l', 0, 'h', 0, 'o', 0
};

static const uint8_t usb_serial_number[] = {
    (8 + 1) * 2,
    USB_DTYPE_STRING,
    '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '1', 0
};

static const uint8_t usb_hid_interface_string[] = {
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'P', 0, 'i', 0, 'm', 0, 'p', 0, 'o', 0, 'l', 0, 'h', 0, 'o', 0, ' ', 0,
    'r', 0, 'e', 0, 'c', 0, 'e', 0, 'i', 0, 'v', 0, 'e', 0, 'r', 0
};

static const uint8_t usb_configuration_string[] =
{
    (17 + 1) * 2,
    USB_DTYPE_STRING,
    'P', 0, 'i', 0, 'm', 0, 'p', 0, 'o', 0, 'l', 0, 'h', 0, 'o', 0, ' ', 0,
    'r', 0, 'e', 0, 'c', 0, 'e', 0, 'i', 0, 'v', 0, 'e', 0, 'r', 0
};

static const uint8_t * const usb_string_descriptors[] = {
    usb_language_descriptor,
    usb_manufacturer_string,
    usb_product_string,
    usb_serial_number,
    usb_hid_interface_string,
    usb_configuration_string
};

static uint32_t usb_callback(void __attribute__((unused)) *private_data,
    uint32_t event, uint32_t parameter, void *data);

tUSBDHIDGamepadDevice gamepad_device = {
    .ui16VID = USB_VID_TI_1CBE,    // vendor ID
    .ui16PID = USB_PID_GAMEPAD,    // product ID
    .ui16MaxPowermA = 50,        // max. current (mA)
    .ui8PwrAttributes = USB_CONF_ATTR_BUS_PWR,
    .pfnCallback = usb_callback,
    .pvCBData = NULL,
    .ppui8StringDescriptors = usb_string_descriptors,
    .ui32NumStringDescriptors = sizeof(usb_string_descriptors) / sizeof(uint8_t *),
    .pui8ReportDescriptor = NULL,
    .ui32ReportSize = 0
};

enum {
    REPORT_BUSY,        // USB busy, no new scheduled reports
    REPORT_EMPTY,       // USB free, no new scheduled reports
    REPORT_SCHEDULED,   // USB busy w/ new shceduled reports
};

static int report_status = REPORT_BUSY;

static uint32_t usb_callback(void __attribute__((unused)) *private_data,
    uint32_t event, uint32_t __attribute__((unused)) parameter, void *data) {
    switch (event) {
    case USB_EVENT_TX_COMPLETE:
    case USB_EVENT_CONNECTED:
    case USB_EVENT_RESUME:
        if (report_status == REPORT_SCHEDULED) {
            report_status = REPORT_BUSY;
            USBDHIDGamepadSendReport(&gamepad_device, &usb_report, sizeof(usb_report));
        }
        else
            report_status = REPORT_EMPTY;
        break;
    case USBD_HID_EVENT_GET_REPORT:
        *(gamepad_report_t **)data = &usb_report;
        break;
    case USB_EVENT_DISCONNECTED:
    case USB_EVENT_ERROR:
    case USB_EVENT_SUSPEND:
        report_status = REPORT_BUSY;
        break;
    case USBD_HID_EVENT_GET_REPORT_BUFFER:
        break;
    }
    return 0;
}

void usb_init() {
    // USB pins
    SysCtlGPIOAHBEnable(SYSCTL_PERIPH_GPIOD);
    ROM_GPIOPinTypeUSBAnalog(GPIO_PORTD_AHB_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    // Use device mode
    USBStackModeSet(0, eUSBModeForceDevice, 0);

    if (USBDHIDGamepadInit(0, &gamepad_device) != &gamepad_device) {
        leds_b(4000);
        for (;;) ;
    }
}

void usb_schedule_report() {
    switch (report_status) {
    case REPORT_EMPTY:
        // Can report now
        USBDHIDGamepadSendReport(&gamepad_device, &usb_report, sizeof(usb_report));
        report_status = REPORT_BUSY;
        break;

    case REPORT_BUSY:
        report_status = REPORT_SCHEDULED;
        break;

    case REPORT_SCHEDULED:
        break;
    }
}
