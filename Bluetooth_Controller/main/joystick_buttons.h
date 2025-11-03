#ifndef JOYSTICK_BUTTONS_H
#define JOYSTICK_BUTTONS_H

#include <stdint.h>
#include <stdbool.h>

// Definiciones de GPIO para botones
#define BUTTON_A_GPIO        32
#define BUTTON_B_GPIO        33
#define BUTTON_START_GPIO    25
#define BUTTON_SELECT_GPIO   26

// Definiciones de GPIO para joystick (ADC)
#define JOYSTICK_VRX_GPIO    34
#define JOYSTICK_VRY_GPIO    35

// Estructura para almacenar el estado del joystick
typedef struct {
    int16_t x;  // Valor del eje X (VRX) - rango típico: 0-4095
    int16_t y;  // Valor del eje Y (VRY) - rango típico: 0-4095
} joystick_data_t;

// Estructura para almacenar el estado de los botones
typedef struct {
    bool button_a;
    bool button_b;
    bool button_start;
    bool button_select;
} buttons_state_t;

// Estructura para almacenar todo el estado del controlador
typedef struct {
    joystick_data_t joystick;
    buttons_state_t buttons;
} controller_state_t;

/**
 * @brief Inicializa el joystick y los botones
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
bool joystick_buttons_init(void);

/**
 * @brief Lee el estado actual del joystick
 * @param joystick Puntero a estructura donde se guardarán los valores
 * @return true si la lectura fue exitosa
 */
bool joystick_read(joystick_data_t *joystick);

/**
 * @brief Lee el estado actual de todos los botones
 * @param buttons Puntero a estructura donde se guardará el estado
 * @return true si la lectura fue exitosa
 */
bool buttons_read(buttons_state_t *buttons);

/**
 * @brief Lee el estado completo del controlador (joystick + botones)
 * @param state Puntero a estructura donde se guardará el estado completo
 * @return true si la lectura fue exitosa
 */
bool controller_read(controller_state_t *state);

/**
 * @brief Imprime el estado del joystick en el monitor
 * @param joystick Puntero a la estructura del joystick
 */
void joystick_print(joystick_data_t *joystick);

/**
 * @brief Imprime el estado de los botones en el monitor
 * @param buttons Puntero a la estructura de botones
 */
void buttons_print(buttons_state_t *buttons);

/**
 * @brief Imprime el estado completo del controlador en el monitor
 * @param state Puntero a la estructura del controlador
 */
void controller_print(controller_state_t *state);

#endif // JOYSTICK_BUTTONS_H

