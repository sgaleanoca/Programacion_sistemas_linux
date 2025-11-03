#include "joystick_buttons.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "JOYSTICK_BUTTONS";

// Handles para el ADC
static adc_oneshot_unit_handle_t adc1_handle = NULL;

// Canales ADC para el joystick
static adc_channel_t adc_channel_x = ADC_CHANNEL_6;  // GPIO 34 -> ADC1_CHANNEL_6
static adc_channel_t adc_channel_y = ADC_CHANNEL_7;  // GPIO 35 -> ADC1_CHANNEL_7

/**
 * @brief Inicializa el joystick y los botones
 */
bool joystick_buttons_init(void) {
    esp_err_t ret;

    // Configurar ADC para el joystick usando la nueva API de ESP-IDF v5.x
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
    };
    
    ret = adc_oneshot_new_unit(&init_config, &adc1_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al crear unidad ADC: %s", esp_err_to_name(ret));
        return false;
    }

    // Configurar canal X (VRX)
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_11,  // Rango de 0-3.3V
    };
    
    ret = adc_oneshot_config_channel(adc1_handle, adc_channel_x, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el canal X del ADC: %s", esp_err_to_name(ret));
        return false;
    }

    // Configurar canal Y (VRY)
    ret = adc_oneshot_config_channel(adc1_handle, adc_channel_y, &config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar el canal Y del ADC: %s", esp_err_to_name(ret));
        return false;
    }

    // Configurar GPIO para los botones como inputs con pull-up interno
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_A_GPIO) | 
                        (1ULL << BUTTON_B_GPIO) | 
                        (1ULL << BUTTON_START_GPIO) | 
                        (1ULL << BUTTON_SELECT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,  // Pull-up habilitado (botón presionado = LOW)
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al configurar los GPIO de los botones: %s", esp_err_to_name(ret));
        return false;
    }

    ESP_LOGI(TAG, "Joystick y botones inicializados correctamente");
    ESP_LOGI(TAG, "Botones: A=%d, B=%d, Start=%d, Select=%d", 
             BUTTON_A_GPIO, BUTTON_B_GPIO, BUTTON_START_GPIO, BUTTON_SELECT_GPIO);
    ESP_LOGI(TAG, "Joystick: VRX=%d, VRY=%d", JOYSTICK_VRX_GPIO, JOYSTICK_VRY_GPIO);

    return true;
}

/**
 * @brief Lee el estado actual del joystick
 */
bool joystick_read(joystick_data_t *joystick) {
    if (joystick == NULL) {
        ESP_LOGE(TAG, "Puntero nulo en joystick_read");
        return false;
    }

    if (adc1_handle == NULL) {
        ESP_LOGE(TAG, "ADC no inicializado");
        return false;
    }

    int adc_x, adc_y;

    // Leer valores ADC (rango 0-4095 para 12 bits)
    esp_err_t ret = adc_oneshot_read(adc1_handle, adc_channel_x, &adc_x);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer canal X: %s", esp_err_to_name(ret));
        return false;
    }

    ret = adc_oneshot_read(adc1_handle, adc_channel_y, &adc_y);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error al leer canal Y: %s", esp_err_to_name(ret));
        return false;
    }

    // Asignar valores (normalmente el joystick analógico tiene valores centrados alrededor de 2048)
    joystick->x = (int16_t)adc_x;
    joystick->y = (int16_t)adc_y;

    return true;
}

/**
 * @brief Lee el estado actual de todos los botones
 */
bool buttons_read(buttons_state_t *buttons) {
    if (buttons == NULL) {
        ESP_LOGE(TAG, "Puntero nulo en buttons_read");
        return false;
    }

    // Leer estado de los botones (0 = presionado debido a pull-up)
    buttons->button_a = !gpio_get_level(BUTTON_A_GPIO);     // Invertir porque pull-up
    buttons->button_b = !gpio_get_level(BUTTON_B_GPIO);     // Invertir porque pull-up
    buttons->button_start = !gpio_get_level(BUTTON_START_GPIO);   // Invertir porque pull-up
    buttons->button_select = !gpio_get_level(BUTTON_SELECT_GPIO); // Invertir porque pull-up

    return true;
}

/**
 * @brief Lee el estado completo del controlador
 */
bool controller_read(controller_state_t *state) {
    if (state == NULL) {
        ESP_LOGE(TAG, "Puntero nulo en controller_read");
        return false;
    }

    bool ret1 = joystick_read(&state->joystick);
    bool ret2 = buttons_read(&state->buttons);

    return ret1 && ret2;
}

/**
 * @brief Imprime el estado del joystick en el monitor
 */
void joystick_print(joystick_data_t *joystick) {
    if (joystick == NULL) {
        return;
    }

    // Calcular valores centrados (asumiendo centro en ~2048)
    int16_t x_centered = joystick->x - 2048;
    int16_t y_centered = joystick->y - 2048;
    
    // Determinar dirección
    const char* dir_x = (x_centered > 100) ? "DERECHA" : (x_centered < -100) ? "IZQUIERDA" : "CENTRO";
    const char* dir_y = (y_centered > 100) ? "ARRIBA" : (y_centered < -100) ? "ABAJO" : "CENTRO";

    ESP_LOGI(TAG, "Joystick - VRX: %d (centrado: %+d, %s)", 
             joystick->x, x_centered, dir_x);
    ESP_LOGI(TAG, "          VRY: %d (centrado: %+d, %s)", 
             joystick->y, y_centered, dir_y);
}

/**
 * @brief Imprime el estado de los botones en el monitor
 */
void buttons_print(buttons_state_t *buttons) {
    if (buttons == NULL) {
        return;
    }

    ESP_LOGI(TAG, "Botones - A: %s | B: %s | Start: %s | Select: %s",
             buttons->button_a ? "PRESIONADO" : "Libre",
             buttons->button_b ? "PRESIONADO" : "Libre",
             buttons->button_start ? "PRESIONADO" : "Libre",
             buttons->button_select ? "PRESIONADO" : "Libre");
}

/**
 * @brief Imprime el estado completo del controlador en el monitor
 */
void controller_print(controller_state_t *state) {
    if (state == NULL) {
        return;
    }

    joystick_print(&state->joystick);
    buttons_print(&state->buttons);
}

