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
 * @brief Lee el estado de un botón con debounce simple
 * @return true si el botón está presionado (LOW), false si está suelto (HIGH)
 */
static bool read_button(gpio_num_t gpio)
{
    // Leer el estado del GPIO (0 = presionado con pull-up, 1 = suelto)
    int level = gpio_get_level(gpio);
    return (level == 0);
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
 * @brief Convierte coordenadas X,Y a dirección del Hat Switch (D-Pad)
 * @param x Coordenada X normalizada (-1.0 a 1.0)
 * @param y Coordenada Y normalizada (-1.0 a 1.0)
 * @param threshold Umbral para considerar que está en una dirección
 * @return Valor del hat switch (0-7) o 8 si está en el centro
 */
static uint8_t joystick_to_hat_switch(float x, float y, float threshold)
{
    // Hat switch values: 0=North, 1=NE, 2=East, 3=SE, 4=South, 5=SW, 6=West, 7=NW, 8=Center
    bool left = (x < -threshold);
    bool right = (x > threshold);
    bool up = (y < -threshold);    // Y invertido: negativo es arriba
    bool down = (y > threshold);
    
    if (!left && !right && !up && !down) {
        return 8; // Centro
    }
    
    if (up && !left && !right) return 0;      // North
    if (up && right) return 1;                // Northeast
    if (right && !up && !down) return 2;      // East
    if (down && right) return 3;               // Southeast
    if (down && !left && !right) return 4;    // South
    if (down && left) return 5;                // Southwest
    if (left && !up && !down) return 6;       // West
    if (up && left) return 7;                  // Northwest
    
    return 8; // Centro (fallback)
}

/**
 * @brief Tarea para leer y enviar el estado completo del gamepad
 */
static void button_task(void *pvParameters)
{
    uint16_t current_conn_id = 0;
    bool current_connected = false;
    
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
    ESP_LOGI(TAG, "Rangos esperados - Min: %d mV, Max: %d mV, Centro teórico: %d mV", 
             min_mv, max_mv, center_mv);
    
    while (1) {
        // Obtener estado de conexión de forma thread-safe
        if (xSemaphoreTake(hid_conn_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
            current_conn_id = hid_conn_id;
            current_connected = hid_connected;
            xSemaphoreGive(hid_conn_mutex);
        }
        
        // Leer botones (con lectura directa para SELECT y START para debugging)
        bool a_pressed = read_button(GPIO_BUTTON_A);
        bool b_pressed = read_button(GPIO_BUTTON_B);
        bool select_pressed = read_button(GPIO_BUTTON_SELECT);
        bool start_pressed = read_button(GPIO_BUTTON_START);
        
        // Verificación adicional: leer directamente los niveles GPIO para SELECT y START
        // Esto ayuda a detectar problemas de hardware
        int select_level = gpio_get_level(GPIO_BUTTON_SELECT);
        int start_level = gpio_get_level(GPIO_BUTTON_START);
        
        // Verificar consistencia: si pressed es true, level debe ser 0 (LOW)
        // Si pressed es false, level debe ser 1 (HIGH)
        bool select_consistent = (select_pressed == (select_level == 0));
        bool start_consistent = (start_pressed == (start_level == 0));
        
        if (!select_consistent) {
            ESP_LOGW(TAG, "Inconsistencia SELECT - pressed: %d, level: %d", select_pressed, select_level);
        }
        if (!start_consistent) {
            ESP_LOGW(TAG, "Inconsistencia START - pressed: %d, level: %d", start_pressed, start_level);
        }
        
        // Debug: Log estado de botones cuando cambian
        static bool last_a = false, last_b = false, last_select = false, last_start = false;
        if (a_pressed != last_a || b_pressed != last_b || select_pressed != last_select || start_pressed != last_start) {
            ESP_LOGI(TAG, "Botones GPIO - A(GPIO13): %d, B(GPIO12): %d, SELECT(GPIO25): %d, START(GPIO26): %d",
                     a_pressed, b_pressed, select_pressed, start_pressed);
            // Verificar niveles GPIO directamente para debugging
            ESP_LOGI(TAG, "Niveles GPIO - A: %d, B: %d, SELECT: %d, START: %d",
                     gpio_get_level(GPIO_BUTTON_A),
                     gpio_get_level(GPIO_BUTTON_B),
                     gpio_get_level(GPIO_BUTTON_SELECT),
                     gpio_get_level(GPIO_BUTTON_START));
            last_a = a_pressed;
            last_b = b_pressed;
            last_select = select_pressed;
            last_start = start_pressed;
        }
        
        // Leer joystick
        int vrx_mv = read_adc_mv(channel_vrx, adc1_cali_handle_vrx);
        int vry_mv = read_adc_mv(channel_vry, adc1_cali_handle_vry);
        
        // Convertir a coordenadas normalizadas
        float x = normalize_joystick(vrx_mv, calibrated_center_vrx, min_mv, max_mv);
        float y = normalize_joystick(vry_mv, calibrated_center_vry, min_mv, max_mv);
        
        // Debug: Log valores crudos periódicamente (cada 2 segundos aproximadamente)
        static int debug_counter = 0;
        if (++debug_counter >= 40) { // 40 * 50ms = 2 segundos
            ESP_LOGI(TAG, "ADC raw - VRX: %d mV (centro: %d), VRY: %d mV (centro: %d)", 
                     vrx_mv, calibrated_center_vrx, vry_mv, calibrated_center_vry);
            ESP_LOGI(TAG, "Normalized - X: %.3f, Y: %.3f", x, y);
            debug_counter = 0;
        }
        
        // NO invertir eje Y - el joystick ya está en la orientación correcta
        // Si necesitas invertir, quita el comentario de la siguiente línea:
        // y = -y;
        
        // Aplicar zona muerta (dead zone) para evitar drift en el centro
        const float dead_zone = 0.12f; // 12% de zona muerta
        if (fabsf(x) < dead_zone) x = 0.0f;
        if (fabsf(y) < dead_zone) y = 0.0f;
        
        // Aplicar curva de respuesta suave (para mejor control)
        // Usar una curva que sea más suave cerca del centro y más lineal al final
        if (x != 0.0f) {
            float abs_x = fabsf(x);
            float sign = (x > 0) ? 1.0f : -1.0f;
            // Curva: más suave al inicio, más lineal al final
            x = sign * (abs_x * (0.7f + 0.3f * abs_x));
        }
        if (y != 0.0f) {
            float abs_y = fabsf(y);
            float sign = (y > 0) ? 1.0f : -1.0f;
            // Curva: más suave al inicio, más lineal al final
            y = sign * (abs_y * (0.7f + 0.3f * abs_y));
        }
        
        // Convertir a valores de joystick (-127 a 127)
        int8_t x_axis = (int8_t)(x * 127.0f);
        int8_t y_axis = (int8_t)(y * 127.0f);
        
        // Convertir joystick a Hat Switch (D-Pad) solo si hay movimiento significativo
        // NOTA: El hat switch usa valores 0-7 donde 0=North, 1=NE, 2=East, etc.
        // Cuando está en el centro, usamos 15 (0x0F) que muchos sistemas HID
        // interpretan como "centro" o "sin dirección" (valor fuera del rango lógico 0-7)
        const float hat_threshold = 0.4f; // 40% del rango para activar D-Pad
        uint8_t hat_switch = 15; // Por defecto, centro (15 = 0x0F, significa "sin dirección")
        
        // Solo activar hat switch si hay movimiento significativo Y el joystick no está en zona muerta
        if (x != 0.0f || y != 0.0f) {
            if (fabsf(x) > hat_threshold || fabsf(y) > hat_threshold) {
                uint8_t hat_switch_raw = joystick_to_hat_switch(x, y, hat_threshold);
                // Si está en el centro (8), mantener 15. Si no, usar la dirección real (0-7)
                if (hat_switch_raw != 8) {
                    hat_switch = hat_switch_raw;
                }
                // Si hat_switch_raw es 8 (centro), hat_switch permanece en 15
            }
            // Si no hay movimiento suficiente, hat_switch permanece en 15 (centro)
        }
        // Si x e y están ambos en 0 (zona muerta), hat_switch permanece en 15 (centro)
        
        // Preparar botones del gamepad (4 bits: Button 1, Button 2, Button 3, Button 4)
        // Mapeo físico a lógico según estándar HID Gamepad:
        // - Button 1 (bit 0 = 0x01): Botón A físico (GPIO 13) - Acción principal
        // - Button 2 (bit 1 = 0x02): Botón B físico (GPIO 12) - Acción secundaria
        // - Button 3 (bit 2 = 0x04): Botón SELECT físico (GPIO 25)
        // - Button 4 (bit 3 = 0x08): Botón START físico (GPIO 26)
        //
        // Mapeo estándar para emuladores:
        // - Button 1 = A (acción principal)
        // - Button 2 = B (acción secundaria)
        // - Button 3 = SELECT
        // - Button 4 = START
        uint8_t gamepad_buttons = 0;
        
        // Mapear botones directamente - asegurar que los bits se establezcan correctamente
        if (a_pressed) {
            gamepad_buttons |= 0x01;       // Button 1 -> A físico (GPIO 13) - bit 0
        }
        if (b_pressed) {
            gamepad_buttons |= 0x02;       // Button 2 -> B físico (GPIO 12) - bit 1
        }
        if (select_pressed) {
            gamepad_buttons |= 0x04;       // Button 3 -> SELECT físico (GPIO 25) - bit 2
        }
        if (start_pressed) {
            gamepad_buttons |= 0x08;       // Button 4 -> START físico (GPIO 26) - bit 3
        }
        
        // Debug específico para SELECT y START - siempre loggear cuando están presionados
        if (select_pressed || start_pressed) {
            ESP_LOGI(TAG, ">>> SELECT/START PRESIONADOS - SELECT: %d (GPIO25 level=%d), START: %d (GPIO26 level=%d)", 
                     select_pressed, gpio_get_level(GPIO_BUTTON_SELECT),
                     start_pressed, gpio_get_level(GPIO_BUTTON_START));
            ESP_LOGI(TAG, ">>> gamepad_buttons byte: 0x%02X (binario: 0b%04b)", gamepad_buttons, gamepad_buttons);
            ESP_LOGI(TAG, ">>> Bits individuales - A: %d, B: %d, SEL: %d, ST: %d",
                     (gamepad_buttons & 0x01) ? 1 : 0,
                     (gamepad_buttons & 0x02) ? 1 : 0,
                     (gamepad_buttons & 0x04) ? 1 : 0,
                     (gamepad_buttons & 0x08) ? 1 : 0);
        }
        
        // Detectar cambios en botones
        bool state_changed = (a_pressed != button_a_state) ||
                            (b_pressed != button_b_state) ||
                            (select_pressed != button_select_state) ||
                            (start_pressed != button_start_state);
        
        // Detectar cambios en el joystick (solo si hay movimiento significativo)
        static int8_t last_x_axis = 0;
        static int8_t last_y_axis = 0;
        static uint8_t last_hat_switch = 0;
        static uint8_t last_gamepad_buttons = 0;
        
        bool joystick_changed = (abs(x_axis - last_x_axis) > 2) || 
                               (abs(y_axis - last_y_axis) > 2) ||
                               (hat_switch != last_hat_switch);
        
        // Detectar cambios en botones (comparar con el último valor enviado)
        bool buttons_changed = (gamepad_buttons != last_gamepad_buttons);
        
        // Actualizar estados locales
        button_a_state = a_pressed;
        button_b_state = b_pressed;
        button_select_state = select_pressed;
        button_start_state = start_pressed;
        
        // Enviar datos del gamepad si hay conexión
        // IMPORTANTE: Enviar siempre que haya cambios en botones o joystick
        // START y SELECT se comportan igual que A y B: solo se envían cuando hay cambio de estado
        if (current_connected) {
            static int send_counter = 0;
            bool should_send = false;
            
            // PRIORIDAD 1: Enviar SIEMPRE si hay cambios en botones o joystick
            // Esto incluye cambios en START y SELECT, igual que A y B
            if (buttons_changed || joystick_changed) {
                should_send = true;
            }
            
            // PRIORIDAD 2: Enviar periódicamente (cada 100ms) si hay botones presionados
            // Esto aplica a todos los botones por igual (A, B, SELECT, START)
            if (gamepad_buttons != 0 && !should_send) {
                send_counter++;
                if (send_counter >= 2) { // 2 * 50ms = 100ms
                    should_send = true;
                    send_counter = 0;
                }
            } else if (gamepad_buttons == 0) {
                send_counter = 0;
            }
            
            // Enviar si es necesario
            if (should_send) {
                esp_hidd_send_gamepad_value(current_conn_id, gamepad_buttons, hat_switch, x_axis, y_axis);
                last_x_axis = x_axis;
                last_y_axis = y_axis;
                last_hat_switch = hat_switch;
                last_gamepad_buttons = gamepad_buttons;
                
                // Loggear cuando hay cambios en botones
                if (buttons_changed) {
                    ESP_LOGI(TAG, "Gamepad ENVIADO - Buttons: 0x%02X (A:%d B:%d SEL:%d ST:%d), Hat: %d, X: %d, Y: %d", 
                             gamepad_buttons, 
                             (gamepad_buttons & 0x01) ? 1 : 0,
                             (gamepad_buttons & 0x02) ? 1 : 0,
                             (gamepad_buttons & 0x04) ? 1 : 0,
                             (gamepad_buttons & 0x08) ? 1 : 0,
                             hat_switch, x_axis, y_axis);
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // Leer cada 50ms
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

    // Crear tarea para gamepad (combina botones y joystick)
    BaseType_t task_ret;
    task_ret = xTaskCreate(button_task, "gamepad_task", 4096, NULL, 5, NULL);
    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create gamepad task");
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

