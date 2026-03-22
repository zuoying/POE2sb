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
    .bcdDevice          = 0x0114,  // Xbox 360控制器标准版本号
    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,
    .bNumConfigurations = 0x01
};

uint8_t const * tud_descriptor_device_cb(void) {
    return (uint8_t const *) &desc_device;
}

// ------------------------------------------------------------------
// HID 报告描述符 (Xbox 360 控制器)
// ------------------------------------------------------------------
// 注意：Xbox 360控制器使用的是特殊的HID报告格式
uint8_t const hid_report_descriptor[] = {
    0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
    0x09, 0x05,        // USAGE (Game Pad)
    0xA1, 0x01,        // COLLECTION (Application)
    0x09, 0x01,        //   USAGE (Pointer)
    0xA1, 0x00,        //   COLLECTION (Physical)
    0x05, 0x01,        //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,        //     USAGE (X)
    0x09, 0x31,        //     USAGE (Y)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x02,        //     REPORT_COUNT (2)
    0x81, 0x02,        //     INPUT (Data,Var,Abs)
    0xC0,              //   END_COLLECTION
    0x09, 0x01,        //   USAGE (Pointer)
    0xA1, 0x00,        //   COLLECTION (Physical)
    0x05, 0x01,        //     USAGE_PAGE (Generic Desktop)
    0x09, 0x32,        //     USAGE (Z)
    0x09, 0x35,        //     USAGE (Rz)
    0x15, 0x00,        //     LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //     REPORT_SIZE (8)
    0x95, 0x02,        //     REPORT_COUNT (2)
    0x81, 0x02,        //     INPUT (Data,Var,Abs)
    0xC0,              //   END_COLLECTION
    0x05, 0x09,        //   USAGE_PAGE (Button)
    0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
    0x29, 0x10,        //   USAGE_MAXIMUM (Button 16)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,        //   REPORT_SIZE (1)
    0x95, 0x10,        //   REPORT_COUNT (16)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
    0x09, 0x39,        //   USAGE (Hat switch)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
    0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
    0x46, 0x3B, 0x01,  //   PHYSICAL_MAXIMUM (315)
    0x65, 0x14,        //   UNIT (Eng Rot:Angular Pos)
    0x75, 0x04,        //   REPORT_SIZE (4)
    0x95, 0x01,        //   REPORT_COUNT (1)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
    0x09, 0x33,        //   USAGE (Rx)
    0x09, 0x34,        //   USAGE (Ry)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x02,        //   REPORT_COUNT (2)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x06, 0x00, 0xFF,  //   USAGE_PAGE (Vendor Defined 0xFF00)
    0x09, 0x03,        //   USAGE (Vendor Usage 3)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x06,        //   REPORT_COUNT (6)
    0x81, 0x02,        //   INPUT (Data,Var,Abs)
    0x06, 0x00, 0xFF,  //   USAGE_PAGE (Vendor Defined 0xFF00)
    0x09, 0x04,        //   USAGE (Vendor Usage 4)
    0x15, 0x00,        //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,        //   REPORT_SIZE (8)
    0x95, 0x06,        //   REPORT_COUNT (6)
    0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
    0xC0               // END_COLLECTION
};

// 计算HID报告描述符长度
#define HID_REPORT_DESCRIPTOR_LEN sizeof(hid_report_descriptor)

// ------------------------------------------------------------------
// 配置描述符 (双 Xbox 360 控制器 - 使用HID类)
// ------------------------------------------------------------------
// 计算配置描述符总长度
// 配置描述符: 9字节
// 每个HID接口: 接口(9) + HID(9) + IN端点(7) + OUT端点(7) = 32字节
// 两个接口: 2 * 32 = 64字节
// 总长度: 9 + 64 = 73字节
#define CONFIG_TOTAL_LEN (9 + 2 * (9 + 9 + 7 + 7)) // 1 config + 2 HID interfaces + 4 endpoints

uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    9, TUSB_DESC_CONFIGURATION,
    U16_TO_U8S_LE(CONFIG_TOTAL_LEN),
    0x02, // 2 Interfaces
    0x01, // Configuration Value
    0x00, // Index of string descriptor
    0x80, // Attributes: Bus Powered only
    0xFA, // MaxPower 500mA

    // Interface 0: Xbox 360 Controller 1 (HID类)
    9, TUSB_DESC_INTERFACE,
    0x00,        // bInterfaceNumber
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints (IN + OUT)
    0x03,        // bInterfaceClass (HID)
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface

    // HID Descriptor
    9, 0x21,     // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID (v1.11)
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType (Report)
    U16_TO_U8S_LE(HID_REPORT_DESCRIPTOR_LEN), // wDescriptorLength

    // Endpoint IN: Controller 1 Input
    7, TUSB_DESC_ENDPOINT,
    0x81,        // bEndpointAddress (IN 1)
    0x03,        // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32), // wMaxPacketSize
    0x01,        // bInterval

    // Endpoint OUT: Controller 1 Output (Rumble/LED)
    7, TUSB_DESC_ENDPOINT,
    0x01,        // bEndpointAddress (OUT 1)
    0x03,        // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32), // wMaxPacketSize
    0x08,        // bInterval

    // Interface 1: Xbox 360 Controller 2 (HID类)
    9, TUSB_DESC_INTERFACE,
    0x01,        // bInterfaceNumber
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints (IN + OUT)
    0x03,        // bInterfaceClass (HID)
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface

    // HID Descriptor
    9, 0x21,     // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID (v1.11)
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType (Report)
    U16_TO_U8S_LE(HID_REPORT_DESCRIPTOR_LEN), // wDescriptorLength

    // Endpoint IN: Controller 2 Input
    7, TUSB_DESC_ENDPOINT,
    0x82,        // bEndpointAddress (IN 2)
    0x03,        // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32), // wMaxPacketSize
    0x01,        // bInterval

    // Endpoint OUT: Controller 2 Output (Rumble/LED)
    7, TUSB_DESC_ENDPOINT,
    0x02,        // bEndpointAddress (OUT 2)
    0x03,        // bmAttributes (Interrupt)
    U16_TO_U8S_LE(32), // wMaxPacketSize
    0x08,        // bInterval
};

uint8_t const * tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations
    return desc_configuration;
}

// HID 报告描述符回调函数
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf) {
    (void) itf; // 两个接口使用相同的报告描述符
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
