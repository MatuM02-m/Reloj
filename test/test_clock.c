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

/** @file test_clock.c
 ** @brief Código fuente del módulo para las pruebas del módulo de reloj
 **/

/* === Headers files inclusions =============================================================== */

#include "unity.h"
#include "clock.h"

/**
 - Al inicializar el reloj está en 00:00 y con hora invalida.
 - Al ajustar la hora el reloj queda en hora y es válida.
 - Después de n ciclos de reloj la hora avanza un segundo, diez segundos, un minutos, diez minutos,
  una hora, diez horas y un día completo.
 - Fijar la hora de la alarma y consultarla.
 - Fijar la alarma y avanzar el reloj para que suene.
 - Fijar la alarma, deshabilitarla y avanzar el reloj para no
suene.
 - Hacer sonar la alarma y posponerla.
 - Hacer sonar la alarma y cancelarla hasta el otro dia.
 - Tratar de ajustar la hora del reloj con valores inválidos y verificar que los rechaza...
 - Hacer una prueba con frecuencia de reloj diferente
 *
 */

/* === Macros definitions ====================================================================== */

#define CLOCK_TICKS_PER_SECOND 5
#define TEST_ASSERT_TIME(hours_tens, hours_units, minutes_tens, minutes_units, seconds_tens, seconds_units, current_time) \
    clock_time_t current_time = {0}; \
    TEST_ASSERT_TRUE_MESSAGE(ClockGetTime(clock, &current_time), "Clock has invalid time"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(seconds_units, current_time.bcd[0], "Diference in unit seconds"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(seconds_tens, current_time.bcd[1], "Diference in tens seconds"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(minutes_units, current_time.bcd[2], "Diference in unit minutes"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(minutes_tens, current_time.bcd[3], "Diference in tens minutes"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(hours_units, current_time.bcd[4], "Diference in unit hours"); \
    TEST_ASSERT_EQUAL_UINT8_MESSAGE(hours_tens, current_time.bcd[5], "Diference in tens hours")

/* === Private data type declarations ========================================================== */

static void SimulateSeconds(clock_t clock, uint8_t seconds);

/* === Private variable declarations =========================================================== */

/* === Private function declarations =========================================================== */

static void SimulateSeconds(clock_t clock, uint8_t seconds) {
    for (uint8_t i = 0; i < CLOCK_TICKS_PER_SECOND * seconds; i++)
    {
        ClockNewTick(clock);
    }
}

/* === Public variable definitions ============================================================= */

clock_t clock;

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

/* === Public function implementation ========================================================= */

void setUp(void) {
    clock = ClockCreate(CLOCK_TICKS_PER_SECOND);
}

// Al inicializar el reloj está en 00:00 y con hora invalida.
void test_set_up_with_invalid_time(void) {
    clock_time_t current_time = {.bcd = {1, 2, 3, 4, 5, 6}};

    clock_t clock = ClockCreate(CLOCK_TICKS_PER_SECOND);
    TEST_ASSERT_FALSE(ClockTimeIsValid(clock));
    TEST_ASSERT_FALSE(ClockGetTime(clock, &current_time));
    TEST_ASSERT_EACH_EQUAL_UINT8(0, current_time.bcd, 6);
}

// Al ajustar la hora el reloj queda en hora y es valida
void test_set_up_with_valid_time(void) {
    static const clock_time_t new_time = {
        .time = {
            .seconds = {2, 5},
            .minutes = {3, 0},
            .hours = {4, 1},
        }
    };
    //clock_time_t current_time = {0};

    TEST_ASSERT_TRUE(ClockSetTime(clock, &new_time));
    //TEST_ASSERT_TRUE(ClockGetTime(clock, &current_time));

    TEST_ASSERT_TIME(1, 4, 0, 3, 5, 2, current_time);
}

// Después de n ciclos de reloj la hora avanza un segundo
void test_clock_advance_one_second(void) {
    //clock_time_t current_time = {0};

    // Seteo tiempo inicial en 00:00:00
    ClockSetTime(clock, &(clock_time_t){0});
    SimulateSeconds(clock, 1);
    //ClockGetTime(clock, &current_time);
    TEST_ASSERT_TIME(0, 0, 0, 0, 0, 1, current_time);
}

// Después de n ciclos de reloj la hora avanza un minuto
void test_clock_advance_ten_second(void) {
    // Seteo tiempo inicial en 00:00:00
    ClockSetTime(clock, &(clock_time_t){0});
    SimulateSeconds(clock, 10);
    TEST_ASSERT_TIME(0, 0, 0, 0, 1, 0, current_time);
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
