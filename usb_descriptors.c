#include "usb_descriptors.h"

// ------------------------------------------------------------------
// 设备描述符
// ------------------------------------------------------------------
tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,
    .bDeviceClass       = 0x00,  // 设备类由接口指定
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = XBOX_VID,
    .idProduct          = XBOX_PID,
    .bcdDevice          = XINPUT_VERSION,  // 设备版本号
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// ------------------------------------------------------------------
// HID 报告描述符 (标准游戏手柄)
// ------------------------------------------------------------------
uint8_t const hid_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    
    // Buttons
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x10,        //   Usage Maximum (Button 16)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1 bit)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Hat switch
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315 degrees)
    0x65, 0x14,        //   Unit (Degrees)
    0x75, 0x04,        //   Report Size (4 bits)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    
    // Reserved
    0x75, 0x04,        //   Report Size (4 bits)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x03,        //   Input (Const,Var,Abs)
    
    // Axes
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x09, 0x35,        //   Usage (Rz)
    0x15, 0x81,        //   Logical Minimum (-127)
    0x25, 0x7F,        //   Logical Maximum (127)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // Rumble motors (Output Report)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x02,        //   Report Count (2)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    
    0xC0               // End Collection
};

// 计算HID报告描述符长度
#define HID_REPORT_DESCRIPTOR_LEN sizeof(hid_report_descriptor)

// ------------------------------------------------------------------
// 配置描述符 (标准HID游戏手柄)
// ------------------------------------------------------------------
#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN + 7) // 1 config + 1 HID interface + 1 IN endpoint

uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    9, TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),
    0x01, // 1 Interface
    0x01, // Configuration Value
    0x00, // Index of string descriptor
    0x80, // Attributes: Bus Powered only
    0xFA, // MaxPower 500mA

    // Interface 0: Standard Gamepad (HID类)
    9, TUSB_DESC_INTERFACE,
    0x00, // bInterfaceNumber
    0x00, // bAlternateSetting
    0x01, // bNumEndpoints (IN only)
    0x03, // bInterfaceClass: HID
    0x00, // bInterfaceSubClass
    0x00, // bInterfaceProtocol
    0x00, // iInterface

    // HID Descriptor
    9, 0x21,     // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID (v1.11)
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType (Report)
    U16_TO_U8S_LE(HID_REPORT_DESCRIPTOR_LEN), // wDescriptorLength

    // Endpoint IN: Controller Input
    7, TUSB_DESC_ENDPOINT,
    0x81,        // bEndpointAddress (IN 1)
    0x03,        // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32),
    0x01,        // bInterval (1ms)
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

// HID 报告描述符回调函数
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf; // 只有一个接口，所以忽略参数
    return hid_report_descriptor;
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
