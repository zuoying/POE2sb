#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

// 硬件定义
#define POWER_PIN 18
#define USB_HOST_DP_PIN 12
#define USB_HOST_DM_PIN 13

// 全局状态
static bool gamepad_detected = false;
static uint16_t detected_vid = 0;
static uint16_t detected_pid = 0;

// USB主机回调函数
void tuh_mount_cb(uint8_t dev_addr) {
    printf("\n[USB Device Mounted]\n");
    printf("Device address: %u\n", dev_addr);
    
    // 获取设备描述符
    const tusb_desc_device_t* dev_desc = tuh_device_get_descriptor(dev_addr);
    if (dev_desc) {
        printf("VID: 0x%04X\n", dev_desc->idVendor);
        printf("PID: 0x%04X\n", dev_desc->idProduct);
        printf("Device Class: %u\n", dev_desc->bDeviceClass);
        printf("Device SubClass: %u\n", dev_desc->bDeviceSubClass);
        printf("Device Protocol: %u\n", dev_desc->bDeviceProtocol);
        printf("Max Packet Size: %u\n", dev_desc->bMaxPacketSize0);
        
        detected_vid = dev_desc->idVendor;
        detected_pid = dev_desc->idProduct;
        gamepad_detected = true;
        
        printf("\n=== Gamepad Detected! ===\n");
        printf("VID: 0x%04X, PID: 0x%04X\n", detected_vid, detected_pid);
        printf("Add this to _is_xinput_device() in xinput_host.c:\n");
        printf("(vid == 0x%04X && pid == 0x%04X)\n", detected_vid, detected_pid);
    } else {
        printf("Failed to get device descriptor\n");
    }
}

void tuh_umount_cb(uint8_t dev_addr) {
    printf("\n[USB Device Unmounted]\n");
    printf("Device address: %u\n", dev_addr);
    gamepad_detected = false;
}

// HID设备回调
bool tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    printf("\n[HID Device Mounted]\n");
    printf("Device address: %u, Instance: %u\n", dev_addr, instance);
    printf("Report descriptor length: %u\n", desc_len);
    
    // 尝试打开HID设备
    if (tuh_hid_receive_report(dev_addr, instance)) {
        printf("HID device opened successfully\n");
    } else {
        printf("Failed to open HID device\n");
    }
    
    return true;
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("\n[HID Device Unmounted]\n");
    printf("Device address: %u, Instance: %u\n", dev_addr, instance);
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    printf("HID report received - Length: %u\n", len);
    
    // 打印前几个字节
    printf("Data: ");
    for (int i = 0; i < (len > 16 ? 16 : len); i++) {
        printf("%02X ", report[i]);
    }
    printf("\n");
}

int main(void) {
    stdio_init_all();
    
    // 等待串口连接
    sleep_ms(2000);
    
    printf("\n=== Gamepad VID/PID Detector ===\n");
    printf("Connect your gamepad to the USB-A port\n");
    printf("Waiting for device connection...\n\n");
    
    // 初始化电源控制
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    gpio_put(POWER_PIN, 1); // 开启电源
    
    // 初始化USB主机
    tuh_init(0);
    
    // 主循环
    while (true) {
        tuh_task();
        sleep_ms(10);
    }
    
    return 0;
}