/**
 * @file platform_mock.c
 * @brief Mock Implementation for Gaming Platform (For Testing)
 * 
 * 提供硬體平台的模擬實作，用於無硬體環境的開發和測試。
 * 支援環境變數配置，可模擬不同的設備類型和狀態。
 * 
 * 環境變數:
 * - MOCK_DEVICE_TYPE: "client" 或 "server" (預設: "client")
 * - MOCK_BUTTON_STATE: "0" (released) 或 "1" (pressed)
 * - MOCK_PS5_POWER: "off", "standby", "on"
 * 
 * @version 1.0.0
 * @date 2024-11-17
 */

#include "platform_interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

/* ============================================================================
 * Mock State Management
 * ========================================================================== */

/**
 * @brief Mock 平台內部狀態
 */
static struct {
    bool initialized;
    
    // 設備資訊
    char device_type[16];
    char version[32];
    
    // LED 狀態
    platform_led_state_t led_state;
    uint8_t led_rgb[3];  // R, G, B
    
    // Button 狀態
    platform_button_state_t button_state;
    
    // PS5 狀態
    platform_ps5_power_t ps5_power;
    
    // 錯誤訊息
    char last_error[256];
    
    // 統計資訊
    struct {
        int init_count;
        int led_set_count;
        int button_read_count;
        int ps5_query_count;
        int ps5_wake_count;
    } stats;
    
} g_mock_platform = {
    .initialized = false,
    .device_type = "client",
    .version = "Mock-v1.0.0",
    .led_state = LED_STATE_OFF,
    .led_rgb = {0, 0, 0},
    .button_state = PLATFORM_BUTTON_RELEASED,
    .ps5_power = PLATFORM_PS5_OFF,
    .last_error = {0},
    .stats = {0}
};

/* ============================================================================
 * Internal Helper Functions
 * ========================================================================== */

/**
 * @brief 從環境變數讀取設備類型
 */
static void load_device_type_from_env(void) {
    const char *env_type = getenv("MOCK_DEVICE_TYPE");
    if (env_type) {
        if (strcmp(env_type, "client") == 0 || strcmp(env_type, "server") == 0) {
            strncpy(g_mock_platform.device_type, env_type, 
                   sizeof(g_mock_platform.device_type) - 1);
            printf("[Platform Mock] Device type set to '%s' from environment\n", env_type);
        } else {
            fprintf(stderr, "[Platform Mock] Invalid MOCK_DEVICE_TYPE: %s (using default: client)\n", 
                   env_type);
        }
    }
}

/**
 * @brief 從環境變數讀取按鈕狀態
 */
static platform_button_state_t get_button_state_from_env(void) {
    const char *env_button = getenv("MOCK_BUTTON_STATE");
    if (env_button) {
        if (strcmp(env_button, "1") == 0 || strcmp(env_button, "pressed") == 0) {
            return PLATFORM_BUTTON_PRESSED;
        }
    }
    return g_mock_platform.button_state;
}

/**
 * @brief 從環境變數讀取PS5狀態
 */
static platform_ps5_power_t get_ps5_power_from_env(void) {
    const char *env_ps5 = getenv("MOCK_PS5_POWER");
    if (env_ps5) {
        if (strcmp(env_ps5, "on") == 0) {
            return PLATFORM_PS5_ON;
        } else if (strcmp(env_ps5, "standby") == 0) {
            return PLATFORM_PS5_STANDBY;
        } else if (strcmp(env_ps5, "off") == 0) {
            return PLATFORM_PS5_OFF;
        }
    }
    return g_mock_platform.ps5_power;
}

/**
 * @brief LED 狀態轉換為 RGB 值
 */
static void led_state_to_rgb(platform_led_state_t state, uint8_t *r, uint8_t *g, uint8_t *b) {
    switch (state) {
        case LED_STATE_OFF:
            *r = 0; *g = 0; *b = 0;
            break;
        case LED_STATE_VPN_CONNECTING:
            *r = 0; *g = 0; *b = 255;  // 藍色閃爍
            break;
        case LED_STATE_VPN_CONNECTED:
            *r = 0; *g = 255; *b = 0;  // 綠色
            break;
        case LED_STATE_QUERYING:
            *r = 255; *g = 255; *b = 0;  // 黃色
            break;
        case LED_STATE_PS5_OFF:
            *r = 255; *g = 0; *b = 0;  // 紅色
            break;
        case LED_STATE_PS5_ON:
            *r = 0; *g = 255; *b = 0;  // 綠色
            break;
        case LED_STATE_WAKING:
            *r = 128; *g = 0; *b = 255;  // 紫色閃爍
            break;
        case LED_STATE_ERROR:
            *r = 255; *g = 0; *b = 0;  // 紅色閃爍
            break;
        default:
            *r = 0; *g = 0; *b = 0;
            break;
    }
}

/**
 * @brief 設定錯誤訊息
 */
static void set_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(g_mock_platform.last_error, sizeof(g_mock_platform.last_error), format, args);
    va_end(args);
}

/* ============================================================================
 * Public API Implementation
 * ========================================================================== */

/**
 * @brief 初始化 Mock Platform
 * @return PLATFORM_OK 成功, 其他值失敗
 */
platform_result_t platform_init(void) {
    if (g_mock_platform.initialized) {
        printf("[Platform Mock] Already initialized\n");
        return PLATFORM_OK;
    }
    
    // 從環境變數載入配置
    load_device_type_from_env();
    
    // 初始化狀態
    g_mock_platform.led_state = LED_STATE_OFF;
    g_mock_platform.button_state = PLATFORM_BUTTON_RELEASED;
    g_mock_platform.ps5_power = PLATFORM_PS5_OFF;
    memset(&g_mock_platform.stats, 0, sizeof(g_mock_platform.stats));
    memset(g_mock_platform.last_error, 0, sizeof(g_mock_platform.last_error));
    
    g_mock_platform.initialized = true;
    g_mock_platform.stats.init_count++;
    
    printf("[Platform Mock] Initialized successfully\n");
    printf("  Device Type: %s\n", g_mock_platform.device_type);
    printf("  Version: %s\n", g_mock_platform.version);
    printf("  Init Count: %d\n", g_mock_platform.stats.init_count);
    
    return PLATFORM_OK;
}

/**
 * @brief 清理 Mock Platform
 */
void platform_cleanup(void) {
    if (!g_mock_platform.initialized) {
        return;
    }
    
    printf("[Platform Mock] Cleanup - Statistics:\n");
    printf("  Init Count: %d\n", g_mock_platform.stats.init_count);
    printf("  LED Set Count: %d\n", g_mock_platform.stats.led_set_count);
    printf("  Button Read Count: %d\n", g_mock_platform.stats.button_read_count);
    printf("  PS5 Query Count: %d\n", g_mock_platform.stats.ps5_query_count);
    printf("  PS5 Wake Count: %d\n", g_mock_platform.stats.ps5_wake_count);
    
    g_mock_platform.initialized = false;
    printf("[Platform Mock] Cleaned up\n");
}

/**
 * @brief 取得 Platform 版本
 * @return 版本字串
 */
const char* platform_get_version(void) {
    return g_mock_platform.version;
}

/**
 * @brief 取得設備類型
 * @return "client" 或 "server"
 */
const char* platform_get_device_type(void) {
    if (!g_mock_platform.initialized) {
        // 如果未初始化,先初始化
        platform_init();
    }
    return g_mock_platform.device_type;
}

/**
 * @brief 設定 LED 狀態
 * @param state LED 狀態
 * @return PLATFORM_OK 成功, 其他值失敗
 */
platform_result_t platform_set_led_state(platform_led_state_t state) {
    if (!g_mock_platform.initialized) {
        set_error("Platform not initialized");
        return PLATFORM_ERROR_NOT_INITIALIZED;
    }
    
    if (state < LED_STATE_OFF || state > LED_STATE_ERROR) {
        set_error("Invalid LED state: %d", state);
        return PLATFORM_ERROR_INVALID_PARAM;
    }
    
    g_mock_platform.led_state = state;
    g_mock_platform.stats.led_set_count++;
    
    // 轉換為 RGB
    led_state_to_rgb(state, &g_mock_platform.led_rgb[0], 
                            &g_mock_platform.led_rgb[1],
                            &g_mock_platform.led_rgb[2]);
    
    printf("[Platform Mock] LED state set to %d (RGB: %d,%d,%d)\n",
           state, g_mock_platform.led_rgb[0], 
           g_mock_platform.led_rgb[1], 
           g_mock_platform.led_rgb[2]);
    
    return PLATFORM_OK;
}

/**
 * @brief 設定 LED RGB 顏色 (可選)
 * @param r 紅色 (0-255)
 * @param g 綠色 (0-255)
 * @param b 藍色 (0-255)
 * @return PLATFORM_OK 成功, 其他值失敗
 */
platform_result_t platform_set_led_rgb(uint8_t r, uint8_t g, uint8_t b) {
    if (!g_mock_platform.initialized) {
        set_error("Platform not initialized");
        return PLATFORM_ERROR_NOT_INITIALIZED;
    }
    
    g_mock_platform.led_rgb[0] = r;
    g_mock_platform.led_rgb[1] = g;
    g_mock_platform.led_rgb[2] = b;
    g_mock_platform.stats.led_set_count++;
    
    printf("[Platform Mock] LED RGB set to (%d, %d, %d)\n", r, g, b);
    
    return PLATFORM_OK;
}

/**
 * @brief 取得按鈕狀態
 * @return 按鈕狀態
 */
platform_button_state_t platform_get_button_state(void) {
    if (!g_mock_platform.initialized) {
        platform_init();
    }
    
    g_mock_platform.stats.button_read_count++;
    
    // 優先從環境變數讀取 (用於測試)
    platform_button_state_t state = get_button_state_from_env();
    
    printf("[Platform Mock] Button state queried: %s (count: %d)\n",
           state == PLATFORM_BUTTON_PRESSED ? "PRESSED" : "RELEASED",
           g_mock_platform.stats.button_read_count);
    
    return state;
}

/**
 * @brief 取得 PS5 電源狀態
 * @return PS5 電源狀態
 */
platform_ps5_power_t platform_get_ps5_power(void) {
    if (!g_mock_platform.initialized) {
        platform_init();
    }
    
    g_mock_platform.stats.ps5_query_count++;
    
    // 優先從環境變數讀取 (用於測試)
    platform_ps5_power_t power = get_ps5_power_from_env();
    
    const char *power_str;
    switch (power) {
        case PLATFORM_PS5_OFF: power_str = "OFF"; break;
        case PLATFORM_PS5_STANDBY: power_str = "STANDBY"; break;
        case PLATFORM_PS5_ON: power_str = "ON"; break;
        default: power_str = "UNKNOWN"; break;
    }
    
    printf("[Platform Mock] PS5 power queried: %s (count: %d)\n",
           power_str, g_mock_platform.stats.ps5_query_count);
    
    return power;
}

/**
 * @brief 發送 PS5 喚醒命令
 * @return PLATFORM_OK 成功, 其他值失敗
 */
platform_result_t platform_send_ps5_wake(void) {
    if (!g_mock_platform.initialized) {
        set_error("Platform not initialized");
        return PLATFORM_ERROR_NOT_INITIALIZED;
    }
    
    g_mock_platform.stats.ps5_wake_count++;
    
    // 模擬喚醒: 將 PS5 狀態設為 ON
    g_mock_platform.ps5_power = PLATFORM_PS5_ON;
    
    printf("[Platform Mock] PS5 wake command sent (count: %d)\n",
           g_mock_platform.stats.ps5_wake_count);
    printf("[Platform Mock] PS5 power state changed to ON\n");
    
    return PLATFORM_OK;
}

/**
 * @brief 取得最後錯誤訊息
 * @return 錯誤訊息字串, 無錯誤返回 NULL
 */
const char* platform_get_last_error(void) {
    if (g_mock_platform.last_error[0] == '\0') {
        return NULL;
    }
    return g_mock_platform.last_error;
}

/**
 * @brief 重置硬體層 (可選功能)
 * @return PLATFORM_OK 成功, 其他值失敗
 */
platform_result_t platform_reset(void) {
    if (!g_mock_platform.initialized) {
        set_error("Platform not initialized");
        return PLATFORM_ERROR_NOT_INITIALIZED;
    }
    
    printf("[Platform Mock] Resetting platform...\n");
    
    // 重置狀態
    g_mock_platform.led_state = LED_STATE_OFF;
    g_mock_platform.button_state = PLATFORM_BUTTON_RELEASED;
    memset(g_mock_platform.led_rgb, 0, sizeof(g_mock_platform.led_rgb));
    memset(g_mock_platform.last_error, 0, sizeof(g_mock_platform.last_error));
    
    printf("[Platform Mock] Reset complete\n");
    
    return PLATFORM_OK;
}

/* ============================================================================
 * Mock Control Functions (僅供測試使用)
 * ========================================================================== */

#ifdef TESTING

/**
 * @brief 設定 Mock 設備類型 (測試用)
 * @param type "client" 或 "server"
 */
void mock_platform_set_device_type(const char *type) {
    if (type && (strcmp(type, "client") == 0 || strcmp(type, "server") == 0)) {
        strncpy(g_mock_platform.device_type, type, sizeof(g_mock_platform.device_type) - 1);
        printf("[Platform Mock] Device type manually set to: %s\n", type);
    }
}

/**
 * @brief 設定 Mock 按鈕狀態 (測試用)
 * @param state 按鈕狀態
 */
void mock_platform_set_button_state(platform_button_state_t state) {
    g_mock_platform.button_state = state;
    printf("[Platform Mock] Button state manually set to: %s\n",
           state == PLATFORM_BUTTON_PRESSED ? "PRESSED" : "RELEASED");
}

/**
 * @brief 設定 Mock PS5 電源狀態 (測試用)
 * @param power PS5 電源狀態
 */
void mock_platform_set_ps5_power(platform_ps5_power_t power) {
    g_mock_platform.ps5_power = power;
    const char *power_str;
    switch (power) {
        case PLATFORM_PS5_OFF: power_str = "OFF"; break;
        case PLATFORM_PS5_STANDBY: power_str = "STANDBY"; break;
        case PLATFORM_PS5_ON: power_str = "ON"; break;
        default: power_str = "UNKNOWN"; break;
    }
    printf("[Platform Mock] PS5 power manually set to: %s\n", power_str);
}

/**
 * @brief 取得 Mock 統計資訊 (測試用)
 * @param init_count 初始化次數
 * @param led_count LED 設定次數
 * @param button_count 按鈕讀取次數
 * @param ps5_query_count PS5 查詢次數
 * @param ps5_wake_count PS5 喚醒次數
 */
void mock_platform_get_stats(int *init_count, int *led_count, int *button_count,
                             int *ps5_query_count, int *ps5_wake_count)
{
    if (init_count) *init_count = g_mock_platform.stats.init_count;
    if (led_count) *led_count = g_mock_platform.stats.led_set_count;
    if (button_count) *button_count = g_mock_platform.stats.button_read_count;
    if (ps5_query_count) *ps5_query_count = g_mock_platform.stats.ps5_query_count;
    if (ps5_wake_count) *ps5_wake_count = g_mock_platform.stats.ps5_wake_count;
}

/**
 * @brief 重置 Mock 統計資訊 (測試用)
 */
void mock_platform_reset_stats(void) {
    memset(&g_mock_platform.stats, 0, sizeof(g_mock_platform.stats));
    printf("[Platform Mock] Statistics reset\n");
}

#endif // TESTING
