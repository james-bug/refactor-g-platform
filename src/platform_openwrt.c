
/**
 * @file platform_openwrt.c
 * @brief OpenWrt One implementation (hardware team fills this)
 */

#include "platform_interface.h"
#include <stdio.h>
#include <string.h>

// TODO: 硬體團隊實作所有接口
// 內部實作方式完全自由：
// - 可使用 ADC
// - 可使用 HW ID
// - 可使用 GPIO
// - 可使用任何其他方式

int platform_init(void) {
    // TODO: 硬體團隊實作
    printf("[Platform OpenWrt] TODO: Implement platform_init()\n");
    return PLATFORM_OK;
}

void platform_cleanup(void) {
    // TODO: 硬體團隊實作
}

const char* platform_get_version(void) {
    return "OpenWrt-TODO-v1.0";
}

const char* platform_get_device_type(void) {
    // TODO: 硬體團隊實作
    // 可選方案:
    // 1. 讀取 ADC
    // 2. 讀取 HW ID
    // 3. 讀取 GPIO 組合
    // 4. 讀取配置檔
    return "client";  // 臨時返回
}

int platform_set_led_state(platform_led_state_t state) {
    // TODO: 硬體團隊實作
    return PLATFORM_OK;
}

int platform_set_led_rgb(uint8_t r, uint8_t g, uint8_t b) {
    // TODO: 硬體團隊實作
    return PLATFORM_OK;
}

platform_button_state_t platform_get_button_state(void) {
    // TODO: 硬體團隊實作
    return BUTTON_RELEASED;
}

platform_ps5_power_t platform_get_ps5_power(void) {
    // TODO: 硬體團隊實作
    return PLATFORM_PS5_UNKNOWN;
}

int platform_send_ps5_wake(void) {
    // TODO: 硬體團隊實作
    return PLATFORM_OK;
}

const char* platform_get_last_error(void) {
    return NULL;
}

int platform_reset(void) {
    // TODO: 硬體團隊實作
    return PLATFORM_OK;
}
