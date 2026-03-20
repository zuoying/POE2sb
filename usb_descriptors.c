#include "usb_descriptors.h"

// ------------------------------------------------------------------
// 设备描述符
// ------------------------------------------------------------------
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0xFF,  // Vendor Specific
    .bDeviceSubClass    = 0xFF,
    .bDeviceProtocol    = 0xFF,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = XBOX_VID,
    .idProduct          = XBOX_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// ------------------------------------------------------------------
// 配置描述符 (双 Xbox 360 控制器)
// ------------------------------------------------------------------
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + 2 * (9 + 7 + 7)) // 1 config + 2 interfaces + 4 endpoints

uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    9, TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),
    0x02, // 2 Interfaces
    0x01, // Configuration Value
    0x00, // Index of string descriptor
    0x80, // Attributes: Bus Powered
    0xFA, // MaxPower 500mA

    // Interface 0: Xbox 360 Controller 1
    9, TUSB_DESC_INTERFACE,
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x02, // bNumEndpoints (IN + OUT)
    0xFF, // bInterfaceClass: Vendor Specific
    0x5D, // bInterfaceSubClass: Xbox 360 Controller
    0x01, // bInterfaceProtocol: XInput
    0x00, // iInterface

    // Endpoint IN: Controller 1 Input
    7, TUSB_DESC_ENDPOINT,
    0x81, // bEndpointAddress (IN 1)
    0x03, // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32),
    0x01, // bInterval

    // Endpoint OUT: Controller 1 Output (Rumble/LED)
    7, TUSB_DESC_ENDPOINT,
    0x01, // bEndpointAddress (OUT 1)
    0x03, // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32),
    0x08, // bInterval

    // Interface 1: Xbox 360 Controller 2
    9, TUSB_DESC_INTERFACE,
    0x01, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x02, // bNumEndpoints (IN + OUT)
    0xFF, // bInterfaceClass: Vendor Specific
    0x5D, // bInterfaceSubClass: Xbox 360 Controller
    0x01, // bInterfaceProtocol: XInput
    0x00, // iInterface

    // Endpoint IN: Controller 2 Input
    7, TUSB_DESC_ENDPOINT,
    0x82, // bEndpointAddress (IN 2)
    0x03, // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32),
    0x01, // bInterval

    // Endpoint OUT: Controller 2 Output (Rumble/LED)
    7, TUSB_DESC_ENDPOINT,
    0x02, // bEndpointAddress (OUT 2)
    0x03, // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32),
    0x08, // bInterval
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

// ------------------------------------------------------------------
// 字符串描述符
// ------------------------------------------------------------------
char const* string_desc_arr[] = {
    (char const[]) { 0x04, 0x03, 0x09, 0x04 }, // 0: supported language is English (0x0409)
    "Waveshare",                  // 1: Manufacturer
    "Xbox 360 Controller Sync",   // 2: Product
    "1234567890",                 // 3: Serials
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
