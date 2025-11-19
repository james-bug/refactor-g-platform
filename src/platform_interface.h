/**
 * @file platform_interface.h
 * @brief Gaming Platform Hardware Abstraction Interface
 *
 * 此接口定義了應用層與硬體層之間的契約。
 * 硬體團隊負責實作這些接口，應用團隊只調用這些接口。
 *
 * @version 1.0.2
 * @date 2025-11-18
 */

#ifndef PLATFORM_INTERFACE_H
#define PLATFORM_INTERFACE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * 返回值定義
 * ========================================================================== */

#define PLATFORM_OK             0
#define PLATFORM_ERROR          -1
#define PLATFORM_ERROR_INIT     -2
#define PLATFORM_ERROR_PARAM    -3
#define PLATFORM_ERROR_TIMEOUT  -4
#define PLATFORM_ERROR_NOT_FOUND -5

/* ============================================================================
 * 1. 系統初始化與清理
 * ========================================================================== */

/**
 * @brief 初始化 Platform 硬體層
 *
 * 應用層啟動時首先調用此函數，初始化所有硬體資源。
 *
 * @return PLATFORM_OK 成功，其他值失敗
 *
 * @note 此函數應該是冪等的（可重複調用）
 */
int platform_init(void);

/**
 * @brief 清理 Platform 硬體層
 *
 * 應用層退出時調用，釋放所有硬體資源。
 */
void platform_cleanup(void);

/**
 * @brief 獲取 Platform 實作版本
 *
 * @return 版本字串，例如 "OpenWrt-ADC-v1.0" 或 "Mock-v1.0"
 *
 * @note 用於調試和日誌記錄
 */
const char* platform_get_version(void);

/* ============================================================================
 * 2. 裝置類型檢測
 * ========================================================================== */

/**
 * @brief 檢測裝置類型（Client 或 Server）
 *
 * 應用層調用此函數判斷當前裝置應該啟動 gaming-client 還是 gaming-server。
 *
 * @return "client" 或 "server"，失敗返回 NULL
 *
 * @note 內部實作可使用任何方式：
 *       - 讀取 ADC 值
 *       - 讀取硬體 ID
 *       - 讀取 GPIO 組合
 *       - 讀取配置檔或快取
 *       - 等等...
 *
 * @example
 *   const char *type = platform_get_device_type();
 *   if (strcmp(type, "client") == 0) {
 *       // 啟動 gaming-client
 *   } else if (strcmp(type, "server") == 0) {
 *       // 啟動 gaming-server
 *   }
 */
const char* platform_get_device_type(void);

/* ============================================================================
 * 3. LED 控制
 * ========================================================================== */

/**
 * @brief LED 狀態定義
 *
 * 這些是應用層需要顯示的所有 LED 狀態。
 * 硬體層負責將這些狀態轉換為實際的 LED 顏色/閃爍模式。
 */
typedef enum {
    LED_STATE_OFF = 0,           /**< LED 關閉 */
    LED_STATE_PS5_ON,            /**< PS5 開機（建議：白色） */
    LED_STATE_PS5_STANDBY,       /**< PS5 待機（建議：橙色） */
    LED_STATE_PS5_OFF,           /**< PS5 關機（同 OFF） */
    LED_STATE_VPN_CONNECTING,    /**< VPN 連線中（建議：藍色閃爍） */
    LED_STATE_VPN_CONNECTED,     /**< VPN 已連線（建議：綠色） */
    LED_STATE_VPN_ERROR,         /**< VPN 錯誤（建議：紅色閃爍） */
    LED_STATE_QUERYING,          /**< 查詢 PS5 狀態中（建議：紫色閃爍） */
    LED_STATE_WAKING,            /**< 喚醒 PS5 中（建議：黃色閃爍） */
    LED_STATE_ERROR,             /**< 一般錯誤（建議：紅色） */
    LED_STATE_SYSTEM_ERROR,      /**< 系統錯誤（建議：紅色快閃） */
    LED_STATE_SYSTEM_STARTUP,    /**< 系統啟動中（建議：黃色） */
} platform_led_state_t;

/**
 * @brief 設定 LED 狀態
 *
 * @param state LED 狀態
 * @return PLATFORM_OK 成功，其他值失敗
 *
 * @note 硬體層應根據狀態設定對應的 LED 顏色/閃爍模式
 *       具體的顏色映射由硬體層決定
 *
 * @example
 *   platform_set_led_state(LED_STATE_PS5_ON);  // 顯示白色
 */
int platform_set_led_state(platform_led_state_t state);

/**
 * @brief 自定義 LED 顏色（可選功能）
 *
 * @param r 紅色 (0-255)
 * @param g 綠色 (0-255)
 * @param b 藍色 (0-255)
 * @return PLATFORM_OK 成功，其他值失敗
 *
 * @note 如果硬體不支援自定義顏色，可返回 PLATFORM_ERROR
 */
int platform_set_led_rgb(uint8_t r, uint8_t g, uint8_t b);

/* ============================================================================
 * 4. 按鈕狀態
 * ========================================================================== */

/**
 * @brief 按鈕狀態定義
 */
typedef enum {
    BUTTON_RELEASED = 0,  /**< 按鈕未按下 */
    BUTTON_PRESSED = 1,   /**< 按鈕已按下 */
} platform_button_state_t;

/**
 * @brief 獲取按鈕狀態
 *
 * 應用層（gaming-client）會持續輪詢此函數來檢測按鈕按下。
 *
 * @return BUTTON_PRESSED 或 BUTTON_RELEASED
 *
 * @note 內部實作可使用任何方式：
 *       - 讀取 GPIO
 *       - 讀取 /dev/input/eventX
 *       - 等等...
 *
 * @note Debounce 處理可以在硬體層或應用層實作，
 *       建議在應用層處理以保持接口簡單。
 *
 * @example
 *   // gaming-client/button_handler.c 中的使用
 *   platform_button_state_t current = platform_get_button_state();
 *   if (prev == BUTTON_RELEASED && current == BUTTON_PRESSED) {
 *       // 檢測到按鈕按下，觸發 VPN 連線
 *       vpn_controller_connect();
 *   }
 */
platform_button_state_t platform_get_button_state(void);

/* ============================================================================
 * 5. PS5 電源狀態與控制
 * ========================================================================== */

/**
 * @brief PS5 電源狀態定義
 * 
 * ⭐ 注意: 使用 PLATFORM_PS5_* 前綴以避免與 cec_monitor.h 的 ps5_power_state_t 衝突
 */
typedef enum {
    PLATFORM_PS5_UNKNOWN = 0,  /**< 未知狀態 */
    PLATFORM_PS5_OFF,          /**< 關機 */
    PLATFORM_PS5_STANDBY,      /**< 待機(橙色燈) */
    PLATFORM_PS5_ON,           /**< 開機(白色燈) */
} platform_ps5_power_t;

/**
 * @brief 獲取 PS5 電源狀態
 *
 * 應用層（gaming-server）調用此函數獲取 PS5 當前電源狀態。
 *
 * @return PS5 電源狀態
 *
 * @note 內部實作可使用任何方式：
 *       - HDMI-CEC 查詢
 *       - 網路 ping
 *       - 其他硬體信號
 *       - 快取機制（避免頻繁查詢）
 *
 * @note 建議實作快取機制（例如 1 秒內返回快取值）
 *
 * @example
 *   platform_ps5_power_t power = platform_get_ps5_power();
 *   if (power == PLATFORM_PS5_ON) {
 *       platform_set_led_state(LED_STATE_PS5_ON);
 *   }
 */
platform_ps5_power_t platform_get_ps5_power(void);

/**
 * @brief 喚醒 PS5
 *
 * 應用層（gaming-server）調用此函數喚醒 PS5。
 *
 * @return PLATFORM_OK 成功發送喚醒命令，其他值失敗
 *
 * @note 內部實作可使用任何方式：
 *       - HDMI-CEC wake 命令
 *       - Wake-on-LAN（雖然 PS5 不支援）
 *       - 其他硬體信號
 *
 * @note 此函數只負責發送喚醒命令，不等待 PS5 實際開機
 *       應用層會透過 platform_get_ps5_power() 輪詢確認
 *
 * @example
 *   if (platform_send_ps5_wake() == PLATFORM_OK) {
 *       // 等待 PS5 開機
 *       for (int i = 0; i < 30; i++) {
 *           sleep(1);
 *           if (platform_get_ps5_power() == PLATFORM_PS5_ON) {
 *               break;
 *           }
 *       }
 *   }
 */
int platform_send_ps5_wake(void);

/* ============================================================================
 * 6. 錯誤處理與調試
 * ========================================================================== */

/**
 * @brief 獲取最後錯誤訊息
 *
 * @return 錯誤訊息字串，無錯誤返回 NULL
 *
 * @note 用於調試，應用層可記錄到日誌
 */
const char* platform_get_last_error(void);

/**
 * @brief 重置硬體層（可選功能）
 *
 * 當檢測到錯誤時，應用層可調用此函數嘗試恢復。
 *
 * @return PLATFORM_OK 成功，其他值失敗
 */
int platform_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* PLATFORM_INTERFACE_H */
