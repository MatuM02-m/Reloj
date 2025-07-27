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

#ifndef CLOCK_H_
#define CLOCK_H_

/** @file clock.h
 ** @brief Librería para la gestión de un reloj
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

/**
 * @brief Estructura que representa el tiempo del reloj en formato BCD.
 * La estructura contiene campos para los segundos, minutos y horas, cada uno representado por dos bytes.
 */
typedef union {
    struct {
        uint8_t seconds[2];
        uint8_t minutes[2];
        uint8_t hours[2];
    } time;
    uint8_t bcd[6];
} clock_time_t;

/**
 * @brief Estructura que representa un reloj.
 */
typedef struct clock_s * clock_t;

/* === Public variable declarations =============================================================================== */

/* === Public function declarations =============================================================================== */

/**
 * @brief                   Función para crear un reloj.
 * @param ticks_per_seconds Cantidad de ticks por segundo para el reloj.
 * @return                  El reloj creado.
 */
clock_t ClockCreate(uint16_t ticks_per_seconds);

/**
 * @brief           Verifica si el tiempo del reloj es válido.
 * @param new_time  Estructura que contiene el nuevo tiempo a verificar.
 * @return          true si el tiempo es válido, false en caso contrario.
 */
bool ClockTimeIsValid(clock_time_t * new_time);

/**
 * @brief        Obtiene el tiempo actual del reloj.
 * @param clock  El reloj del cual obtener el tiempo.
 * @param result Puntero donde se almacenará el tiempo actual.
 * @return       true si se obtuvo el tiempo correctamente, false en caso contrario.
 */
bool ClockGetTime(clock_t clock, clock_time_t * result);

/**
 * @brief           Establece el tiempo del reloj.
 * @param clock     El reloj donde se establecerá el nuevo tiempo.
 * @param new_time  Puntero al nuevo tiempo a establecer.
 * @return          true si se estableció el tiempo correctamente, false en caso contrario.
 */
bool ClockSetTime(clock_t clock, const clock_time_t * new_time);

/**
 * @brief       Simula un tick del reloj.
 * @param clock El reloj al que se le simulará el tick.
 */
void ClockNewTick(clock_t clock);

/**
 * @brief           Habilita o deshabilita la alarma del reloj.
 * @param clock     El reloj al que se le habilitará o deshabilitará la alarma.
 * @param enable    true para habilitar la alarma, false para deshabilitarla.
 * @return          true si se cambió el estado de la alarma correctamente, false en caso contrario.
 */
bool ClockEnableAlarm(clock_t clock, bool enable);

/**
 * @brief               Establece la hora de la alarma.
 * @param clock         El reloj al que se le establecerá la alarma.
 * @param alarm_time    Puntero al tiempo de la alarma a establecer.
 * @return              true si se estableció la alarma correctamente, false en caso contrario.
 */
bool ClockSetAlarm(clock_t clock, const clock_time_t * alarm_time);

/**
 * @brief           Comprueba si la alarma del reloj ha sonado.
 * @param clock     El reloj a verificar.
 * @return          true si la alarma ha sonado, false en caso contrario.
 */
bool ClockCheckAlarm(clock_t clock);

/**
 * @brief           Pospone la alarma del reloj por una cantidad de minutos.
 * @param clock     El reloj al que se le pospondrá la alarma.
 * @param minutes   Cantidad de minutos para posponer la alarma.
 * @return          true si se pospuso la alarma correctamente, false en caso contrario.
 */
bool ClockPostponeAlarm(clock_t clock, uint16_t minutes);

void ClockTimeToBCD(clock_time_t * self, uint8_t * value);

/* === End of conditional blocks ================================================================================== */

#ifdef __cplusplus
}
#endif

#endif /* CLOCK_H_ */
