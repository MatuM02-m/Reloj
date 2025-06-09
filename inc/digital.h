/*********************************************************************************************************************
Copyright (c) 2025, Matías Milenkovitch <matiasmilenko02@gmail.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

SPDX-License-Identifier: MIT
*********************************************************************************************************************/

#ifndef DIGITAL_H_
#define DIGITAL_H_

/** @file digital.h
 ** @brief Declaraciones del módulo para la gestión de entradas y salidas digitales
 **/

/* === Headers files inclusions =================================================================================== */

#include <stdint.h>
#include <stdbool.h>

/* === Header for C++ compatibility =============================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions ================================================================================== */

/* === Public data type declarations ============================================================================== */

//! Estructura que representa una salida digital
typedef struct digital_output_s * digital_output_t;

//! Estructura que representa una entrada digital
typedef struct digital_input_s * digital_input_t;

typedef enum digital_states_e {
    DIGITAL_INPUT_WAS_DEACTIVATED = -1,
    DIGITAL_INPUT_NO_CHANGE = 0,
    DIGITAL_INPUT_WAS_ACTIVATED = 1,
} digital_states_t;

/* === Public variable declarations =============================================================================== */

/* === Public function declarations =============================================================================== */

/**
 * @brief   Función para crear una salida digital
 *
 * @param port  Puerto de la salida digital
 * @param pin   Pin de la salida digital
 * @param active_high  Indica si la salida es activa en bajo (false) o en alto (true)
 * @return      Estructura que representa la salida digital
*/
digital_output_t DigitalOutputCreate(uint8_t port, uint8_t pin, bool active_high);

/**
 * @brief   Función para activar una salida digital
 *
 * @param self  Estructura que representa la salida digital
*/
void DigitalOutputActivate(digital_output_t self);

/**
 * @brief   Función para desactivar una salida digital
 *
 * @param self  Estructura que representa la salida digital
*/
void DigitalOutputDeactivate(digital_output_t self);

/**
 * @brief   Función para alternar el estado de una salida digital
 *
 * @param self  Estructura que representa la salida digital
*/
void DigitalOutputToggle(digital_output_t self);

/**
    * @brief   Función para crear una entrada digital
    *
    * @param port  Puerto de la entrada digital
    * @param pin   Pin de la entrada digital
    * @param inverted Indica si la entrada está invertida
    * @note   Si inverted es true, la entrada se considera activa cuando el pin está en estado bajo
    * @return      Estructura que representa la entrada digital
    */
digital_input_t DigitalInputCreate(uint8_t port, uint8_t pin, bool inverted);

/**
 * @brief   Función para leer el estado de una entrada digital
 *
 * @param input  Estructura que representa la entrada digital
 * @return       Estado de la entrada digital
*/
bool DigitalInputGetState(digital_input_t input);

/**
 * @brief   Función para saber si una entrada digital fue activada
 *
 * @param input  Estructura que representa la entrada digital
 * @return       Estado de la entrada digital
*/
bool DigitalInputWasActivated(digital_input_t input);

/**
 * @brief   Función para saber si una entrada digital fue desactivada
 *
 * @param input  Estructura que representa la entrada digital
 * @return       Estado de la entrada digital
*/
bool DigitalInputWasDeactivated(digital_input_t input);

/**
 * @brief   Función para saber si una entrada digital cambió de estado
 *
 * @param input  Estructura que representa la entrada digital
 * @return       Estado de la entrada digital
*/
enum digital_states_t DigitalWasChanged(digital_input_t input);

/* === End of conditional blocks ================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* DIGITAL_H_ */
