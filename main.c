#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "pio_usb.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "ws2812.pio.h"

#define DEBUG_printf printf

typedef enum {
    MODE_SYNC = 0,
    MODE_MAIN_ONLY,
    MODE_SUB_ONLY,
    MODE_COUNT
} sync_mode_t;

static sync_mode_t current_mode = MODE_SYNC;
static bool host_connected = false;
static hid_report_t host_report;

#define ANTI_CHEAT_STRENGTH 30

typedef struct {
    uint32_t last_update_ms;
    uint32_t delay_ms;
    hid_report_t delayed_report;
} anti_detect_t;

static anti_detect_t ad_ctrl = {0, 1, {0}};
static volatile bool core1_ready = false;

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

void set_led_by_mode(void) {
    uint8_t brightness = host_connected ? 200 : 80;
    switch (current_mode) {
        case MODE_SYNC: put_rgb(0, brightness, 0); break;
        case MODE_MAIN_ONLY: put_rgb(0, 0, brightness); break;
        case MODE_SUB_ONLY: put_rgb(brightness, 0, 0); break;
    }
}

static int16_t calc_offset(int16_t range, uint8_t strength) {
    if (strength == 0 || range == 0) return 0;
    int16_t scaled = (range * strength) / 100;
    if (scaled < 1) scaled = 1;
    return (int16_t)((get_rand_32() % (scaled * 2 + 1)) - scaled);
}

static int8_t offset_stick(int8_t v, uint8_t s) {
    if (v == 0) return 0;
    int16_t new_val = (int16_t)v + calc_offset(2, s);
    return (int8_t)(new_val > 127 ? 127 : (new_val < -128 ? -128 : new_val));
}

static uint8_t offset_trigger(uint8_t v, uint8_t s) {
    if (v == 0) return 0;
    int16_t new_val = (int16_t)v + calc_offset(2, s);
    return (uint8_t)(new_val > 255 ? 255 : (new_val < 0 ? 0 : new_val));
}

void process_gamepad(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (now - ad_ctrl.last_update_ms >= ad_ctrl.delay_ms) {
        ad_ctrl.delayed_report = host_report;

        uint8_t s = ANTI_CHEAT_STRENGTH;
        ad_ctrl.delayed_report.left_x = offset_stick(host_report.left_x, s);
        ad_ctrl.delayed_report.left_y = offset_stick(host_report.left_y, s);
        ad_ctrl.delayed_report.right_x = offset_stick(host_report.right_x, s);
        ad_ctrl.delayed_report.right_y = offset_stick(host_report.right_y, s);
        ad_ctrl.delayed_report.left_trigger = offset_trigger(host_report.left_trigger, s);
        ad_ctrl.delayed_report.right_trigger = offset_trigger(host_report.right_trigger, s);

        ad_ctrl.delay_ms = (get_rand_32() % 6) + 1;
        ad_ctrl.last_update_ms = now;

        if (tud_ready() && host_connected) {
            tud_hid_report(0, &ad_ctrl.delayed_report, sizeof(hid_report_t));
        }
    }
}

void core1_main(void) {
    sleep_ms(100);

    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = PICO_PIO_USB_PIN_DP;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);

    core1_ready = true;

    while (1) {
        tuh_task();
        sleep_ms(10);
    }
}

void tud_mount_cb(void) {
    DEBUG_printf("Device mounted\r\n");
}

void tud_umount_cb(void) {
    DEBUG_printf("Device unmounted\r\n");
}

void tuh_mount_cb(uint8_t dev_addr) {
    DEBUG_printf("Host mount: dev=%d\r\n", dev_addr);
    host_connected = true;
    set_led_by_mode();
    tuh_hid_receive_report(dev_addr, 0);
}

void tuh_umount_cb(uint8_t dev_addr) {
    DEBUG_printf("Host unmount: dev=%d\r\n", dev_addr);
    host_connected = false;
    memset(&host_report, 0, sizeof(hid_report_t));
    set_led_by_mode();
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if (len >= sizeof(hid_report_t)) {
        memcpy(&host_report, report, sizeof(hid_report_t));
    }
    tuh_hid_receive_report(dev_addr, instance);
}

int main(void) {
    set_sys_clock_khz(120000, true);

    stdio_init_all();
    DEBUG_printf("\r\nPOE2GamePad v2\r\n");
    DEBUG_printf("Clock: %ldHz\r\n", clock_get_hz(clk_sys));

    gpio_init(18);
    gpio_set_dir(18, GPIO_OUT);
    gpio_put(18, 0);

    init_ws2812();
    current_mode = MODE_SYNC;
    set_led_by_mode();

    DEBUG_printf("Init TinyUSB...\r\n");
    tusb_init();
    DEBUG_printf("TinyUSB done\r\n");

    gpio_put(18, 1);

    multicore_launch_core1(core1_main);
    while (!core1_ready) tight_loop_contents();

    DEBUG_printf("Main loop\r\n");

    while (1) {
        tud_task();
        if (host_connected) {
            process_gamepad();
        }
        static uint32_t last_led = 0;
        if (to_ms_since_boot(get_absolute_time()) - last_led > 100) {
            set_led_by_mode();
            last_led = to_ms_since_boot(get_absolute_time());
        }
    }
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t const* b, uint16_t s) {(void)itf;(void)id;(void)t;(void)b;(void)s;}
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t id, hid_report_type_t t, uint8_t* b, uint16_t r) {(void)itf;(void)id;(void)t;(void)b;memset(b,0,r);return r;}
