#include "class/hid/hid_host.h"
#include "tusb.h"
#include <string.h>
#include <stdio.h>

// 全局HID主机实例
hid_host_t hid_host = {
    .dev_addr = 0,
    .inst = 0,
    .connected = false,
    .vid = 0,
    .pid = 0,
    .report_len = 0
};

// 缓存最新的原始报告
static uint8_t _last_report[64] = {0};
static uint16_t _last_report_len = 0;
static bool _report_updated = false;

// GameSir T4 Kaleid的VID/PID
#define GAMESIR_VID 0x3537
#define GAMESIR_PID 0x100E

// 检测设备是否为游戏手柄
static bool _is_gamepad_device(uint16_t vid, uint16_t pid) {
    // 接受GameSir T4 Kaleid
    if (vid == GAMESIR_VID && pid == GAMESIR_PID) {
        return true;
    }
    
    // 也接受其他常见游戏手柄（可选）
    if ((vid == 0x045E && (pid == 0x028E || pid == 0x028F || pid == 0x02A1 || pid == 0x02D1)) ||  // Xbox手柄
        (vid == 0x054C && (pid == 0x0268 || pid == 0x09CC || pid == 0x05C4 || pid == 0x0CE6)) ||  // PlayStation手柄
        (vid == 0x057E && pid == 0x2009) ||  // Nintendo Switch Pro
        (vid == 0x0F0D && pid == 0x00C1) ||  // Hori手柄
        (vid == 0x0E6F && (pid == 0x0139 || pid == 0x0151))) {  // Afterglow手柄
        return true;
    }
    
    // 对于未知设备，通过HID接口类来判断
    // HID类（0x03）且接口协议可能为游戏手柄
    printf("Unknown device: VID=0x%04X, PID=0x%04X\n", vid, pid);
    
    return false; // 默认不接受未知设备
}

// 自动检测报告格式
static uint8_t _detect_report_format(const uint8_t* report, uint16_t len) {
    // 根据报告长度判断格式
    if (len >= 19 && len <= 20) {
        // 19-20字节：可能是XInput格式
        printf("Detected XInput format (%u bytes)\n", len);
        return 1; // XInput格式
    } else if (len == 8) {
        // 8字节：可能是DInput格式
        printf("Detected DInput format (%u bytes)\n", len);
        return 2; // DInput格式
    } else if (len >= 6 && len <= 10) {
        // 6-10字节：可能是标准HID游戏手柄格式
        printf("Detected standard HID gamepad format (%u bytes)\n", len);
        return 3; // 标准HID格式
    }
    
    printf("Unknown report format (%u bytes)\n", len);
    return 0; // 未知格式
}

// tinyUSB主机HID匹配回调
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    (void)desc_report;
    (void)desc_len;
    
    printf("HID device mounted: addr=%u, instance=%u\n", dev_addr, instance);
    
    // 获取设备Vid/Pid
    uint16_t vid = 0, pid = 0;
    
    // 使用tuh_descriptor_get_device获取设备描述符
    tusb_desc_device_t dev_desc_buf;
    if (tuh_descriptor_get_device(dev_addr, &dev_desc_buf, sizeof(dev_desc_buf), NULL, 0)) {
        vid = dev_desc_buf.idVendor;
        pid = dev_desc_buf.idProduct;
    }
    
    printf("Device VID: 0x%04X, PID: 0x%04X\n", vid, pid);
    
    // 检查是否为游戏手柄设备
    if (!_is_gamepad_device(vid, pid)) {
        printf("Not a gamepad device, skipping\n");
        return;
    }
    
    printf("Gamepad device detected!\n");
    
    // 记录HID手柄的设备地址和实例
    hid_host.dev_addr = dev_addr;
    hid_host.inst = instance;
    hid_host.connected = true;
    hid_host.vid = vid;
    hid_host.pid = pid;
    _report_updated = false;
    memset(_last_report, 0, sizeof(_last_report));
    _last_report_len = 0;
    
    // 打开设备并开始接收报告
    if (!tuh_hid_receive_report(dev_addr, instance)) {
        printf("Failed to open HID gamepad device\n");
        hid_host.connected = false;
        return;
    }
    
    printf("HID gamepad opened successfully\n");
}

// tinyUSB主机HID断开回调
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    printf("HID device unmounted: addr=%u, instance=%u\n", dev_addr, instance);
    
    if (hid_host.dev_addr == dev_addr && hid_host.inst == instance) {
        printf("Gamepad device disconnected\n");
        hid_host.connected = false;
        hid_host.dev_addr = 0;
        hid_host.inst = 0;
        hid_host.vid = 0;
        hid_host.pid = 0;
        hid_host.report_len = 0;
    }
}

// 接收HID手柄报告的回调
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if (hid_host.dev_addr != dev_addr || hid_host.inst != instance) return;
    
    // 更新缓存的原始报告
    if (len > 0 && len <= sizeof(_last_report)) {
        memcpy(_last_report, report, len);
        _last_report_len = len;
        _report_updated = true;
        
        // 保存报告长度信息
        hid_host.report_len = len;
        if (len <= sizeof(hid_host.raw_report)) {
            memcpy(hid_host.raw_report, report, len);
        }
        
        // 定期打印报告信息（每20个报告打印一次）
        static uint32_t report_count = 0;
        report_count++;
        if (report_count % 20 == 0) {
            printf("HID report #%lu: length=%u, format=", report_count, len);
            _detect_report_format(report, len);
            
            // 打印前几个字节
            printf("Data: ");
            uint8_t print_len = len > 8 ? 8 : len;
            for (uint8_t i = 0; i < print_len; i++) {
                printf("%02X ", report[i]);
            }
            printf("\n");
        }
    }
    
    // 继续接收下一个报告
    tuh_hid_receive_report(dev_addr, instance);
}

// 初始化HID主机模式
void hid_host_init(void) {
    // 确保tinyUSB主机模式已初始化
    if (!tuh_inited()) {
        tuh_init(0); // RP2350 PIO-USB使用端口0
    }
    memset(&hid_host, 0, sizeof(hid_host_t));
    memset(_last_report, 0, sizeof(_last_report));
    _last_report_len = 0;
    _report_updated = false;
}

// 主循环轮询（处理USB主机事件）
void hid_host_task(void) {
    tuh_task(); // 必须调用：处理tinyUSB主机核心逻辑
}

// 读取HID手柄的最新原始报告
bool hid_host_get_raw_report(uint8_t* buffer, uint16_t* len) {
    if (!buffer || !len || !hid_host.connected || !_report_updated) return false;
    
    uint16_t copy_len = _last_report_len > 64 ? 64 : _last_report_len;
    memcpy(buffer, _last_report, copy_len);
    *len = copy_len;
    _report_updated = false; // 重置更新标志
    
    return true;
}