#include "usb_descriptors.h"

tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = GAMEPAD_VID,
    .idProduct          = GAMEPAD_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

uint8_t const hid_report_descriptor[] = {
    0x05, 0x01,
    0x09, 0x05,
    0xA1, 0x01,
    0x85, 0x01,

    0x05, 0x09,
    0x19, 0x01,
    0x29, 0x10,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x10,
    0x81, 0x02,

    0x05, 0x01,
    0x09, 0x39,
    0x15, 0x00,
    0x25, 0x07,
    0x35, 0x00,
    0x46, 0x3B, 0x01,
    0x65, 0x14,
    0x75, 0x04,
    0x95, 0x01,
    0x81, 0x42,

    0x75, 0x04,
    0x95, 0x01,
    0x81, 0x03,

    0x05, 0x01,
    0x09, 0x30,
    0x09, 0x31,
    0x09, 0x32,
    0x09, 0x33,
    0x09, 0x34,
    0x09, 0x35,
    0x15, 0x81,
    0x25, 0x7F,
    0x75, 0x08,
    0x95, 0x06,
    0x81, 0x02,

    0xC0
};

#define HID_REPORT_DESCRIPTOR_LEN sizeof(hid_report_descriptor)

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + 7)

uint8_t const desc_configuration[] = {
    9, TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),
    0x01,
    0x01,
    0x00,
    0x80,
    0xFA,

    9, TUSB_DESC_INTERFACE,
    0x00,
    0x00,
    0x01,
    0x03,
    0x00,
    0x00,
    0x00,

    9, 0x21,
    0x11, 0x01,
    0x00,
    0x01,
    0x22,
    U16_TO_U8S_LE(HID_REPORT_DESCRIPTOR_LEN),

    7, TUSB_DESC_ENDPOINT,
    0x81,
    0x03,
    U16_TO_U8S_LE(32),
    0x01,
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index;
    return desc_configuration;
}

uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf;
    return hid_report_descriptor;
}

char const* string_desc_arr[] = {
    (char const[]) { 0x04, 0x03, 0x09, 0x04 },
    "Waveshare",
    "Xbox 360 Controller Sync",
    "1234567890",
};

static uint16_t _desc_str[32];

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void) langid;
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&_desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) return NULL;
        const char* str = string_desc_arr[index];
        chr_count = strlen(str);
        if (chr_count > 31) chr_count = 31;
        for (uint8_t i = 0; i < chr_count; i++) {
            _desc_str[1 + i] = str[i];
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}
