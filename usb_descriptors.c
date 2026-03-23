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

uint8_t const desc_hid_report[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)
};

tusb_desc_device_t const* tud_descriptor_device_cb(void) {
    return &desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    return NULL;
}

uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)itf;(void)report_id;(void)report_type;(void)buffer;(void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void)itf;(void)report_id;(void)report_type;(void)buffer;(void)bufsize;
}
