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
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

static const char *TAG = "CONTROLLER";

// Definición de pines GPIO para los botones
// NOTA: Cambia estos valores según tu conexión física
#define GPIO_BUTTON_A       GPIO_NUM_13
#define GPIO_BUTTON_B      GPIO_NUM_12
#define GPIO_BUTTON_SELECT  GPIO_NUM_25
#define GPIO_BUTTON_START   GPIO_NUM_26

// Definición de GPIO para el joystick (ADC)
// NOTA: Cambia estos valores según tu conexión física
// Para ESP32, los GPIO ADC1 son: 36, 37, 38, 39, 32, 33, 34, 35
#define GPIO_JOYSTICK_VRX   GPIO_NUM_36  // ADC1_CH0
#define GPIO_JOYSTICK_VRY   GPIO_NUM_39  // ADC1_CH3
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
static void adc_init(void)
{
    // Configuración del ADC1
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // Mapear GPIO a canales ADC
    adc_channel_t channel_vrx = ADC_CHANNEL_0;  // GPIO 36 -> ADC1_CH0
    adc_channel_t channel_vry = ADC_CHANNEL_3;  // GPIO 39 -> ADC1_CH3

    // Configuración del canal VRX
    adc_oneshot_chan_cfg_t config_vrx = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel_vrx, &config_vrx));

    // Configuración del canal VRY
    adc_oneshot_chan_cfg_t config_vry = {
        .bitwidth = ADC_BITWIDTH,
        .atten = ADC_ATTEN,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, channel_vry, &config_vry));

    // Calibración
    adc_calibration_init(ADC_UNIT_1, channel_vrx, ADC_ATTEN, &adc1_cali_handle_vrx);
    adc_calibration_init(ADC_UNIT_1, channel_vry, ADC_ATTEN, &adc1_cali_handle_vry);
}

/**
 * @brief Inicialización de los botones GPIO
 */
static void buttons_init(void)
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
    gpio_config(&io_conf);
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
    
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, channel, &adc_raw));
    
    if (cali_handle) {
        esp_err_t ret = adc_cali_raw_to_voltage(cali_handle, adc_raw, &voltage);
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
    while (1) {
        bool a_pressed = read_button(GPIO_BUTTON_A);
        bool b_pressed = read_button(GPIO_BUTTON_B);
        bool select_pressed = read_button(GPIO_BUTTON_SELECT);
        bool start_pressed = read_button(GPIO_BUTTON_START);

        // Solo imprimir si hay un cambio de estado
        if (a_pressed != button_a_state) {
            button_a_state = a_pressed;
            if (a_pressed) {
                printf("Botón A: PRESIONADO\n");
            } else {
                printf("Botón A: SUELTO\n");
            }
        }

        if (b_pressed != button_b_state) {
            button_b_state = b_pressed;
            if (b_pressed) {
                printf("Botón B: PRESIONADO\n");
            } else {
                printf("Botón B: SUELTO\n");
            }
        }

        if (select_pressed != button_select_state) {
            button_select_state = select_pressed;
            if (select_pressed) {
                printf("Botón SELECT: PRESIONADO\n");
            } else {
                printf("Botón SELECT: SUELTO\n");
            }
        }

        if (start_pressed != button_start_state) {
            button_start_state = start_pressed;
            if (start_pressed) {
                printf("Botón START: PRESIONADO\n");
            } else {
                printf("Botón START: SUELTO\n");
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

    while (1) {
        int vrx_mv = read_adc_mv(channel_vrx, adc1_cali_handle_vrx);
        int vry_mv = read_adc_mv(channel_vry, adc1_cali_handle_vry);

        // Convertir a coordenadas normalizadas
        float x = normalize_joystick(vrx_mv, calibrated_center_vrx, min_mv, max_mv);
        float y = normalize_joystick(vry_mv, calibrated_center_vry, min_mv, max_mv);

        // Solo imprimir si hay un cambio significativo
        if (fabsf(x - last_x) > threshold || fabsf(y - last_y) > threshold) {
            printf("Joystick - x: %.3f, y: %.3f\n", x, y);
            last_x = x;
            last_y = y;
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Leer cada 50ms para mejor respuesta
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Inicializando controlador de botones y joystick...");

    // Inicializar botones
    buttons_init();
    ESP_LOGI(TAG, "Botones inicializados en GPIO: A=%d, B=%d, SELECT=%d, START=%d",
             GPIO_BUTTON_A, GPIO_BUTTON_B, GPIO_BUTTON_SELECT, GPIO_BUTTON_START);

    // Inicializar ADC para joystick
    adc_init();
    ESP_LOGI(TAG, "ADC inicializado para joystick: VRX=GPIO%d (ADC1_CH0), VRY=GPIO%d (ADC1_CH3)",
             GPIO_JOYSTICK_VRX, GPIO_JOYSTICK_VRY);

    // Crear tareas
    xTaskCreate(button_task, "button_task", 2048, NULL, 5, NULL);
    xTaskCreate(joystick_task, "joystick_task", 2048, NULL, 5, NULL);

    ESP_LOGI(TAG, "Sistema iniciado. Presiona botones o mueve el joystick para ver la salida.");
}
