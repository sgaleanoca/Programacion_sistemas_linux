/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Definición de pines GPIO para los botones
// NOTA: Cambia estos valores según tu conexión física
#define GPIO_BUTTON_A       GPIO_NUM_13
#define GPIO_BUTTON_B       GPIO_NUM_12
#define GPIO_BUTTON_SELECT  GPIO_NUM_25
#define GPIO_BUTTON_START   GPIO_NUM_26

// Definición de GPIO para el joystick (ADC)
// NOTA: Cambia estos valores según tu conexión física
// Para ESP32, los GPIO ADC1 son: 36, 37, 38, 39, 32, 33, 34, 35
#define GPIO_JOYSTICK_VRX   GPIO_NUM_36  // ADC1_CH0
#define GPIO_JOYSTICK_VRY   GPIO_NUM_39  // ADC1_CH3

/**
 * @brief Inicializa el controlador (botones y joystick)
 * 
 * Esta función inicializa:
 * - Los pines GPIO para los botones
 * - El ADC para el joystick
 * - Las tareas de lectura de botones y joystick
 * 
 * @return ESP_OK si la inicialización fue exitosa, código de error en caso contrario
 */
esp_err_t controller_init(void);

#ifdef __cplusplus
}
#endif

#endif // CONTROLLER_H

