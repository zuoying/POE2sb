#include "mode_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 全局工作模式
work_mode_t current_mode = SYNC_MODE;

// 模式切换状态
static uint32_t _menu_view_press_start = 0;
static bool _menu_pressed = false;
static bool _view_pressed = false;

// 反作弊随机偏移状态
static int8_t _joystick_offset_lx = 0;
static int8_t _joystick_offset_ly = 0;
static int8_t _joystick_offset_rx = 0;
static int8_t _joystick_offset_ry = 0;
static int8_t _trigger_offset_left = 0;
static int8_t _trigger_offset_right = 0;
static uint16_t _delay_offset_ms = 0;

// 初始化模式管理器
void mode_manager_init(void) {
    current_mode = SYNC_MODE;
    _menu_view_press_start = 0;
    _menu_pressed = false;
    _view_pressed = false;
    
    // 初始化随机种子（使用时间作为种子）
    srand(to_ms_since_boot(get_absolute_time()));
    
    // 生成初始随机偏移
    _joystick_offset_lx = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_ly = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_rx = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_ry = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _trigger_offset_left = (rand() % (ANTICHEAT_TRIGGER_OFFSET * 2 + 1)) - ANTICHEAT_TRIGGER_OFFSET;
    _trigger_offset_right = (rand() % (ANTICHEAT_TRIGGER_OFFSET * 2 + 1)) - ANTICHEAT_TRIGGER_OFFSET;
    _delay_offset_ms = ANTICHEAT_DELAY_MIN_MS + (rand() % (ANTICHEAT_DELAY_MAX_MS - ANTICHEAT_DELAY_MIN_MS + 1));
    
    printf("Mode manager initialized. Current mode: %s\n", mode_manager_get_name(current_mode));
    printf("Anti-cheat offsets: LX=%d, LY=%d, RX=%d, RY=%d, LT=%d, RT=%d, Delay=%dms\n",
           _joystick_offset_lx, _joystick_offset_ly, _joystick_offset_rx, _joystick_offset_ry,
           _trigger_offset_left, _trigger_offset_right, _delay_offset_ms);
}

// 检测模式切换
bool mode_manager_check_switch(uint16_t buttons) {
    bool menu_pressed = (buttons & BTN_MENU) != 0;
    bool view_pressed = (buttons & BTN_VIEW) != 0;
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // 检测Menu+View同时按下
    if (menu_pressed && view_pressed) {
        if (!_menu_pressed || !_view_pressed) {
            // 刚开始同时按下
            _menu_pressed = true;
            _view_pressed = true;
            _menu_view_press_start = current_time;
        } else {
            // 持续按下，检查是否达到长按时间
            if ((current_time - _menu_view_press_start) >= MODE_SWITCH_LONG_PRESS_MS) {
                // 切换模式
                current_mode = (work_mode_t)((current_mode + 1) % 3);
                
                // 重置状态
                _menu_pressed = false;
                _view_pressed = false;
                _menu_view_press_start = 0;
                
                printf("Mode switched to: %s\n", mode_manager_get_name(current_mode));
                return true;
            }
        }
    } else {
        // 未同时按下，重置状态
        _menu_pressed = false;
        _view_pressed = false;
        _menu_view_press_start = 0;
    }
    
    return false;
}

// 应用反作弊偏移到副控制器报告
static void _apply_anticheat_offsets(xinput_report_t* report) {
    if (!report) return;
    
    // 应用摇杆偏移
    report->lx += _joystick_offset_lx;
    report->ly += _joystick_offset_ly;
    report->rx += _joystick_offset_rx;
    report->ry += _joystick_offset_ry;
    
    // 应用扳机偏移
    int16_t left_trigger = report->left_trigger + _trigger_offset_left;
    int16_t right_trigger = report->right_trigger + _trigger_offset_right;
    
    // 确保值在有效范围内
    if (left_trigger < 0) left_trigger = 0;
    if (left_trigger > 255) left_trigger = 255;
    if (right_trigger < 0) right_trigger = 0;
    if (right_trigger > 255) right_trigger = 255;
    
    report->left_trigger = (int8_t)left_trigger;
    report->right_trigger = (int8_t)right_trigger;
    
    // 确保摇杆值在有效范围内
    if (report->lx < -32768) report->lx = -32768;
    if (report->lx > 32767) report->lx = 32767;
    if (report->ly < -32768) report->ly = -32768;
    if (report->ly > 32767) report->ly = 32767;
    if (report->rx < -32768) report->rx = -32768;
    if (report->rx > 32767) report->rx = 32767;
    if (report->ry < -32768) report->ry = -32768;
    if (report->ry > 32767) report->ry = 32767;
}

// 根据模式处理XInput报告
void mode_manager_process_report(const xinput_report_t* report, 
                                 work_mode_t mode,
                                 xinput_report_t* main_out,
                                 xinput_report_t* sub_out) {
    if (!report || !main_out || !sub_out) return;
    
    // 复制原始报告
    memcpy(main_out, report, sizeof(xinput_report_t));
    memcpy(sub_out, report, sizeof(xinput_report_t));
    
    // 根据模式处理
    switch (mode) {
        case SYNC_MODE:
            // 同步模式：两个控制器都输出，副控制器应用反作弊偏移
            _apply_anticheat_offsets(sub_out);
            break;
            
        case MAIN_MODE:
            // 仅主模式：副控制器输出零报告
            memset(sub_out, 0, sizeof(xinput_report_t));
            sub_out->report_id = 0x00;
            break;
            
        case SUB_MODE:
            // 仅副模式：主控制器输出零报告，副控制器应用反作弊偏移
            memset(main_out, 0, sizeof(xinput_report_t));
            main_out->report_id = 0x00;
            _apply_anticheat_offsets(sub_out);
            break;
            
        default:
            // 默认使用同步模式
            _apply_anticheat_offsets(sub_out);
            break;
    }
}

// 获取模式名称字符串
const char* mode_manager_get_name(work_mode_t mode) {
    switch (mode) {
        case SYNC_MODE: return "SYNC (Both)";
        case MAIN_MODE: return "MAIN (Controller 1 only)";
        case SUB_MODE:  return "SUB (Controller 2 only)";
        default:        return "UNKNOWN";
    }
}

// 生成新的随机偏移（可定期调用）
void mode_manager_generate_new_offsets(void) {
    _joystick_offset_lx = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_ly = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_rx = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _joystick_offset_ry = (rand() % (ANTICHEAT_JOYSTICK_OFFSET * 2 + 1)) - ANTICHEAT_JOYSTICK_OFFSET;
    _trigger_offset_left = (rand() % (ANTICHEAT_TRIGGER_OFFSET * 2 + 1)) - ANTICHEAT_TRIGGER_OFFSET;
    _trigger_offset_right = (rand() % (ANTICHEAT_TRIGGER_OFFSET * 2 + 1)) - ANTICHEAT_TRIGGER_OFFSET;
    _delay_offset_ms = ANTICHEAT_DELAY_MIN_MS + (rand() % (ANTICHEAT_DELAY_MAX_MS - ANTICHEAT_DELAY_MIN_MS + 1));
    
    printf("New anti-cheat offsets: LX=%d, LY=%d, RX=%d, RY=%d, LT=%d, RT=%d, Delay=%dms\n",
           _joystick_offset_lx, _joystick_offset_ly, _joystick_offset_rx, _joystick_offset_ry,
           _trigger_offset_left, _trigger_offset_right, _delay_offset_ms);
}

// 获取反作弊延迟（毫秒）
uint16_t mode_manager_get_anticheat_delay(void) {
    return _delay_offset_ms;
}