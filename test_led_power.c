#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "ws2812.pio.h"

// 硬件定义
#define LED_PIN 16
#define POWER_PIN 18

// WS2812定义
#define WS2812_FREQ 800000

// WS2812 LED驱动函数
static inline void put_pixel(uint32_t pixel_data) {
    pio_sm_put_blocking(pio0, 0, pixel_data);
}

static inline void put_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // RGB格式：GRB
    uint32_t grb = ((uint32_t)g << 16) | ((uint32_t)r << 8) | ((uint32_t)b << 0);
    put_pixel(grb);
}

void init_ws2812(void) {
    uint offset = pio_add_program(pio0, &ws2812_program);
    ws2812_program_init(pio0, 0, offset, LED_PIN, WS2812_FREQ, false);
    puts("WS2812 LED initialized");
}

// LED闪烁函数
void led_blink(int times, int delay_ms) {
    for (int i = 0; i < times; i++) {
        put_rgb(255, 255, 255); // 白色闪烁
        sleep_ms(delay_ms);
        put_rgb(0, 0, 0);       // 关闭
        sleep_ms(delay_ms);
    }
}

// 设置LED颜色
void set_led_color(uint8_t r, uint8_t g, uint8_t b) {
    put_rgb(r, g, b);
}

// 初始化硬件
void init_hardware(void) {
    // 初始化LED
    init_ws2812();
    put_rgb(0, 0, 255); // 蓝色：初始化中
    
    // 初始化5V电源控制
    gpio_init(POWER_PIN);
    gpio_set_dir(POWER_PIN, GPIO_OUT);
    gpio_set_drive_strength(POWER_PIN, GPIO_DRIVE_STRENGTH_16MA); // 最大驱动能力
    gpio_put(POWER_PIN, 0); // 先关闭电源
    
    printf("Hardware initialized\n");
}

// 主函数
int main(void) {
    stdio_init_all();
    printf("\n=== LED and Power Test ===\n");
    
    // 初始化硬件
    init_hardware();
    led_blink(3, 200); // 启动闪烁
    
    // 设置系统时钟
    printf("Setting system clock to 120MHz\n");
    set_sys_clock_khz(120000, true);
    
    // 测试电源控制
    printf("Testing power control\n");
    
    // 依次测试不同颜色和电源状态
    for (int i = 0; i < 5; i++) {
        // 红色 - 电源关闭
        set_led_color(255, 0, 0);
        gpio_put(POWER_PIN, 0);
        printf("LED: Red, Power: OFF\n");
        sleep_ms(1000);
        
        // 绿色 - 电源开启
        set_led_color(0, 255, 0);
        gpio_put(POWER_PIN, 1);
        printf("LED: Green, Power: ON\n");
        sleep_ms(1000);
        
        // 蓝色 - 电源开启
        set_led_color(0, 0, 255);
        gpio_put(POWER_PIN, 1);
        printf("LED: Blue, Power: ON\n");
        sleep_ms(1000);
        
        // 白色 - 电源关闭
        set_led_color(255, 255, 255);
        gpio_put(POWER_PIN, 0);
        printf("LED: White, Power: OFF\n");
        sleep_ms(1000);
    }
    
    // 最终状态：绿色常亮，电源开启
    set_led_color(0, 255, 0);
    gpio_put(POWER_PIN, 1);
    printf("Test completed. LED: Green, Power: ON\n");
    
    while (1) {
        // 保持电源开启
        gpio_put(POWER_PIN, 1);
        sleep_ms(100);
    }
}
