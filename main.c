#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "pico/multicore.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"

#include "pio_usb.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "ws2812.pio.h"

#define DEBUG_printf printf
#define DEBUG_LED 25

typedef enum {
    MODE_SYNC = 0,
    MODE_MAIN_ONLY,
    MODE_SUB_ONLY,
    MODE_COUNT
} sync_mode_t;

static sync_mode_t current_mode = MODE_SYNC;
static bool last_toggle_state = false;
static hid_report_t host_report;
static bool host_connected = false;

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

static inline void put_off(void) {
    put_rgb(0, 0, 0);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, false);
    DEBUG_printf("WS2812 initialized on GPIO %d\r\n", WS2812_PIN);
}

void update_led_indicator(void) {
    uint8_t brightness = host_connected ? 255 : 64;

    switch (current_mode) {
        case MODE_SYNC:
            put_rgb(0, brightness, 0);
            break;
        case MODE_MAIN_ONLY:
            put_rgb(0, 0, brightness);
            break;
        case MODE_SUB_ONLY:
            put_rgb(brightness, 0, 0);
            break;
        default:
            put_rgb(brightness, 0, 0);
            break;
    }
}

void flash_white_3times(void) {
    for (int i = 0; i < 3; i++) {
        put_rgb(255, 255, 255);
        sleep_ms(100);
        put_off();
        sleep_ms(100);
    }
}

static int16_t calculate_offset(int16_t base_range, uint8_t strength) {
    if (strength == 0) return 0;
    int16_t scaled_range = (base_range * strength) / 100;
    if (scaled_range < 1) scaled_range = 1;
    return (int16_t)((get_rand_32() % (scaled_range * 2 + 1)) - scaled_range);
}

static int8_t apply_stick_offset(int8_t value, uint8_t strength) {
    if (value == 0) return 0;
    int16_t offset = calculate_offset(2, strength);
    int32_t new_val = (int32_t)value + offset;
    return (int8_t)(new_val > 127 ? 127 : (new_val < -128 ? -128 : new_val));
}

static uint8_t apply_trigger_offset(uint8_t value, uint8_t strength) {
    if (value == 0) return 0;
    int16_t offset = calculate_offset(2, strength);
    int16_t new_val = (int16_t)value + offset;
    return (uint8_t)(new_val > 255 ? 255 : (new_val < 0 ? 0 : new_val));
}

static bool check_mode_toggle(void) {
    static uint32_t last_toggle_time = 0;
    bool current_toggle = (host_report.buttons & (BTN_BACK | BTN_START)) == (BTN_BACK | BTN_START);
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (current_toggle && !last_toggle_state) {
        if (now - last_toggle_time > 300) {
            last_toggle_time = now;
            return true;
        }
    }
    last_toggle_state = current_toggle;
    return false;
}

void process_and_send_reports(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());

    if (host_connected && check_mode_toggle()) {
        current_mode = (current_mode + 1) % MODE_COUNT;
        flash_white_3times();
        update_led_indicator();
        DEBUG_printf("Mode changed to: %d\r\n", current_mode);
    }

    if (now - ad_ctrl.last_update_ms >= ad_ctrl.delay_ms) {
        ad_ctrl.delayed_report = host_report;

        uint8_t strength = ANTI_CHEAT_STRENGTH;

        ad_ctrl.delayed_report.left_x = apply_stick_offset(host_report.left_x, strength);
        ad_ctrl.delayed_report.left_y = apply_stick_offset(host_report.left_y, strength);
        ad_ctrl.delayed_report.right_x = apply_stick_offset(host_report.right_x, strength);
        ad_ctrl.delayed_report.right_y = apply_stick_offset(host_report.right_y, strength);
        ad_ctrl.delayed_report.left_trigger = apply_trigger_offset(host_report.left_trigger, strength);
        ad_ctrl.delayed_report.right_trigger = apply_trigger_offset(host_report.right_trigger, strength);

        ad_ctrl.delay_ms = (get_rand_32() % 6) + 1;
        ad_ctrl.last_update_ms = now;

        if (tud_ready()) {
            tud_hid_report(0, &ad_ctrl.delayed_report, sizeof(hid_report_t));
        }
    }
}

void core1_main(void) {
    DEBUG_printf("Core1: Initializing USB Host...\r\n");

    sleep_ms(100);

    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = PICO_PIO_USB_PIN_DP;

    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);

    core1_ready = true;
    DEBUG_printf("Core1: USB Host initialized\r\n");

    while (1) {
        tuh_task();
        sleep_ms(10);
    }
}

void tud_mount_cb(void) {
    DEBUG_printf("TinyUSB: Device mounted\r\n");
}

void tud_umount_cb(void) {
    DEBUG_printf("TinyUSB: Device unmounted\r\n");
}

void tuh_mount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device mounted (addr=%d)\r\n", dev_addr);
    host_connected = true;
    tuh_hid_receive_report(dev_addr, 0);
}

void tuh_umount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device unmounted (addr=%d)\r\n", dev_addr);
    host_connected = false;
    memset(&host_report, 0, sizeof(hid_report_t));
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)dev_addr;
    (void)instance;

    if (len >= 8) {
        memcpy(&host_report, report, sizeof(hid_report_t));
    }

    if (tuh_hid_receive_report(dev_addr, instance) == false) {
        DEBUG_printf("Error: cannot request report\r\n");
    }
}

int main(void) {
    set_sys_clock_khz(120000, true);

    stdio_init_all();
    DEBUG_printf("\r\n=== POE2GamePad Starting ===\r\n");
    DEBUG_printf("System clock: %ld Hz\r\n", clock_get_hz(clk_sys));

    const uint32_t GPIO_5V_EN = 18;
    gpio_init(GPIO_5V_EN);
    gpio_set_dir(GPIO_5V_EN, GPIO_OUT);
    gpio_set_drive_strength(GPIO_5V_EN, GPIO_DRIVE_STRENGTH_16MA);
    gpio_put(GPIO_5V_EN, 0);

    init_ws2812();

    current_mode = MODE_SYNC;
    update_led_indicator();

    DEBUG_printf("Initializing TinyUSB Device...\r\n");
    tusb_init();
    DEBUG_printf("TinyUSB Device initialized\r\n");

    gpio_put(GPIO_5V_EN, 1);
    sleep_ms(500);

    DEBUG_printf("Starting Core1...\r\n");
    multicore_launch_core1(core1_main);

    while (!core1_ready) {
        tight_loop_contents();
    }

    DEBUG_printf("Core0: Entering main loop...\r\n");

    while (1) {
        tud_task();

        static uint32_t last_led_update = 0;
        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_led_update >= 100) {
            update_led_indicator();
            last_led_update = now;
        }

        if (host_connected) {
            process_and_send_reports();
        }
    }
    return 0;
}
