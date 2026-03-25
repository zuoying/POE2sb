#include "usb_descriptors.h"

// XInput游戏手柄设备描述符
tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = 0x01,           // 设备描述符类型 (0x01)
    .bcdUSB = 0x0200,
    .bDeviceClass = 0x00,        // 每个接口指定类
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = GAMEPAD_VID,
    .idProduct = GAMEPAD_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01
};

// 通用游戏手柄HID报告描述符
// 20字节报告，匹配xinput_report_t结构
uint8_t const desc_hid_report[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    
    // 报告ID (1字节)
    0x85, 0x01,        //   Report ID (1)
    
    // 按钮 (16位 = 2字节)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (Button 1)
    0x29, 0x10,        //   Usage Maximum (Button 16)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // 左右扳机 (2字节)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x33,        //   Usage (Rx) - 左扳机
    0x09, 0x34,        //   Usage (Ry) - 右扳机
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0xFF,        //   Logical Maximum (255)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0xFF,        //   Physical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // 左摇杆 X/Y (4字节 = 2个16位值)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x16, 0x00, 0x80,  //   Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // 右摇杆 X/Y (4字节 = 2个16位值)
    0x05, 0x01,        //   Usage Page (Generic Desktop)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x16, 0x00, 0x80,  //   Logical Minimum (-32768)
    0x26, 0xFF, 0x7F,  //   Logical Maximum (32767)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    
    // 保留字段 (6字节)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x03,        //   Input (Constant,Var,Abs)
    
    0xC0               // End Collection
};

// HID配置描述符
uint8_t const desc_configuration[] = {
    // 配置描述符 (9 bytes)
    9, 0x02,                      // 配置描述符类型 (0x02)
    34, 0,                        // 配置总长度 (little-endian: 34 = 0x22)
    0x01,                         // 接口数量
    0x01,                         // 配置值
    0x00,                         // 字符串索引
    0x80,                         // 供电模式 (总线供电)
    0x32,                         // 最大电流 (100mA)
    
    // 接口描述符 (9 bytes)
    9, 0x04,                      // 接口描述符类型 (0x04)
    0x00,                         // 接口号
    0x00,                         // 备用设置
    0x01,                         // 端点数量
    0x03,                         // 接口类 (HID)
    0x00,                         // 接口子类
    0x00,                         // 接口协议
    0x00,                         // 字符串索引
    
    // HID描述符 (9 bytes)
    9, 0x21,                      // HID描述符类型 (0x21)
    0x11, 0x01,                   // HID版本 (1.11)
    0x00,                         // 国家代码
    0x01,                         // 描述符数量
    0x22,                         // 报告描述符类型 (0x22)
    (uint8_t)(sizeof(desc_hid_report) & 0xFF),        // 描述符长度 (low byte)
    (uint8_t)((sizeof(desc_hid_report) >> 8) & 0xFF), // 描述符长度 (high byte)
    
    // 端点描述符 (7 bytes)
    7, 0x05,                      // 端点描述符类型 (0x05)
    0x81,                         // 端点地址 (IN 1)
    0x03,                         // 属性 (中断)
    64, 0,                        // 最大包大小 (little-endian: 64 = 0x40)
    0x01                          // 轮询间隔 (1ms)
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*) &desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return desc_configuration;
}

// HID报告描述符长度
uint16_t const desc_hid_report_len = sizeof(desc_hid_report);

// HID报告描述符回调
uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    return desc_hid_report;
}

// 字符串描述符
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint16_t desc_str[32];
    
    if (index == 0) {
        // 语言ID (英语)
        desc_str[1] = 0x0409;
        desc_str[0] = (0x03 << 8) | 4;  // 字符串描述符类型 (0x03)
        return desc_str;
    }
    
    if (langid != 0x0409) {
        // 只支持英语
        return NULL;
    }
    
    const char* str;
    uint8_t len;
    
    switch (index) {
        case 1:  // 制造商
            str = "POE2sb";
            break;
        case 2:  // 产品名称
            str = "Xbox 360 Controller";
            break;
        case 3:  // 序列号
            str = "123456";
            break;
        default:
            return NULL;
    }
    
    len = strlen(str);
    desc_str[0] = (0x03 << 8) | (2 * len + 2);  // 字符串描述符类型 (0x03)
    for (uint8_t i = 0; i < len; i++) {
        desc_str[i + 1] = str[i];
    }
    
    return desc_str;
}
