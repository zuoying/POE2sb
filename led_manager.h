#ifndef _LED_MANAGER_H_
#define _LED_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "usb_descriptors.h"

// LED引脚定义
#define LED_PIN 16
#define LED_FREQ 800000  // 800kHz

// 亮度级别
#define BRIGHTNESS_FULL    255    // 全亮度（手柄连接时）
#define BRIGHTNESS_LOW     64     // 低亮度（手柄未连接时）

// LED颜色定义（模式指示）
#define LED_COLOR_SYNC     0, 255, 0    // 绿色：同步模式
#define LED_COLOR_MAIN     0, 0, 255    // 蓝色：仅主模式
#define LED_COLOR_SUB      255, 0, 0    // 红色：仅副模式
#define LED_COLOR_INIT     0, 0, 255    // 蓝色：初始化
#define LED_COLOR_READY    0, 255, 255  // 青色：系统就绪
#define LED_COLOR_CONNECTED 0, 255, 0   // 绿色：手柄已连接
#define LED_COLOR_ERROR    255, 0, 0    // 红色：错误

// LED状态枚举
typedef enum {
    LED_STATE_INIT,        // 初始化中
    LED_STATE_READY,       // 系统就绪，等待手柄
    LED_STATE_CONNECTED,   // 手柄已连接
    LED_STATE_MODE_SYNC,   // 同步模式
    LED_STATE_MODE_MAIN,   // 仅主模式
    LED_STATE_MODE_SUB,    // 仅副模式
    LED_STATE_ERROR        // 错误状态
} led_state_t;

// LED初始化
void led_init(void);

// 设置LED颜色和亮度
void led_set_color(uint8_t r, uint8_t g, uint8_t b, uint8_t brightness);

// 设置LED状态
void led_set_state(led_state_t state);

// 根据工作模式和手柄连接状态更新LED
// connected: 手柄是否连接
// mode: 当前工作模式
void led_update(bool connected, work_mode_t mode);

// LED闪烁（用于指示状态变化）
void led_blink(uint8_t r, uint8_t g, uint8_t b, uint8_t times, uint16_t delay_ms);

#ifdef __cplusplus
}
#endif

#endif /* _LED_MANAGER_H_ */