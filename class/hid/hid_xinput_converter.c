#include "class/hid/hid_host.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// 将原始HID数据转换为标准XInput报告
xinput_report_t hid_to_xinput_convert(const uint8_t* hid_data, uint16_t hid_len) {
    xinput_report_t xinput = {0};
    xinput.report_id = 0x00; // XInput标准报告ID
    
    if (hid_len == 0 || hid_data == NULL) {
        return xinput;
    }
    
    // 根据报告长度判断格式并转换
    if (hid_len >= 19 && hid_len <= 20) {
        // XInput格式（19-20字节）
        // 假设GameSir XInput格式：前2字节是按钮，然后是摇杆和扳机
        if (hid_len >= 2) {
            xinput.buttons = hid_data[0] | (hid_data[1] << 8);
        }
        
        // 扳机值（8位无符号）
        if (hid_len >= 4) {
            xinput.left_trigger = hid_data[2];
            xinput.right_trigger = hid_data[3];
        }
        
        // 左摇杆（16位有符号）
        if (hid_len >= 8) {
            xinput.lx = (int16_t)(hid_data[4] | (hid_data[5] << 8));
            xinput.ly = (int16_t)(hid_data[6] | (hid_data[7] << 8));
        }
        
        // 右摇杆（16位有符号）
        if (hid_len >= 12) {
            xinput.rx = (int16_t)(hid_data[8] | (hid_data[9] << 8));
            xinput.ry = (int16_t)(hid_data[10] | (hid_data[11] << 8));
        }
        
        // 保留字段
        if (hid_len >= 18) {
            memcpy(xinput.reserved, &hid_data[12], 6);
        }
        
    } else if (hid_len == 8) {
        // DInput格式（8字节）
        // 假设格式：字节0=右摇杆X，字节1=右摇杆Y，字节2=左摇杆X，字节3=左摇杆Y
        // 字节4=方向键+按钮低4位，字节5=按钮高4位+其他，字节6=左扳机，字节7=右扳机
        
        // 左摇杆（转换8位到16位）
        if (hid_len >= 4) {
            xinput.lx = (int16_t)(((int16_t)hid_data[2] - 128) * 256);
            xinput.ly = (int16_t)(((int16_t)hid_data[3] - 128) * 256);
        }
        
        // 右摇杆
        if (hid_len >= 2) {
            xinput.rx = (int16_t)(((int16_t)hid_data[0] - 128) * 256);
            xinput.ry = (int16_t)(((int16_t)hid_data[1] - 128) * 256);
        }
        
        // 扳机值
        if (hid_len >= 8) {
            xinput.left_trigger = hid_data[6];
            xinput.right_trigger = hid_data[7];
        }
        
        // 按钮映射（DInput到XInput）
        if (hid_len >= 6) {
            uint8_t buttons_low = hid_data[4];
            uint8_t buttons_high = hid_data[5];
            
            // 方向键
            if ((buttons_low & 0x0F) == 0x00) xinput.buttons |= BTN_UP;
            else if ((buttons_low & 0x0F) == 0x04) xinput.buttons |= BTN_RIGHT;
            else if ((buttons_low & 0x0F) == 0x08) xinput.buttons |= BTN_DOWN;
            else if ((buttons_low & 0x0F) == 0x0C) xinput.buttons |= BTN_LEFT;
            else if ((buttons_low & 0x0F) == 0x02) xinput.buttons |= (BTN_UP | BTN_RIGHT);
            else if ((buttons_low & 0x0F) == 0x01) xinput.buttons |= (BTN_UP | BTN_LEFT);
            else if ((buttons_low & 0x0F) == 0x06) xinput.buttons |= (BTN_DOWN | BTN_RIGHT);
            else if ((buttons_low & 0x0F) == 0x05) xinput.buttons |= (BTN_DOWN | BTN_LEFT);
            
            // 按钮
            if (buttons_low & 0x10) xinput.buttons |= BTN_START;   // Start
            if (buttons_low & 0x20) xinput.buttons |= BTN_BACK;    // Select
            if (buttons_low & 0x40) xinput.buttons |= BTN_L3;      // L3
            if (buttons_low & 0x80) xinput.buttons |= BTN_R3;      // R3
            
            if (buttons_high & 0x01) xinput.buttons |= BTN_LB;     // L1
            if (buttons_high & 0x02) xinput.buttons |= BTN_RB;     // R1
            if (buttons_high & 0x04) xinput.buttons |= BTN_A;      // Cross/A
            if (buttons_high & 0x08) xinput.buttons |= BTN_B;      // Circle/B
            if (buttons_high & 0x10) xinput.buttons |= BTN_X;      // Square/X
            if (buttons_high & 0x20) xinput.buttons |= BTN_Y;      // Triangle/Y
            if (buttons_high & 0x40) xinput.buttons |= BTN_GUIDE;  // Guide/PS
        }
        
    } else if (hid_len >= 6 && hid_len <= 10) {
        // 标准HID游戏手柄格式（6-10字节）
        // 假设格式：字节0=按钮低8位，字节1=方向键，字节2=左摇杆X，字节3=左摇杆Y
        // 字节4=右摇杆X，字节5=右摇杆Y，字节6=左扳机，字节7=右扳机
        
        // 按钮
        if (hid_len >= 1) {
            uint8_t buttons = hid_data[0];
            if (buttons & 0x01) xinput.buttons |= BTN_A;
            if (buttons & 0x02) xinput.buttons |= BTN_B;
            if (buttons & 0x04) xinput.buttons |= BTN_X;
            if (buttons & 0x08) xinput.buttons |= BTN_Y;
            if (buttons & 0x10) xinput.buttons |= BTN_LB;
            if (buttons & 0x20) xinput.buttons |= BTN_RB;
            if (buttons & 0x40) xinput.buttons |= BTN_BACK;
            if (buttons & 0x80) xinput.buttons |= BTN_START;
        }
        
        // 方向键（第二个字节）
        if (hid_len >= 2) {
            uint8_t hat = hid_data[1];
            if (hat == 0x00) xinput.buttons |= BTN_UP;
            else if (hat == 0x01) xinput.buttons |= (BTN_UP | BTN_RIGHT);
            else if (hat == 0x02) xinput.buttons |= BTN_RIGHT;
            else if (hat == 0x03) xinput.buttons |= (BTN_DOWN | BTN_RIGHT);
            else if (hat == 0x04) xinput.buttons |= BTN_DOWN;
            else if (hat == 0x05) xinput.buttons |= (BTN_DOWN | BTN_LEFT);
            else if (hat == 0x06) xinput.buttons |= BTN_LEFT;
            else if (hat == 0x07) xinput.buttons |= (BTN_UP | BTN_LEFT);
        }
        
        // 摇杆（8位到16位转换）
        if (hid_len >= 4) {
            xinput.lx = (int16_t)(((int16_t)hid_data[2] - 128) * 256);
            xinput.ly = (int16_t)(((int16_t)hid_data[3] - 128) * 256);
        }
        
        if (hid_len >= 6) {
            xinput.rx = (int16_t)(((int16_t)hid_data[4] - 128) * 256);
            xinput.ry = (int16_t)(((int16_t)hid_data[5] - 128) * 256);
        }
        
        // 扳机
        if (hid_len >= 8) {
            xinput.left_trigger = hid_data[6];
            xinput.right_trigger = hid_data[7];
        }
        
    } else {
        // 未知格式，尝试通用解析
        printf("Unknown HID format (%u bytes), attempting generic conversion\n", hid_len);
        
        // 简单映射：假设前几个字节包含有用信息
        if (hid_len >= 2) {
            // 尝试将前2字节解释为按钮
            xinput.buttons = hid_data[0] | (hid_data[1] << 8);
        }
        
        // 如果有足够的数据，尝试解析摇杆
        if (hid_len >= 6) {
            xinput.lx = (int16_t)(hid_data[2] | (hid_data[3] << 8));
            xinput.ly = (int16_t)(hid_data[4] | (hid_data[5] << 8));
        }
        
        if (hid_len >= 10) {
            xinput.rx = (int16_t)(hid_data[6] | (hid_data[7] << 8));
            xinput.ry = (int16_t)(hid_data[8] | (hid_data[9] << 8));
        }
    }
    
    // 确保摇杆值在合理范围内
    if (xinput.lx < -32768) xinput.lx = -32768;
    if (xinput.lx > 32767) xinput.lx = 32767;
    if (xinput.ly < -32768) xinput.ly = -32768;
    if (xinput.ly > 32767) xinput.ly = 32767;
    if (xinput.rx < -32768) xinput.rx = -32768;
    if (xinput.rx > 32767) xinput.rx = 32767;
    if (xinput.ry < -32768) xinput.ry = -32768;
    if (xinput.ry > 32767) xinput.ry = 32767;
    
    return xinput;
}