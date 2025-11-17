/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "controller.h"
#include "esp_hidd_prf_api.h"
#include "hid_dev.h"
#include "freertos/semphr.h"

static const char *TAG = "CONTROLLER";

#define ADC_ATTEN           ADC_ATTEN_DB_11
#define ADC_BITWIDTH        ADC_BITWIDTH_12

// Handles para ADC
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc1_cali_handle_vrx = NULL;
static adc_cali_handle_t adc1_cali_handle_vry = NULL;

// Estados anteriores de los botones para detectar cambios
static bool button_a_state = false;
static bool button_b_state = false;
static bool button_select_state = false;
static bool button_start_state = false;

// Estado de conexión HID
static uint16_t hid_conn_id = 0;
static bool hid_connected = false;
static SemaphoreHandle_t hid_conn_mutex = NULL;

/**
 * @brief Calibración del ADC
 */
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is %s", "Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = atten,
            .bitwidth = ADC_BITWIDTH,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Calibration Success");
    } else if (ret == ESP_ERR_NOT_SUPPORTED || !calibrated) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else {
        ESP_LOGE(TAG, "Invalid arg or no memory");
    }

    return calibrated;
}

/**
 * @brief Inicialización del ADC
 */
static esp_err_t adc_init(void)
{
    esp_err_t ret = ESP_OK;
    
    // Configuración del ADC1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ret = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC unit");
        return ret;
    }

    // Mapear GPIO a canales ADC
    adc_channel_t channel_vrx = ADC_CHANNEL_0;  // GPIO 36 -> ADC1_CH0
    adc_channel_t channel_vry = ADC_CHANNEL_3;  // GPIO 39 -> ADC1_CH3

    // Configuración del canal VRX
    adc_oneshot_chan_cfg_t config_vrx = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ret = adc_oneshot_config_channel(adc1_handle, channel_vrx, &config_vrx);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure VRX channel");
        return ret;
    }

    // Configuración del canal VRY
    adc_oneshot_chan_cfg_t config_vry = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ret = adc_oneshot_config_channel(adc1_handle, channel_vry, &config_vry);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure VRY channel");
        return ret;
    }

    // Calibración
    adc_calibration_init(ADC_UNIT_1, channel_vrx, ADC_ATTEN, &adc1_cali_handle_vrx);
    adc_calibration_init(ADC_UNIT_1, channel_vry, ADC_ATTEN, &adc1_cali_handle_vry);

    return ESP_OK;
}

/**
 * @brief Inicialización de los botones GPIO
 */
static esp_err_t buttons_init(void)
{
    // Configuración de los pines como entradas con pull-up
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << GPIO_BUTTON_A) | 
                        (1ULL << GPIO_BUTTON_B) | 
                        (1ULL << GPIO_BUTTON_SELECT) | 
                        (1ULL << GPIO_BUTTON_START),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO buttons");
        return ret;
    }
    
    return ESP_OK;
}

/**
 * @brief Lee el estado de un botón
 * @return true si el botón está presionado (LOW), false si está suelto (HIGH)
 */
static bool read_button(gpio_num_t gpio)
{
    return (gpio_get_level(gpio) == 0);
}

/**
 * @brief Lee el valor del ADC y lo convierte a milivoltios
 */
static int read_adc_mv(adc_channel_t channel, adc_cali_handle_t cali_handle)
{
    int adc_raw;
    int voltage = 0;
    
    esp_err_t ret = adc_oneshot_read(adc1_handle, channel, &adc_raw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error reading ADC");
        return 0;
    }
    
    if (cali_handle) {
        ret = adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Error converting ADC raw value to voltage");
        }
    } else {
        // Sin calibración, usar valor raw
        voltage = adc_raw;
    }
    
    return voltage;
}

/**
 * @brief Tarea para leer y mostrar el estado de los botones
 */
static void button_task(void *pvParameters)
{
    uint16_t current_conn_id = 0;
    bool current_connected = false;
    
    while (1) {
        // Obtener estado de conexión de forma thread-safe
        if (xSemaphoreTake(hid_conn_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_conn_id = hid_conn_id;
            current_connected = hid_connected;
            xSemaphoreGive(hid_conn_mutex);
        }
        
        bool a_pressed = read_button(GPIO_BUTTON_A);
        bool b_pressed = read_button(GPIO_BUTTON_B);
        bool select_pressed = read_button(GPIO_BUTTON_SELECT);
        bool start_pressed = read_button(GPIO_BUTTON_START);

        // Mapeo de botones del controlador a botones del mouse HID (para gamepad)
        // Botón A -> Botón izquierdo del mouse (bit 0 = 0x01)
        // Botón B -> Botón derecho del mouse (bit 1 = 0x02)
        // Botón SELECT -> Botón medio del mouse (bit 2 = 0x04)
        // Botón START -> Tecla Enter (para compatibilidad con emuladores)
        
        // Calcular el estado combinado de los botones del mouse
        uint8_t mouse_buttons = 0;
        if (a_pressed) mouse_buttons |= 0x01;      // Botón izquierdo
        if (b_pressed) mouse_buttons |= 0x02;      // Botón derecho
        if (select_pressed) mouse_buttons |= 0x04; // Botón medio

        // Solo procesar si hay un cambio de estado en los botones del mouse
        uint8_t prev_mouse_buttons = 0;
        if (button_a_state) prev_mouse_buttons |= 0x01;
        if (button_b_state) prev_mouse_buttons |= 0x02;
        if (button_select_state) prev_mouse_buttons |= 0x04;
        
        if (mouse_buttons != prev_mouse_buttons) {
            button_a_state = a_pressed;
            button_b_state = b_pressed;
            button_select_state = select_pressed;
            
            if (current_connected) {
                // Enviar estado de botones del mouse (sin movimiento)
                esp_hidd_send_mouse_value(current_conn_id, mouse_buttons, 0, 0);
                ESP_LOGI(TAG, "Botones mouse: A=%d B=%d SELECT=%d (mask=0x%02X)", 
                         a_pressed, b_pressed, select_pressed, mouse_buttons);
            }
        }
        
        // START se envía como teclado (Enter) para compatibilidad
        if (start_pressed != button_start_state) {
            button_start_state = start_pressed;
            if (start_pressed) {
                ESP_LOGI(TAG, "Botón START: PRESIONADO");
                if (current_connected) {
                    uint8_t key = HID_KEY_RETURN;
                    esp_hidd_send_keyboard_value(current_conn_id, 0, &key, 1);
                }
            } else {
                ESP_LOGI(TAG, "Botón START: SUELTO");
                if (current_connected) {
                    uint8_t key = 0;
                    esp_hidd_send_keyboard_value(current_conn_id, 0, &key, 1);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Leer cada 50ms
    }
}

/**
 * @brief Convierte un valor de milivoltios a coordenada normalizada (-1.0 a 1.0)
 * @param mv Valor en milivoltios
 * @param center_mv Valor en mV cuando el joystick está en el centro
 * @param min_mv Valor mínimo en mV
 * @param max_mv Valor máximo en mV
 * @return Coordenada normalizada de -1.0 a 1.0
 */
static float normalize_joystick(int mv, int center_mv, int min_mv, int max_mv)
{
    // Limitar el valor al rango
    if (mv < min_mv) mv = min_mv;
    if (mv > max_mv) mv = max_mv;
    
    // Calcular la desviación desde el centro
    float deviation = (float)(mv - center_mv);
    
    // Normalizar al rango -1.0 a 1.0
    // Usar el rango más grande (arriba o abajo del centro) para normalizar
    float range_up = (float)(max_mv - center_mv);
    float range_down = (float)(center_mv - min_mv);
    float max_range = (range_up > range_down) ? range_up : range_down;
    
    if (max_range > 0) {
        return deviation / max_range;
    }
    return 0.0f;
}

/**
 * @brief Tarea para leer y mostrar los valores del joystick
 */
static void joystick_task(void *pvParameters)
{
    float last_x = 999.0f;  // Valor inicial imposible
    float last_y = 999.0f;
    const float threshold = 0.05f; // Umbral de cambio (5% del rango)
    
    adc_channel_t channel_vrx = ADC_CHANNEL_0;  // GPIO 36
    adc_channel_t channel_vry = ADC_CHANNEL_3;  // GPIO 39

    // Parámetros de calibración del joystick
    // Estos valores pueden ajustarse según tu joystick específico
    // Para un joystick típico con alimentación de 3.3V:
    const int min_mv = 0;        // Mínimo teórico
    const int max_mv = 3300;     // Máximo teórico (3.3V)
    const int center_mv = 1650;  // Centro teórico (1.65V)
    
    // Leer valores iniciales para calibrar el centro real
    int initial_vrx = 0;
    int initial_vry = 0;
    const int calibration_samples = 10;
    for (int i = 0; i < calibration_samples; i++) {
        initial_vrx += read_adc_mv(channel_vrx, adc1_cali_handle_vrx);
        initial_vry += read_adc_mv(channel_vry, adc1_cali_handle_vry);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    int calibrated_center_vrx = initial_vrx / calibration_samples;
    int calibrated_center_vry = initial_vry / calibration_samples;
    
    ESP_LOGI(TAG, "Joystick calibrado - Centro VRX: %d mV, Centro VRY: %d mV", 
             calibrated_center_vrx, calibrated_center_vry);

    uint16_t current_conn_id = 0;
    bool current_connected = false;

    while (1) {
        // Obtener estado de conexión de forma thread-safe
        if (xSemaphoreTake(hid_conn_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_conn_id = hid_conn_id;
            current_connected = hid_connected;
            xSemaphoreGive(hid_conn_mutex);
        }
        
        int vrx_mv = read_adc_mv(channel_vrx, adc1_cali_handle_vrx);
        int vry_mv = read_adc_mv(channel_vry, adc1_cali_handle_vry);

        // Convertir a coordenadas normalizadas
        float x = normalize_joystick(vrx_mv, calibrated_center_vrx, min_mv, max_mv);
        float y = normalize_joystick(vry_mv, calibrated_center_vry, min_mv, max_mv);

        // Convertir joystick a D-Pad (direcciones discretas)
        // Umbral para considerar que el joystick está en una dirección
        const float dpad_threshold = 0.3f; // 30% del rango
        // Sensibilidad del D-Pad (valores más pequeños para movimiento más controlado)
        const int8_t dpad_sensitivity = 20; // Ajustar según necesidad
        
        int8_t dpad_x = 0;
        int8_t dpad_y = 0;
        
        // Determinar dirección D-Pad basada en el joystick
        if (x < -dpad_threshold) {
            dpad_x = -dpad_sensitivity; // LEFT
        } else if (x > dpad_threshold) {
            dpad_x = dpad_sensitivity;  // RIGHT
        }
        
        if (y < -dpad_threshold) {
            dpad_y = -dpad_sensitivity; // UP (Y invertido)
        } else if (y > dpad_threshold) {
            dpad_y = dpad_sensitivity;  // DOWN (Y invertido)
        }
        
        // Calcular dirección anterior para detectar cambios
        int8_t last_dpad_x = 0;
        int8_t last_dpad_y = 0;
        if (last_x < -dpad_threshold) last_dpad_x = -dpad_sensitivity;
        else if (last_x > dpad_threshold) last_dpad_x = dpad_sensitivity;
        if (last_y < -dpad_threshold) last_dpad_y = -dpad_sensitivity;
        else if (last_y > dpad_threshold) last_dpad_y = dpad_sensitivity;
        
        // Solo procesar si hay un cambio de dirección o si el joystick está activo
        if (dpad_x != last_dpad_x || dpad_y != last_dpad_y || 
            (dpad_x != 0 || dpad_y != 0)) {
            
            ESP_LOGI(TAG, "Joystick - x: %.3f, y: %.3f -> D-Pad: X=%d, Y=%d", 
                     x, y, dpad_x, dpad_y);
            
            // Enviar movimiento del mouse como D-Pad si hay conexión
            if (current_connected) {
                // Obtener estado actual de los botones del mouse
                uint8_t current_mouse_buttons = 0;
                if (button_a_state) current_mouse_buttons |= 0x01;
                if (button_b_state) current_mouse_buttons |= 0x02;
                if (button_select_state) current_mouse_buttons |= 0x04;
                
                // Enviar movimiento del mouse (D-Pad) manteniendo los botones
                // Solo enviar si hay una dirección activa
                if (dpad_x != 0 || dpad_y != 0) {
                    esp_hidd_send_mouse_value(current_conn_id, current_mouse_buttons, dpad_x, dpad_y);
                } else {
                    // Si el joystick está en el centro, enviar sin movimiento pero con botones
                    esp_hidd_send_mouse_value(current_conn_id, current_mouse_buttons, 0, 0);
                }
            }
            
            last_x = x;
            last_y = y;
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Leer cada 50ms para mejor respuesta
    }
}

esp_err_t controller_init(void)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Inicializando controlador de botones y joystick...");

    // Crear mutex para acceso thread-safe a la conexión HID
    hid_conn_mutex = xSemaphoreCreateMutex();
    if (hid_conn_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create HID connection mutex");
        return ESP_ERR_NO_MEM;
    }

    // Inicializar botones
    ret = buttons_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize buttons");
        return ret;
    }
    ESP_LOGI(TAG, "Botones inicializados en GPIO: A=%d, B=%d, SELECT=%d, START=%d",
             GPIO_BUTTON_A, GPIO_BUTTON_B, GPIO_BUTTON_SELECT, GPIO_BUTTON_START);

    // Inicializar ADC para joystick
    ret = adc_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize ADC");
        return ret;
    }
    ESP_LOGI(TAG, "ADC inicializado para joystick: VRX=GPIO%d (ADC1_CH0), VRY=GPIO%d (ADC1_CH3)",
             GPIO_JOYSTICK_VRX, GPIO_JOYSTICK_VRY);

    // Crear tareas
    BaseType_t task_ret;
    task_ret = xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create button task");
        return ESP_FAIL;
    }

    task_ret = xTaskCreate(joystick_task, "joystick_task", 2048, NULL, 5, NULL);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create joystick task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sistema iniciado. Presiona botones o mueve el joystick para ver la salida.");

    return ESP_OK;
}

void controller_set_hid_connection(uint16_t conn_id)
{
    if (hid_conn_mutex != NULL) {
        if (xSemaphoreTake(hid_conn_mutex, portMAX_DELAY) == pdTRUE) {
            hid_conn_id = conn_id;
            hid_connected = true;
            xSemaphoreGive(hid_conn_mutex);
            ESP_LOGI(TAG, "Conexión HID establecida (conn_id=%d)", conn_id);
        }
    }
}

void controller_clear_hid_connection(void)
{
    if (hid_conn_mutex != NULL) {
        if (xSemaphoreTake(hid_conn_mutex, portMAX_DELAY) == pdTRUE) {
            hid_connected = false;
            hid_conn_id = 0;
            xSemaphoreGive(hid_conn_mutex);
            ESP_LOGI(TAG, "Conexión HID cerrada");
        }
    }
}

