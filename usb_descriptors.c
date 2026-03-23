#include "usb_descriptors.h"

tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = 0xEF,
    .bDeviceSubClass = 0x02,
    .bDeviceProtocol = 0x01,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = GAMEPAD_VID,
    .idProduct = GAMEPAD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

#define CONFIG_TOTAL_LEN  (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN)

uint8_t const desc_configuration[] = {
    9, TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),
    0x02,
    0x01,
    0x00,
    0x80,
    0xFA,

    9, TUSB_DESC_INTERFACE,
    0x00,
    0x00,
    0x02,
    0x02,
    0x02,
    0x01,
    0x00,

    7, TUSB_DESC_ENDPOINT,
    0x81,
    0x03,
    U16_TO_U8S_LE(64),
    0x10,

    7, TUSB_DESC_ENDPOINT,
    0x01,
    0x02,
    U16_TO_U8S_LE(64),
    0x10,
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)index;(void)langid;
    return NULL;
}
