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

#ifndef SCREEN_H_
#define SCREEN_H_

/** @file screen.h
 ** @brief Declaraciones del módulo para la gestión de una pantalla multiplexada de 7 segmentos
 **/

/* === Headers files inclusions =================================================================================== */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* === Header for C++ compatibility =============================================================================== */

#ifdef __cplusplus
extern "C" {
#endif

/* === Public macros definitions ================================================================================== */

#define SEGMENT_A (1 << 0)
#define SEGMENT_B (1 << 1)
#define SEGMENT_C (1 << 2)
#define SEGMENT_D (1 << 3)
#define SEGMENT_E (1 << 4)
#define SEGMENT_F (1 << 5)
#define SEGMENT_G (1 << 6)
#define SEGMENT_P (1 << 7)

/* === Public data type declarations ============================================================================== */

/**
 * @brief   Estructura que representa una pantalla multiplexada de 7 segmentos
 */
typedef struct screen_s * screen_t;

/**
 * @brief   Puntero a una función que apaga los dígitos de la pantalla
 */
typedef void (*digits_turn_off_t)(void);

/**
 * @brief   Puntero a una función que actualiza los segmentos de la pantalla
 *
 * @param   value  Valor a mostrar en los segmentos
 */
typedef void (*segments_update_t)(uint8_t);

/**
 * @brief   Puntero a una función que enciende un dígito de la pantalla
 *
 * @param   digit  Número del dígito a encender (0 a 3)
 */
typedef void (*digits_turn_on_t)(uint8_t digit);

/**
 * @brief   Estructura que representa el controlador de la pantalla multiplexada de 7 segmentos
 */
typedef struct screen_driver_s {
    digits_turn_off_t DigitsTurnOff;
    segments_update_t SegmentsUpdate;
    digits_turn_on_t DigitsTurnOn;
} const * screen_driver_t;

/* === Public variable declarations =============================================================================== */

/* === Public function declarations =============================================================================== */

/**
 * @brief   Función para crear una pantalla multiplexada de 7 segmentos
 *
 * @param   digits  Número de dígitos de la pantalla
 * @param   driver  Estructura que representa el controlador de la pantalla
 * @return          Estructura que representa la pantalla
 */
screen_t ScreenCreate(uint8_t digits, screen_driver_t driver);

/**
 * @brief   Función para escribir una pantalla multiplexada de 7 segmentos
 *
 * @param   screen Estructura que representa la pantalla
 * @param   value  Valor a escribir en la pantalla
 * @param   size   Tamaño del valor a escribir
 */
void ScreenWriteBCD(screen_t screen, uint8_t value[], uint8_t size);

/**
 * @brief   Función para refrescar la pantalla multiplexada de 7 segmentos
 *
 * @param   screen  Estructura que representa la pantalla
 */
void ScreenRefresh(screen_t screen);

/**
 * @brief   Función para hacer parpadear algunos dígitos de la pantalla multiplexada de 7 segmentos
 *
 * @param   screen      Puntero a la estructura que representa la pantalla
 * @param   from        Posición del primer dígito que se quiere hacer parpadear
 * @param   to          Posición del último dígito que se quiere hacer parpadear
 * @param   frecuency   Tiempo que debe estar encendido cada dígito
 * @return              0 si se ha realizado correctamente, -1 si no se ha podido realizar|
 */
int ScreenFlashDigits(screen_t screen, uint8_t from, uint8_t to, uint16_t frecuency);

/**
 * @brief   Función para hacer parpadear los puntos decimales de la pantalla multiplexada de 7 segmentos
 *
 * @param   screen      Puntero a la estructura que representa la pantalla
 * @param   from        Posición del primer dígito que se quiere hacer parpadear
 * @param   to          Posición del último dígito que se quiere hacer parpadear
 * @param   frecuency   Tiempo que debe estar encendido cada punto decimal
 * @return              0 si se ha realizado correctamente, -1 si no se ha podido realizar|
 */
int ScreenFlashDots(screen_t screen, uint8_t from, uint8_t to, uint16_t frecuency);

int ScreenClearDots(screen_t screen);

int ScreenSetDots(screen_t screen, uint8_t from, uint8_t to);

/* === End of conditional blocks ================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* SCREEN_H_ */
