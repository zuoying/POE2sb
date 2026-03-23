#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "tusb.h"

int main(void) {
    set_sys_clock_khz(120000, true);
    stdio_init_all();
    
    printf("\nRP2350 USB Device Test\n");
    
    tusb_init();
    
    while (1) {
        tud_task();
        sleep_ms(10);
    }
}

void tud_mount_cb(void) {
    printf("Device mounted\n");
}

void tud_umount_cb(void) {
    printf("Device unmounted\n");
}

uint8_t const* tud_descriptor_device_cb(void) {
    static uint8_t desc_device[] = {
        18, TUSB_DESC_DEVICE,
        0x00, 0x02,
        0x00,
        0x00,
        0x00,
        64,
        0x12, 0x34,
        0x56, 0x78,
        0x00, 0x01,
        1, 2, 3,
        1
    };
    return desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    static uint8_t desc_config[] = {
        9, TUSB_DESC_CONFIGURATION,
        32, 0,
        1, 0, 0, 0x80, 0x32,
        9, TUSB_DESC_INTERFACE,
        0, 0, 1, 3, 0, 0, 0,
        9, 0x21,
        0x11, 0x01, 0, 1, 0x22, 34, 0,
        7, TUSB_DESC_ENDPOINT,
        0x81, 3, 64, 0, 1
    };
    return desc_config;
}

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;
    static uint16_t desc_str[] = {
        4, 3, 9, 4,
        10, 3, 'T','e','s','t',' ','D','e','v','i','c','e',
        10, 3, 'G','a','m','e','p','a','d',
        8, 3, '0','0','0','1'
    };
    uint16_t* p = desc_str;
    for (int i = 0; i < index && p[0]; i++) {
        p += p[0]/2 + 1;
    }
    return p[0] ? p : NULL;
}

uint8_t const* tud_hid_descriptor_report_cb(uint8_t itf) {
    (void)itf;
    static uint8_t desc_hid_report[] = {
        0x05, 0x01,
        0x09, 0x05,
        0xA1, 0x01,
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
        0x05, 0x09,
        0x19, 0x01,
        0x29, 0x08,
        0x15, 0x00,
        0x25, 0x01,
        0x75, 0x01,
        0x95, 0x08,
        0x81, 0x02,
        0xC0
    };
    return desc_hid_report;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t id, uint8_t t, uint8_t const* b, uint16_t s) {
    (void)itf;(void)id;(void)t;(void)b;(void)s;
}

uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t id, uint8_t t, uint8_t* b, uint16_t r) {
    (void)itf;(void)id;(void)t;(void)b;
    memset(b, 0, r);
    return r;
}
