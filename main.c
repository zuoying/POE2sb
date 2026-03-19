#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/rand.h"
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"

// 必须在 tusb.h 之前包含 pio_usb.h
#include "pio_usb.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "ws2812.pio.h"

// UART 调试输出
#define DEBUG_printf printf
#define DEBUG_LED 25

// ------------------------------------------------------------------
// 全局变量与枚举
// ------------------------------------------------------------------
typedef enum {
    MODE_SYNC = 0,    // 绿灯：同步模式
    MODE_MAIN_ONLY,   // 蓝灯：仅主角色 (手柄1)
    MODE_SUB_ONLY,    // 红灯：仅副角色 (手柄2)
    MODE_COUNT
} sync_mode_t;

static sync_mode_t current_mode = MODE_SYNC;
static bool last_toggle_state = false;
static xbox_report_t host_report;      // 从真实手柄读取的原始数据
static bool host_connected = false;

typedef struct {
    uint32_t last_update_ms;
    uint32_t delay_ms;
    xbox_report_t delayed_report;
} anti_detect_t;

static anti_detect_t ad_ctrl1 = {0, 1, {0}};
static anti_detect_t ad_ctrl2 = {0, 1, {0}};

// ------------------------------------------------------------------
// LED 定义
// ------------------------------------------------------------------
#define WS2812_PIN 16
#define WS2812_FREQ 800000

// ------------------------------------------------------------------
// WS2812 RGB LED 驱动 (官方方法)
// ------------------------------------------------------------------
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
    put_pixel(grb);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, WS2812_PIN, WS2812_FREQ, false);
    printf("WS2812 initialized on GPIO %d\r\n", WS2812_PIN);
}

void update_led_indicator(void) {
    switch (current_mode) {
        case MODE_SYNC:      put_rgb(0, 255, 0); break;    // Green
        case MODE_MAIN_ONLY: put_rgb(0, 0, 255); break;   // Blue
        case MODE_SUB_ONLY:  put_rgb(255, 0, 0); break;   // Red
        default:            put_rgb(255, 0, 0); break;
    }
}

// ------------------------------------------------------------------
// 核心逻辑
// ------------------------------------------------------------------
static int16_t apply_stick_offset(int16_t value) {
    if (value == 0) return 0;
    int16_t offset = (int16_t)((get_rand_32() % 9) - 4);
    int32_t new_val = (int32_t)value + offset;
    return (int16_t)(new_val > 32767 ? 32767 : (new_val < -32768 ? -32768 : new_val));
}

static uint8_t apply_trigger_offset(uint8_t value) {
    if (value == 0) return 0;
    int16_t offset = (int16_t)((get_rand_32() % 5) - 2);
    int16_t new_val = (int16_t)value + offset;
    return (uint8_t)(new_val > 255 ? 255 : (new_val < 0 ? 0 : new_val));
}

void process_and_send_reports(void) {
    uint32_t now = to_ms_since_boot(get_absolute_time());
    bool current_toggle = (host_report.buttons & (XBOX_BTN_BACK | XBOX_BTN_START)) == (XBOX_BTN_BACK | XBOX_BTN_START);
    if (current_toggle && !last_toggle_state) {
        current_mode = (current_mode + 1) % MODE_COUNT;
        update_led_indicator();
    }
    last_toggle_state = current_toggle;

    if (now - ad_ctrl1.last_update_ms >= ad_ctrl1.delay_ms) {
        if (current_mode == MODE_SYNC || current_mode == MODE_MAIN_ONLY) {
            ad_ctrl1.delayed_report = host_report;
            ad_ctrl1.delayed_report.left_stick_x = apply_stick_offset(host_report.left_stick_x);
            ad_ctrl1.delayed_report.left_stick_y = apply_stick_offset(host_report.left_stick_y);
            ad_ctrl1.delayed_report.right_stick_x = apply_stick_offset(host_report.right_stick_x);
            ad_ctrl1.delayed_report.right_stick_y = apply_stick_offset(host_report.right_stick_y);
            ad_ctrl1.delayed_report.left_trigger = apply_trigger_offset(host_report.left_trigger);
            ad_ctrl1.delayed_report.right_trigger = apply_trigger_offset(host_report.right_trigger);
        } else {
            memset(&ad_ctrl1.delayed_report, 0, sizeof(xbox_report_t));
            ad_ctrl1.delayed_report.report_size = 0x14;
        }
        ad_ctrl1.delay_ms = (get_rand_32() % 6) + 1;
        ad_ctrl1.last_update_ms = now;
        if (tud_ready()) {
            tud_vendor_n_write(0, &ad_ctrl1.delayed_report, sizeof(xbox_report_t));
            tud_vendor_n_flush(0);
        }
    }

    if (now - ad_ctrl2.last_update_ms >= ad_ctrl2.delay_ms) {
        if (current_mode == MODE_SYNC || current_mode == MODE_SUB_ONLY) {
            ad_ctrl2.delayed_report = host_report;
            ad_ctrl2.delayed_report.left_stick_x = apply_stick_offset(host_report.left_stick_x);
            ad_ctrl2.delayed_report.left_stick_y = apply_stick_offset(host_report.left_stick_y);
            ad_ctrl2.delayed_report.right_stick_x = apply_stick_offset(host_report.right_stick_x);
            ad_ctrl2.delayed_report.right_stick_y = apply_stick_offset(host_report.right_stick_y);
            ad_ctrl2.delayed_report.left_trigger = apply_trigger_offset(host_report.left_trigger);
            ad_ctrl2.delayed_report.right_trigger = apply_trigger_offset(host_report.right_trigger);
        } else {
            memset(&ad_ctrl2.delayed_report, 0, sizeof(xbox_report_t));
            ad_ctrl2.delayed_report.report_size = 0x14;
        }
        ad_ctrl2.delay_ms = (get_rand_32() % 6) + 1;
        ad_ctrl2.last_update_ms = now;
        if (tud_ready()) {
            tud_vendor_n_write(0, &ad_ctrl2.delayed_report, sizeof(xbox_report_t));
            tud_vendor_n_flush(0);
        }
    }
}

// ------------------------------------------------------------------
// USB Device 回调
// ------------------------------------------------------------------
void tud_mount_cb(void) {
    DEBUG_printf("TinyUSB: Device mounted\r\n");
}

void tud_umount_cb(void) {
    DEBUG_printf("TinyUSB: Device unmounted\r\n");
}

// ------------------------------------------------------------------
// USB Host 回调
// ------------------------------------------------------------------
void tuh_mount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device mounted (addr=%d)\r\n", dev_addr);
    host_connected = true;
}

void tuh_umount_cb(uint8_t dev_addr) {
    DEBUG_printf("TinyUSB: Host device unmounted (addr=%d)\r\n", dev_addr);
    host_connected = false;
    memset(&host_report, 0, sizeof(xbox_report_t));
}

// HID 报告接收回调
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    (void)instance;
    if (len >= sizeof(xbox_report_t)) {
        memcpy(&host_report, report, sizeof(xbox_report_t));
        process_and_send_reports();
    }
}

// ------------------------------------------------------------------
// 主函数
// ------------------------------------------------------------------
int main(void) {
    stdio_init_all();
    DEBUG_printf("\r\n=== POE2GamePad Starting ===\r\n");
    DEBUG_printf("RP2350 PIO-USB Xbox 360 Controller Sync\r\n");

    init_ws2812();
    DEBUG_printf("WS2812 initialized\r\n");

    current_mode = MODE_SYNC;
    update_led_indicator();
    DEBUG_printf("LED set to GREEN (SYNC mode)\r\n");

    DEBUG_printf("Initializing TinyUSB Device (tusb_init)...\r\n");
    tusb_init();
    DEBUG_printf("TinyUSB Device initialized\r\n");

    DEBUG_printf("Configuring PIO-USB Host...\r\n");
    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = 12;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);

    DEBUG_printf("Initializing TinyUSB Host (tuh_init)...\r\n");
    if (!tuh_init(1)) {
        DEBUG_printf("ERROR: tuh_init failed!\r\n");
    } else {
        DEBUG_printf("TinyUSB Host initialized on rhport 1\r\n");
    }

    DEBUG_printf("Entering main loop...\r\n");

    while (1) {
        tud_task();
        tuh_task();
    }
    return 0;
}

void tud_vendor_rx_cb(uint8_t itf) {
    uint8_t buf[32];
    tud_vendor_n_read(itf, buf, sizeof(buf));
}
