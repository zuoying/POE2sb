#include "led_manager.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"
#include <stdio.h>

// PIO和状态机配置
static PIO _pio = NULL;
static uint _sm = 0;
static uint _offset = 0;

// 当前LED状态
static led_state_t _current_state = LED_STATE_INIT;

// 初始化WS2812 LED
void led_init(void) {
    printf("Initializing WS2812 LED on GPIO%d\n", LED_PIN);
    
    // 设置PIO - 改用PIO1避免与PIO-USB主机冲突
    _pio = pio1;  // RP2040有PIO0和PIO1两个实例
    _sm = pio_claim_unused_sm(_pio, true);
    _offset = pio_add_program(_pio, &ws2812_program);
    
    // 初始化WS2812程序
    ws2812_program_init(_pio, _sm, _offset, LED_PIN, LED_FREQ, false);
    
    // 设置初始状态
    led_set_state(LED_STATE_INIT);
    printf("LED initialized on PIO1\n");
}

// 设置LED颜色和亮度
void led_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness) {
    if (!_pio) return;
    
    // 应用亮度调整
    uint8_t adjusted_r = (uint16_t)r * brightness / 255;
    uint8_t adjusted_g = (uint16_t)g * brightness / 255;
    uint8_t adjusted_b = (uint16_t)b * brightness / 255;
    
    // WS2812使用GRB格式
    uint32_t grb = ((uint32_t)adjusted_g << 16) | 
                   ((uint32_t)adjusted_r << 8) | 
                   ((uint32_t)adjusted_b << 0);
    
    // 发送颜色数据
    pio_sm_put_blocking(_pio, _sm, grb);
}

// 设置LED状态
void led_set_state(led_state_t state) {
    _current_state = state;
    
    switch (state) {
        case LED_STATE_INIT:
            led_set_color(LED_COLOR_INIT, BRIGHTNESS_LOW);
            break;
            
        case LED_STATE_READY:
            led_set_color(LED_COLOR_READY, BRIGHTNESS_LOW);
            break;
            
        case LED_STATE_CONNECTED:
            led_set_color(LED_COLOR_CONNECTED, BRIGHTNESS_FULL);
            break;
            
        case LED_STATE_MODE_SYNC:
            led_set_color(LED_COLOR_SYNC, BRIGHTNESS_FULL);
            break;
            
        case LED_STATE_MODE_MAIN:
            led_set_color(LED_COLOR_MAIN, BRIGHTNESS_FULL);
            break;
            
        case LED_STATE_MODE_SUB:
            led_set_color(LED_COLOR_SUB, BRIGHTNESS_FULL);
            break;
            
        case LED_STATE_ERROR:
            led_set_color(LED_COLOR_ERROR, BRIGHTNESS_FULL);
            break;
            
        default:
            led_set_color(LED_COLOR_READY, BRIGHTNESS_LOW);
            break;
    }
}

// 根据工作模式和手柄连接状态更新LED
void led_update(bool connected, work_mode_t mode) {
    if (!connected) {
        // 手柄未连接：显示就绪状态（低亮度）
        led_set_state(LED_STATE_READY);
        return;
    }
    
    // 手柄已连接：根据模式显示不同颜色
    switch (mode) {
        case SYNC_MODE:
            led_set_state(LED_STATE_MODE_SYNC);
            break;
            
        case MAIN_MODE:
            led_set_state(LED_STATE_MODE_MAIN);
            break;
            
        case SUB_MODE:
            led_set_state(LED_STATE_MODE_SUB);
            break;
            
        default:
            led_set_state(LED_STATE_CONNECTED);
            break;
    }
}

// LED闪烁（用于指示状态变化）
void led_blink(uint8_t r, uint8_t g, uint8_t b, uint8_t times, uint16_t delay_ms) {
    for (uint8_t i = 0; i < times; i++) {
        // 亮
        led_set_color(r, g, b, BRIGHTNESS_FULL);
        sleep_ms(delay_ms);
        
        // 灭
        led_set_color(0, 0, 0, BRIGHTNESS_FULL);
        sleep_ms(delay_ms);
    }
    
    // 恢复之前的状态
    led_set_state(_current_state);
}

// 辅助函数：获取当前亮度（根据手柄连接状态）
static uint8_t _get_brightness(bool connected) {
    return connected ? BRIGHTNESS_FULL : BRIGHTNESS_LOW;
}