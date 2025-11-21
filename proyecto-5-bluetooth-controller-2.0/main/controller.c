/*
 * Controller module for BLE HID Gamepad
 * Based on Bluepad32 concepts, optimized for BLE HID
 */

#include "controller.h"
#include "esp_hidd_prf_api.h"
#include "hidd_le_prf_int.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "CONTROLLER";

// HID Gamepad Report ID (will be defined in hidd_le_prf_int.h)
#define HID_RPT_ID_GAMEPAD_IN    5

// Gamepad report length: buttons(2) + left_x(1) + left_y(1) + right_x(1) + right_y(1) + left_trigger(1) + right_trigger(1) = 8 bytes
#define GAMEPAD_REPORT_LEN       8

// Current connection state
static uint16_t hid_conn_id = 0;
static bool is_connected = false;

// Current gamepad state
static gamepad_state_t current_state = {0};

// GPIO pin definitions - MODIFY THESE ACCORDING TO YOUR HARDWARE
#define BUTTON_1_GPIO     0   // Modify with your GPIO pin
#define BUTTON_2_GPIO     2   // Modify with your GPIO pin
#define BUTTON_3_GPIO     4   // Modify with your GPIO pin
#define BUTTON_4_GPIO     5   // Modify with your GPIO pin
// Add more button GPIOs as needed

// ADC channel definitions for joysticks - MODIFY THESE ACCORDING TO YOUR HARDWARE
// Note: You'll need to configure ADC in your implementation
#define JOYSTICK_LEFT_X_ADC_CH   0
#define JOYSTICK_LEFT_Y_ADC_CH   1
#define JOYSTICK_RIGHT_X_ADC_CH  2
#define JOYSTICK_RIGHT_Y_ADC_CH  3

void controller_init(void)
{
    ESP_LOGI(TAG, "Initializing controller module");
    
    // Initialize gamepad state
    memset(&current_state, 0, sizeof(gamepad_state_t));
    
    // TODO: Initialize GPIO pins for buttons
    // Example:
    // gpio_set_direction(BUTTON_1_GPIO, GPIO_MODE_INPUT);
    // gpio_set_pull_mode(BUTTON_1_GPIO, GPIO_PULLUP_ONLY);
    
    // TODO: Initialize ADC for joysticks
    // Example ADC initialization code would go here
    
    ESP_LOGI(TAG, "Controller module initialized");
}

void controller_set_hid_connection(uint16_t conn_id)
{
    hid_conn_id = conn_id;
    is_connected = true;
    ESP_LOGI(TAG, "HID connection set: conn_id=%d", conn_id);
}

void controller_clear_hid_connection(void)
{
    is_connected = false;
    hid_conn_id = 0;
    ESP_LOGI(TAG, "HID connection cleared");
}

bool controller_is_connected(void)
{
    return is_connected;
}

void controller_update_state(gamepad_state_t *state)
{
    if (state == NULL) {
        return;
    }
    
    // TODO: Read hardware buttons and update state->buttons
    // Example:
    // state->buttons = 0;
    // if (gpio_get_level(BUTTON_1_GPIO) == 0) {
    //     state->buttons |= (1 << (GAMEPAD_BUTTON_1 - 1));
    // }
    
    // TODO: Read ADC for joysticks and update state->left_x, left_y, right_x, right_y
    // Example:
    // int adc_value = adc1_get_raw(JOYSTICK_LEFT_X_ADC_CH);
    // state->left_x = (int8_t)((adc_value - 2048) / 16); // Scale to -127 to 127
    
    // TODO: Read triggers if you have them
    // state->left_trigger = ...;
    // state->right_trigger = ...;
    
    // Update current state
    memcpy(&current_state, state, sizeof(gamepad_state_t));
}

void controller_send_gamepad_report(const gamepad_state_t *state)
{
    if (!is_connected || state == NULL) {
        return;
    }
    
    // Build gamepad HID report
    // Format: [buttons_low, buttons_high, left_x, left_y, right_x, right_y, left_trigger, right_trigger]
    uint8_t report[GAMEPAD_REPORT_LEN] = {0};
    
    // Buttons (16 buttons, 2 bytes)
    report[0] = (uint8_t)(state->buttons & 0xFF);
    report[1] = (uint8_t)((state->buttons >> 8) & 0xFF);
    
    // Left joystick
    report[2] = (uint8_t)state->left_x;
    report[3] = (uint8_t)state->left_y;
    
    // Right joystick
    report[4] = (uint8_t)state->right_x;
    report[5] = (uint8_t)state->right_y;
    
    // Triggers
    report[6] = state->left_trigger;
    report[7] = state->right_trigger;
    
    // Send via HID using the gamepad API function
    esp_hidd_send_gamepad_value(hid_conn_id, state->buttons, 
                                 state->left_x, state->left_y,
                                 state->right_x, state->right_y,
                                 state->left_trigger, state->right_trigger);
}

void controller_set_button(uint8_t button, bool pressed)
{
    if (button < 1 || button > 14) {
        return;
    }
    
    uint16_t mask = (1 << (button - 1));
    
    if (pressed) {
        current_state.buttons |= mask;
    } else {
        current_state.buttons &= ~mask;
    }
    
    // Auto-send report if connected
    if (is_connected) {
        controller_send_gamepad_report(&current_state);
    }
}

void controller_set_left_stick(int8_t x, int8_t y)
{
    current_state.left_x = x;
    current_state.left_y = y;
    
    // Auto-send report if connected
    if (is_connected) {
        controller_send_gamepad_report(&current_state);
    }
}

void controller_set_right_stick(int8_t x, int8_t y)
{
    current_state.right_x = x;
    current_state.right_y = y;
    
    // Auto-send report if connected
    if (is_connected) {
        controller_send_gamepad_report(&current_state);
    }
}

void controller_set_triggers(uint8_t left, uint8_t right)
{
    current_state.left_trigger = left;
    current_state.right_trigger = right;
    
    // Auto-send report if connected
    if (is_connected) {
        controller_send_gamepad_report(&current_state);
    }
}

void controller_set_dpad(uint8_t direction)
{
    if (direction > 8) {
        direction = 0;
    }
    
    current_state.dpad = direction;
    
    // Convert D-Pad direction to button states
    // Clear D-Pad buttons first
    current_state.buttons &= ~((1 << (GAMEPAD_BUTTON_11 - 1)) |
                                (1 << (GAMEPAD_BUTTON_12 - 1)) |
                                (1 << (GAMEPAD_BUTTON_13 - 1)) |
                                (1 << (GAMEPAD_BUTTON_14 - 1)));
    
    // Set D-Pad buttons based on direction
    // Direction mapping: 0=center, 1=up, 2=up-right, 3=right, 4=down-right, 5=down, 6=down-left, 7=left, 8=up-left
    switch (direction) {
        case 1: // Up
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_11 - 1));
            break;
        case 2: // Up-Right
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_11 - 1));
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_14 - 1));
            break;
        case 3: // Right
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_14 - 1));
            break;
        case 4: // Down-Right
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_12 - 1));
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_14 - 1));
            break;
        case 5: // Down
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_12 - 1));
            break;
        case 6: // Down-Left
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_12 - 1));
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_13 - 1));
            break;
        case 7: // Left
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_13 - 1));
            break;
        case 8: // Up-Left
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_11 - 1));
            current_state.buttons |= (1 << (GAMEPAD_BUTTON_13 - 1));
            break;
        default:
            break;
    }
    
    // Auto-send report if connected
    if (is_connected) {
        controller_send_gamepad_report(&current_state);
    }
}

const gamepad_state_t* controller_get_state(void)
{
    return &current_state;
}

