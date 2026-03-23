#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/pio.h"

#include "tusb.h"

#include "ws2812.pio.h"

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
    
    init_ws2812();
    put_rgb(0, 0, 100);
    
    printf("\r\nPOE2 v7 - CDC Test\r\n");
    
    tusb_init();
    
    uint32_t tick = 0;
    while (1) {
        tud_task();
        
        if (tud_cdc_connected()) {
            printf("CDC connected\r\n");
            for (int i = 0; i < 10; i++) {
                tud_cdc_write_str("Hello!\r\n");
                tud_cdc_write_flush();
                sleep_ms(100);
            }
            while(1) { tud_task(); sleep_ms(1000); }
        }
        
        if (to_ms_since_boot(get_absolute_time()) - tick > 500) {
            put_rgb(0, (tick++ & 1) ? 100 : 0, 0);
            tick = to_ms_since_boot(get_absolute_time());
        }
        sleep_ms(10);
    }
}
