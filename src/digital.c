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

/** @file digital.c
 ** @brief Código fuente del módulo para la gestión de entradas y salidas digitales
 **/

/* === Headers files inclusions ==================================================================================== */

#include "config.h"
#include "digital.h"
#include "chip.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* === Macros definitions ========================================================================================== */

/* === Private data type declarations ============================================================================== */

/*! Estructura que representa una salida digital */
struct digital_output_s {
    uint8_t port;   /*!< Puerto al que pertenece la salida */
    uint8_t pin;    /*!< Pin al que pertenece la salida */
    bool estado;    /*!< Estado de la salida */
};

/* === Private function declarations =============================================================================== */

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

/* === Public function definitions ============================================================================== */

digital_output_t DigitalOutputCreate(uint8_t port, uint8_t pin) {
    digital_output_t self = malloc(sizeof(struct digital_output_s));
    if (self != NULL) {
        self->port = port;
        self->pin = pin;
    }
    Chip_GPIO_SetPinState(LPC_GPIO_PORT, self->port, self->pin, false);
    Chip_GPIO_SetPinDIR(LPC_GPIO_PORT, self->port, self->pin, true);
    return self;
}

void DigitalOutputActivate(digital_output_t self) {
   Chip_GPIO_SetPinState(LPC_GPIO_PORT, self->port, self->pin, true);
}

void DigitalOutputDeactivate(digital_output_t self) {
   Chip_GPIO_SetPinState(LPC_GPIO_PORT, self->port, self->pin, false);
}

void DigitalOutputToggle(digital_output_t self) {
   Chip_GPIO_SetPinToggle(LPC_GPIO_PORT, self->port, self->pin);
}

/* === End of documentation ======================================================================================== */
