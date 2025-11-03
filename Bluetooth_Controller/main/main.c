#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "joystick_buttons.h"
#include <stdlib.h>

static const char *TAG = "MAIN";

void app_main(void) {
    ESP_LOGI(TAG, "=========================================");
    ESP_LOGI(TAG, "Controlador Bluetooth - Joystick y Botones");
    ESP_LOGI(TAG, "=========================================");
    
    // Inicializar joystick y botones
    if (!joystick_buttons_init()) {
        ESP_LOGE(TAG, "Error al inicializar joystick y botones");
        return;
    }
    
    ESP_LOGI(TAG, "Controlador listo. Monitoreando entradas...");
    ESP_LOGI(TAG, "Pulsa los botones o mueve el joystick para ver los cambios");
    ESP_LOGI(TAG, "");
    
    controller_state_t controller_state;
    controller_state_t prev_state = {0};
    bool first_read = true;
    
    // Umbral para considerar que el joystick ha cambiado (evita ruido)
    const int16_t JOYSTICK_THRESHOLD = 50;  // Cambio mínimo de 50 unidades ADC
    
    while (1) {
        // Leer estado completo del controlador
        if (controller_read(&controller_state)) {
            // Comparar con el estado anterior para mostrar solo cambios
            // Para el joystick, usar umbral para evitar ruido
            int16_t joystick_delta_x = abs(controller_state.joystick.x - prev_state.joystick.x);
            int16_t joystick_delta_y = abs(controller_state.joystick.y - prev_state.joystick.y);
            bool joystick_changed = first_read || 
                                    (joystick_delta_x > JOYSTICK_THRESHOLD) || 
                                    (joystick_delta_y > JOYSTICK_THRESHOLD);
            
            bool buttons_changed = first_read ||
                                   controller_state.buttons.button_a != prev_state.buttons.button_a ||
                                   controller_state.buttons.button_b != prev_state.buttons.button_b ||
                                   controller_state.buttons.button_start != prev_state.buttons.button_start ||
                                   controller_state.buttons.button_select != prev_state.buttons.button_select;
            
            bool changed = joystick_changed || buttons_changed;
            
            if (changed) {
                // Mostrar en el monitor
                ESP_LOGI(TAG, "┌─────────────────────────────────────┐");
                if (joystick_changed && !first_read) {
                    ESP_LOGI(TAG, "│  JOYSTICK MOVIDO                   │");
                }
                if (buttons_changed && !first_read) {
                    ESP_LOGI(TAG, "│  BOTÓN PRESIONADO                  │");
                }
                ESP_LOGI(TAG, "└─────────────────────────────────────┘");
                controller_print(&controller_state);
                ESP_LOGI(TAG, "");
                
                // Guardar estado actual
                prev_state = controller_state;
                first_read = false;
            }
        } else {
            ESP_LOGE(TAG, "Error al leer el estado del controlador");
        }
        
        // Esperar 50ms antes de la siguiente lectura (más responsivo)
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}
