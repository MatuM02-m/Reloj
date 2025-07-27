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

#ifndef BSP_H_
#define BSP_H_

/** @file bsp.h
 ** @brief Declaraciones del módulo para la gestión de placas
 **/

/* === Headers files inclusions =================================================================================== */

#include <stdint.h>
#include "digital.h"
#include "screen.h"

/* === Header for C++ compatibility =============================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions ================================================================================== */

/* === Public data type declarations ============================================================================== */

//! Estructura que representa una placa
typedef struct board_s {
    digital_output_t buzzer;
    digital_output_t led_red;
    digital_output_t led_green;
    digital_output_t led_blue;
    digital_input_t set_time;
    digital_input_t set_alarm;
    digital_input_t increase;
    digital_input_t decrease;
    digital_input_t accept;
    digital_input_t cancel;
    screen_t screen;
} * board_t;

/* === Public variable declarations =============================================================================== */

/* === Public function declarations =============================================================================== */

/**
 * @brief   Función para crear una placa
 *
 * @return      Estructura que representa la placa
*/
board_t BoardCreate();

void SysTickInit(uint32_t ticks);

/* === End of conditional blocks ================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* BSP_H_ */
