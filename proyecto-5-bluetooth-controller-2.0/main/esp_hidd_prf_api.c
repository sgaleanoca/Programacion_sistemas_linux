/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include "esp_hidd_prf_api.h"
#include "hidd_le_prf_int.h"
#include "hid_dev.h"
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"

// Include HID report ID definitions
#ifndef HID_RPT_ID_GAMEPAD_IN
#define HID_RPT_ID_GAMEPAD_IN    5
#endif

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN     8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN         1

// HID mouse input report length
#define HID_MOUSE_IN_RPT_LEN        5


// HID consumer control input report length
#define HID_CC_IN_RPT_LEN           2

esp_err_t esp_hidd_register_callbacks(esp_hidd_event_cb_t callbacks)
{
    esp_err_t hidd_status;

    if(callbacks != NULL) {
   	    hidd_le_env.hidd_cb = callbacks;
    } else {
        return ESP_FAIL;
    }

    if((hidd_status = hidd_register_cb()) != ESP_OK) {
        return hidd_status;
    }

    esp_ble_gatts_app_register(BATTRAY_APP_ID);

    if((hidd_status = esp_ble_gatts_app_register(HIDD_APP_ID)) != ESP_OK) {
        return hidd_status;
    }

    return hidd_status;
}

esp_err_t esp_hidd_profile_init(void)
{
     if (hidd_le_env.enabled) {
        ESP_LOGE(HID_LE_PRF_TAG, "HID device profile already initialized");
        return ESP_FAIL;
    }
    // Reset the hid device target environment
    memset(&hidd_le_env, 0, sizeof(hidd_le_env_t));
    hidd_le_env.enabled = true;
    return ESP_OK;
}

esp_err_t esp_hidd_profile_deinit(void)
{
    uint16_t hidd_svc_hdl = hidd_le_env.hidd_inst.att_tbl[HIDD_LE_IDX_SVC];
    if (!hidd_le_env.enabled) {
        ESP_LOGE(HID_LE_PRF_TAG, "HID device profile already initialized");
        return ESP_OK;
    }

    if(hidd_svc_hdl != 0) {
	esp_ble_gatts_stop_service(hidd_svc_hdl);
	esp_ble_gatts_delete_service(hidd_svc_hdl);
    } else {
	return ESP_FAIL;
   }

    /* register the HID device profile to the BTA_GATTS module*/
    esp_ble_gatts_app_unregister(hidd_le_env.gatt_if);

    return ESP_OK;
}

uint16_t esp_hidd_get_version(void)
{
	return HIDD_VERSION;
}

void esp_hidd_send_consumer_value(uint16_t conn_id, uint8_t key_cmd, bool key_pressed)
{
    uint8_t buffer[HID_CC_IN_RPT_LEN] = {0, 0};
    if (key_pressed) {
        ESP_LOGD(HID_LE_PRF_TAG, "hid_consumer_build_report");
        hid_consumer_build_report(buffer, key_cmd);
    }
    ESP_LOGD(HID_LE_PRF_TAG, "buffer[0] = %x, buffer[1] = %x", buffer[0], buffer[1]);
    hid_dev_send_report(hidd_le_env.gatt_if, conn_id,
                        HID_RPT_ID_CC_IN, HID_REPORT_TYPE_INPUT, HID_CC_IN_RPT_LEN, buffer);
    return;
}

void esp_hidd_send_keyboard_value(uint16_t conn_id, key_mask_t special_key_mask, uint8_t *keyboard_cmd, uint8_t num_key)
{
    if (num_key > HID_KEYBOARD_IN_RPT_LEN - 2) {
        ESP_LOGE(HID_LE_PRF_TAG, "%s(), the number key should not be more than %d", __func__, HID_KEYBOARD_IN_RPT_LEN);
        return;
    }

    uint8_t buffer[HID_KEYBOARD_IN_RPT_LEN] = {0};

    buffer[0] = special_key_mask;

    for (int i = 0; i < num_key; i++) {
        buffer[i+2] = keyboard_cmd[i];
    }

    ESP_LOGD(HID_LE_PRF_TAG, "the key vaule = %d,%d,%d, %d, %d, %d,%d, %d", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7]);
    hid_dev_send_report(hidd_le_env.gatt_if, conn_id,
                        HID_RPT_ID_KEY_IN, HID_REPORT_TYPE_INPUT, HID_KEYBOARD_IN_RPT_LEN, buffer);
    return;
}

// Función mantenida por compatibilidad, pero no se usa en este proyecto
void esp_hidd_send_mouse_value(uint16_t conn_id, uint8_t mouse_button, int8_t mickeys_x, int8_t mickeys_y)
{
    // Esta función se mantiene solo por compatibilidad de la API
    ESP_LOGW(HID_LE_PRF_TAG, "esp_hidd_send_mouse_value called but not implemented in this project");
    (void)conn_id;
    (void)mouse_button;
    (void)mickeys_x;
    (void)mickeys_y;
    return;
}

// Gamepad report length: buttons(2) + left_x(1) + left_y(1) + right_x(1) + right_y(1) + left_trigger(1) + right_trigger(1) = 8 bytes
#define HID_GAMEPAD_IN_RPT_LEN       8

void esp_hidd_send_gamepad_value(uint16_t conn_id, uint16_t buttons, int8_t left_x, int8_t left_y, int8_t right_x, int8_t right_y, uint8_t left_trigger, uint8_t right_trigger)
{
    uint8_t buffer[HID_GAMEPAD_IN_RPT_LEN] = {0};
    
    // Buttons (16 buttons, 2 bytes)
    buffer[0] = (uint8_t)(buttons & 0xFF);
    buffer[1] = (uint8_t)((buttons >> 8) & 0xFF);
    
    // Left joystick
    buffer[2] = (uint8_t)left_x;
    buffer[3] = (uint8_t)left_y;
    
    // Right joystick
    buffer[4] = (uint8_t)right_x;
    buffer[5] = (uint8_t)right_y;
    
    // Triggers
    buffer[6] = left_trigger;
    buffer[7] = right_trigger;
    
    ESP_LOGD(HID_LE_PRF_TAG, "Sending gamepad report: buttons=0x%04X, L(%d,%d) R(%d,%d) T(%d,%d)",
             buttons, left_x, left_y, right_x, right_y, left_trigger, right_trigger);
    
    hid_dev_send_report(hidd_le_env.gatt_if, conn_id,
                        HID_RPT_ID_GAMEPAD_IN, HID_REPORT_TYPE_INPUT, HID_GAMEPAD_IN_RPT_LEN, buffer);
    return;
}

