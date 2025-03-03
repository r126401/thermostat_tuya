/**
 * @file tkl_sleep.c
 * @brief this file was auto-generated by tuyaos v&v tools, developer can add implements between BEGIN and END
 * 
 * @warning: changes between user 'BEGIN' and 'END' will be keeped when run tuyaos v&v tools
 *           changes in other place will be overwrited and lost
 *
 * @copyright Copyright 2020-2021 Tuya Inc. All Rights Reserved.
 * 
 */

// --- BEGIN: user defines and implements ---
#include "freertos/FreeRTOS.h"

#include "esp_err.h"
#include "esp_log.h"

#include "tkl_sleep.h"
// --- END: user defines and implements ---

/**
 * @brief sleep callback register
 * 
 * @param[in] sleep_cb:  sleep callback
 *
 * @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
 */
OPERATE_RET tkl_cpu_sleep_callback_register(TUYA_SLEEP_CB_T *sleep_cb)
{
    // --- BEGIN: user implements ---
    return OPRT_NOT_SUPPORTED;
    // --- END: user implements ---
}

/**
 * @brief allow to sleep
 * 
 * @param[in] none
 *
 * @return none
 */
void tkl_cpu_allow_sleep(void)
{
    // --- BEGIN: user implements ---
    return ;
    // --- END: user implements ---
}

/**
 * @brief force wakeup
 * 
 * @param[in] none
 *
 * @return none
 */
void tkl_cpu_force_wakeup(void)
{
    // --- BEGIN: user implements ---
    return ;
    // --- END: user implements ---
}

/**
* @brief Set the low power mode of CPU
*
* @param[in] enable: enable switch
* @param[in] mode:   cpu sleep mode
*
* @note This API is used for setting the low power mode of CPU.
*
* @return OPRT_OK on success. Others on error, please refer to tuya_error_code.h
*/
OPERATE_RET tkl_cpu_sleep_mode_set(BOOL_T enable, TUYA_CPU_SLEEP_MODE_E mode)
{
    // --- BEGIN: user implements ---
#if CONFIG_PM_ENABLE    
#define MHZ (1000000)    
    int cur_freq_mhz = esp_clk_cpu_freq() / MHZ;
    int xtal_freq = esp_clk_xtal_freq() / MHZ;
#if CONFIG_IDF_TARGET_ESP32
    esp_pm_config_esp32_t pm_config;
#elif CONFIG_IDF_TARGET_ESP32S2
    esp_pm_config_esp32s2_t pm_config;
#elif CONFIG_IDF_TARGET_ESP32C3
    esp_pm_config_esp32c3_t pm_config;
#elif CONFIG_IDF_TARGET_ESP32S3
    esp_pm_config_esp32s3_t pm_config;
#elif CONFIG_IDF_TARGET_ESP32C2
    esp_pm_config_esp32c2_t pm_config;
#endif
#endif // CONFIG_PM_ENABLE

#if CONFIG_PM_ENABLE
    //ESP_LOGI(DBG_TAG, "%s: enable %d mode %d cur_freq_mhz %d xtal_freq %d",
    //    __func__, enable, mode, cur_freq_mhz, xtal_freq);
#endif

#if CONFIG_PM_ENABLE
    if (enable) {
        pm_config.max_freq_mhz = cur_freq_mhz;
        pm_config.min_freq_mhz = xtal_freq;
    } else {
        pm_config.max_freq_mhz = pm_config.min_freq_mhz = cur_freq_mhz;
    }
#endif

    switch (mode) {
    case TUYA_CPU_SLEEP:
#if CONFIG_PM_ENABLE && CONFIG_FREERTOS_USE_TICKLESS_IDLE    
        //pm_config.light_sleep_enable = enable ? true : false;
        pm_config.light_sleep_enable = false;
#endif
#if CONFIG_PM_ENABLE
        if (ESP_OK != esp_pm_configure(&pm_config)) {
            //ESP_LOGE(DBG_TAG, "%s: call esp_pm_configure failed", __func__);
            return OPRT_COM_ERROR;
        }
#endif //CONFIG_PM_ENABLE
        break;
    case TUYA_CPU_DEEP_SLEEP:
        break;
    default:
        break;        
    }

	return OPRT_OK;
    // --- END: user implements ---
}

