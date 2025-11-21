/*
 * Controller module for BLE HID Gamepad
 * Based on Bluepad32 concepts, optimized for BLE HID
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_hidd_prf_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Gamepad button definitions (HID Gamepad Usage IDs)
#define GAMEPAD_BUTTON_1     0x01  // Button 1 (A/Cross)
#define GAMEPAD_BUTTON_2     0x02  // Button 2 (B/Circle)
#define GAMEPAD_BUTTON_3     0x03  // Button 3 (X/Square)
#define GAMEPAD_BUTTON_4     0x04  // Button 4 (Y/Triangle)
#define GAMEPAD_BUTTON_5     0x05  // Left Shoulder
#define GAMEPAD_BUTTON_6     0x06  // Right Shoulder
#define GAMEPAD_BUTTON_7     0x07  // Select/Back
#define GAMEPAD_BUTTON_8     0x08  // Start
#define GAMEPAD_BUTTON_9     0x09  // Left Stick Press
#define GAMEPAD_BUTTON_10    0x0A  // Right Stick Press
#define GAMEPAD_BUTTON_11    0x0B  // D-Pad Up
#define GAMEPAD_BUTTON_12    0x0C  // D-Pad Down
#define GAMEPAD_BUTTON_13    0x0D  // D-Pad Left
#define GAMEPAD_BUTTON_14    0x0E  // D-Pad Right

// Joystick range: -127 to 127 (signed 8-bit)
#define JOYSTICK_MIN     -127
#define JOYSTICK_MAX      127
#define JOYSTICK_CENTER  0

// Gamepad state structure
typedef struct {
    // Buttons bitmask (16 buttons supported)
    uint16_t buttons;
    
    // Left joystick
    int8_t left_x;
    int8_t left_y;
    
    // Right joystick
    int8_t right_x;
    int8_t right_y;
    
    // Triggers (0-255)
    uint8_t left_trigger;
    uint8_t right_trigger;
    
    // D-Pad (0-8, where 0 = center, 1-8 = directions)
    uint8_t dpad;
} gamepad_state_t;

/**
 * @brief Initialize the controller module
 * 
 * This function initializes GPIO pins for buttons and ADC for joysticks.
 * Configure your hardware pins in this function.
 */
void controller_init(void);

/**
 * @brief Set the HID connection ID
 * 
 * @param conn_id BLE connection ID
 */
void controller_set_hid_connection(uint16_t conn_id);

/**
 * @brief Clear the HID connection ID
 */
void controller_clear_hid_connection(void);

/**
 * @brief Check if HID is connected
 * 
 * @return true if connected, false otherwise
 */
bool controller_is_connected(void);

/**
 * @brief Update gamepad state from hardware
 * 
 * This function reads buttons and joysticks from hardware
 * and updates the internal gamepad state.
 * 
 * @param state Pointer to gamepad state structure to update
 */
void controller_update_state(gamepad_state_t *state);

/**
 * @brief Send gamepad report via BLE HID
 * 
 * @param state Pointer to gamepad state structure
 */
void controller_send_gamepad_report(const gamepad_state_t *state);

/**
 * @brief Set a button state
 * 
 * @param button Button ID (GAMEPAD_BUTTON_1 to GAMEPAD_BUTTON_14)
 * @param pressed true if pressed, false if released
 */
void controller_set_button(uint8_t button, bool pressed);

/**
 * @brief Set left joystick position
 * 
 * @param x X position (-127 to 127)
 * @param y Y position (-127 to 127)
 */
void controller_set_left_stick(int8_t x, int8_t y);

/**
 * @brief Set right joystick position
 * 
 * @param x X position (-127 to 127)
 * @param y Y position (-127 to 127)
 */
void controller_set_right_stick(int8_t x, int8_t y);

/**
 * @brief Set trigger values
 * 
 * @param left Left trigger (0-255)
 * @param right Right trigger (0-255)
 */
void controller_set_triggers(uint8_t left, uint8_t right);

/**
 * @brief Set D-Pad direction
 * 
 * @param direction 0 = center, 1-8 = directions (1=up, 2=up-right, etc.)
 */
void controller_set_dpad(uint8_t direction);

/**
 * @brief Get current gamepad state
 * 
 * @return Pointer to current gamepad state
 */
const gamepad_state_t* controller_get_state(void);

#ifdef __cplusplus
}
#endif

#endif /* CONTROLLER_H */

