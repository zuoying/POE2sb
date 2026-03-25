#ifndef _MODE_MANAGER_H_
#define _MODE_MANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "usb_descriptors.h"

// 按钮映射（XInput标准）
#define BTN_MENU   0x0400  // Menu/Start按钮
#define BTN_VIEW   0x0020  // View/Back按钮

// 模式切换长按时间（毫秒）
#define MODE_SWITCH_LONG_PRESS_MS 1000

// 反作弊偏移参数
#define ANTICHEAT_JOYSTICK_OFFSET 4    // 摇杆偏移量（±4）
#define ANTICHEAT_TRIGGER_OFFSET  2    // 扳机偏移量（±2）
#define ANTICHEAT_DELAY_MIN_MS    1    // 最小延迟（毫秒）
#define ANTICHEAT_DELAY_MAX_MS    6    // 最大延迟（毫秒）

// 全局工作模式
extern work_mode_t current_mode;

// 初始化模式管理器
void mode_manager_init(void);

// 检测模式切换（基于按钮状态）
// buttons: 当前按钮状态（XInput格式）
// 返回值：true=模式已切换，false=模式未切换
bool mode_manager_check_switch(uint16_t buttons);

// 根据模式处理XInput报告
// 参数：
//   report: 原始XInput报告
//   mode: 当前工作模式
//   main_out: 输出给主控制器的报告
//   sub_out: 输出给副控制器的报告
void mode_manager_process_report(const xinput_report_t* report, 
                                 work_mode_t mode,
                                 xinput_report_t* main_out,
                                 xinput_report_t* sub_out);

// 获取模式名称字符串
const char* mode_manager_get_name(work_mode_t mode);

// 获取反作弊延迟（毫秒）
uint16_t mode_manager_get_anticheat_delay(void);

#ifdef __cplusplus
}
#endif

#endif /* _MODE_MANAGER_H_ */