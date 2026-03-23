#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "tusb.h"

#include "usb_descriptors.h"
#include "ws2812.pio.h"

#define DEBUG_printf printf

#define WS2812_PIN 16
#define WS2812_FREQ 800000

static inline void put_pixel(uint32_t pixel_data) {
    pio_sm_put_blocking(pio0, 0, pixel_data);
}

static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | ((uint32_t)b << 0);
    put_pixel(grb);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, false);
}

int main(void) {
    set_sys_clock_khz(120000, true);

    stdio_init_all();
    printf("\r\nPOE2GamePad v6 - CDC+HID Test\r\n");

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);
    gpio_put(18, 0);

    init_ws2812();
    put_rgb(0, 0, 100);

    printf("Init TinyUSB...\r\n");
    tusb_init();
    printf("TinyUSB init done\r\n");

    gpio_put(18, 1);

    uint32_t tick = 0;
    hid_report_t report = {0};

    printf("Main loop\r\n");

    while (1) {
        tud_task();

        static uint32_t last_led = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_led > 500) {
            put_rgb(0, (tick & 1) ? 100 : 0, 0);
            tick++;
            last_led = now;
        }

        if (tud_ready()) {
            report.left_x = (int8_t)((get_rand_32() % 21) - 10);
            report.left_y = (int8_t)((get_rand_32() % 21) - 10);
            report.right_x = 0;
            report.right_y = 0;
            report.buttons = 0;
            report.hat = 0;
            report.report_id = 1;
            tud_hid_report(0, &report, sizeof(hid_report_t));
        }

        sleep_ms(10);
    }
}

void tud_mount_cb(void) {
    printf("Device mounted\r\n");
}

void tud_umount_cb(void) {
    printf("Device unmounted\r\n");
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const* b, uint16_t s) {(void)itf;(void)id;(void)t;(void)b;(void)s;}
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t* b, uint16_t r) {(void)itf;(void)id;(void)t;(void)b;memset(b,0,r);return r;}
