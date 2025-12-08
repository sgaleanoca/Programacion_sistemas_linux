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

// HID keyboard input report length
#define HID_KEYBOARD_IN_RPT_LEN     8

// HID LED output report length
#define HID_LED_OUT_RPT_LEN         1

// HID mouse input report length
#define HID_MOUSE_IN_RPT_LEN        5

// HID gamepad input report length
// Format: [buttons(2 bytes), hat_switch+padding(1 byte), x_axis(1 byte), y_axis(1 byte)]
// Byte 0: 8 bits buttons (Button 1-8: A, B, X, Y, L1, R1, L2, R2)
// Byte 1: 8 bits buttons (Button 9-16: unused, unused, SELECT, START, etc.)
// Byte 2: 4 bits hat switch + 4 bits padding
// Byte 3: X axis (-127 to 127)
// Byte 4: Y axis (-127 to 127)
#define HID_GAMEPAD_IN_RPT_LEN      5

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
// (el dispositivo ahora es un gamepad, no un mouse)
void esp_hidd_send_mouse_value(uint16_t conn_id, uint8_t mouse_button, int8_t mickeys_x, int8_t mickeys_y)
{
    // Esta función ya no es funcional ya que el dispositivo es un gamepad
    // Se mantiene solo por compatibilidad de la API
    ESP_LOGW(HID_LE_PRF_TAG, "esp_hidd_send_mouse_value called but device is configured as gamepad");
    (void)conn_id;
    (void)mouse_button;
    (void)mickeys_x;
    (void)mickeys_y;
    return;
}

void esp_hidd_send_gamepad_value(uint16_t conn_id, uint16_t buttons, uint8_t hat_switch, int8_t x_axis, int8_t y_axis)
{
    uint8_t buffer[HID_GAMEPAD_IN_RPT_LEN];

    // Construir el buffer del reporte HID
    // Byte 0: 8 bits de botones (bits 0-7: Button 1-8)
    // Mapeo: Button 1 (bit 0) = A, Button 2 (bit 1) = B
    // Byte 1: 8 bits de botones (bits 8-15: Button 9-16)
    // Mapeo: Button 11 (bit 10) = SELECT, Button 12 (bit 11) = START
    buffer[0] = buttons & 0xFF;         // Primer byte: Button 1-8 (bits 0-7)
    buffer[1] = (buttons >> 8) & 0xFF;   // Segundo byte: Button 9-16 (bits 8-15)
    buffer[2] = (hat_switch & 0x0F);    // Hat Switch / D-Pad (4 bits, 0-7) + 4 bits padding (0)
    buffer[3] = (uint8_t)x_axis;        // X axis (-127 to 127)
    buffer[4] = (uint8_t)y_axis;        // Y axis (-127 to 127)

    // Debug CRÍTICO: Log cuando SELECT o START están presionados
    if (buttons & 0x0C00) { // 0x0C00 = 0b0000110000000000 (bits 10 y 11 = SELECT y START)
        ESP_LOGI(HID_LE_PRF_TAG, "*** ENVIANDO GAMEPAD HID ***");
        ESP_LOGI(HID_LE_PRF_TAG, "Buttons input: 0x%04X, Buffer[0]: 0x%02X, Buffer[1]: 0x%02X", buttons, buffer[0], buffer[1]);
        ESP_LOGI(HID_LE_PRF_TAG, "Bits - A:%d B:%d SEL:%d ST:%d", 
                 (buttons & 0x01) ? 1 : 0,
                 (buttons & 0x02) ? 1 : 0,
                 (buttons & 0x0400) ? 1 : 0,  // Button 11 (bit 10)
                 (buttons & 0x0800) ? 1 : 0); // Button 12 (bit 11)
        ESP_LOGI(HID_LE_PRF_TAG, "Buffer completo: [0x%02X, 0x%02X, 0x%02X, 0x%02X, 0x%02X]", 
                 buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
    }

    // Enviar el reporte HID
    hid_dev_send_report(hidd_le_env.gatt_if, conn_id,
                        HID_RPT_ID_GAMEPAD_IN, HID_REPORT_TYPE_INPUT, HID_GAMEPAD_IN_RPT_LEN, buffer);
    return;
}
