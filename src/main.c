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

/** @file main.c
 ** @brief Código fuente del módulo principal del reloj
 ** @addtogroup main
 ** @author Milenkovitch, Matías
 **/

/* === Headers files inclusions =============================================================== */

#include "digital.h"
#include "bsp.h"
#include "clock.h"
#include "screen.h"

/* === Macros definitions ====================================================================== */

#define TICKS_PER_SECOND           1000

#define LONG_PRESS_THRESHOLD_TICKS (3 * TICKS_PER_SECOND)

#define CONFIG_TIMEOUT_TICKS       (30 * TICKS_PER_SECOND)

/* === Private data type declarations ========================================================== */

typedef enum {
    CLOCK_MODE_UNSET_TIME,        // Modo para establecer la hora inicial
    CLOCK_MODE_DISPLAY,           // Modo de visualización normal
    CLOCK_MODE_SET_HOURS,         // Modo para establecer horas
    CLOCK_MODE_SET_MINUTES,       // Modo para establecer minutos
    CLOCK_MODE_SET_ALARM_HOURS,   // Modo para establecer horas de la alarma
    CLOCK_MODE_SET_ALARM_MINUTES, // Modo para establecer minutos de la alarma
} clock_mode_t;

/* === Private variable declarations =========================================================== */

static board_t board;

static clock_t clock;

static clock_time_t time;

static clock_time_t time_to_display;

static clock_mode_t clock_mode;

static const uint8_t MINUTES_LIMIT[] = {5, 9};

static const uint8_t HOURS_LIMIT[] = {2, 3};

static uint32_t set_time_press_duration = 0;

static bool set_time_long_pressed = false;

static uint32_t set_alarm_press_duration = 0;

static bool set_alarm_long_pressed = false;

static bool set_time_long_press_detected = false;

static bool set_alarm_long_press_detected = false;

static bool alarm_ringing = false;

static uint32_t timeout_count = 0;

/* === Private function declarations =========================================================== */

/**
 * @brief Función para incrementar un número BCD (Binary-Coded Decimal) con límite.
 *
 * @param numero Puntero al número BCD a incrementar.
 * @param limite Valor límite para el número BCD, representado como un arreglo de dos elementos:
 *              - limite[0]: decena (0-2 para horas, 0-5 para minutos/segundos)
 *              - limite[1]: unidad (0-3 para horas, 0-9 para minutos/segundos)
 */
void IncreaseBCD(uint8_t * numero, const uint8_t limite[2]);

/**
 * @brief Función para decrementar un número BCD (Binary-Coded Decimal) con límite.
 *
 * @param numero Puntero al número BCD a decrementar.
 * @param limite Valor límite para el número BCD, representado como un arreglo de dos elementos:
 *              - limite[0]: decena (0-2 para horas, 0-5 para minutos/segundos)
 *              - limite[1]: unidad (0-3 para horas, 0-9 para minutos/segundos)
 */
void DecreaseBCD(uint8_t * numero, const uint8_t limite[2]);

/**
 * @brief Cambia el modo del reloj y actualiza la pantalla según el nuevo modo.
 *
 * @param mode Nuevo modo del reloj.
 */
void ModeChange(clock_mode_t mode);

/**
 * @brief Actualiza el contenido de la pantalla según el modo actual del reloj.
 *
 * Esta función escribe los valores BCD correspondientes en la pantalla según el modo actual.
 */
void UpdateDisplayContent(void);

/**
 * @brief Verifica si una entrada digital ha sido presionada durante un tiempo largo.
 *
 * @param input Entrada digital a verificar.
 * @param press_duration Puntero al contador de duración de la presión.
 * @param flag Puntero a un flag que indica si se ha detectado una presión larga.
 * @return true Si se detectó una presión larga.
 * @return false Si no se detectó una presión larga.
 */
bool IsLongPress(digital_input_t input, uint32_t * press_duration, bool * flag);

/**
 * @brief Reinicia el contador de tiempo de configuración.
 *
 * Esta función se utiliza para reiniciar el contador que controla el tiempo de configuración del reloj.
 */
void ResetConfigTimeout(void);

/**
 * @brief Verifica si el reloj está en modo de configuración.
 *
 * @return true Si el reloj está en modo de configuración.
 * @return false Si el reloj no está en modo de configuración.
 */
bool IsInConfigMode(void);


/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void IncreaseBCD(uint8_t * numero, const uint8_t limite[2]) {
    bool is_hours = (limite[0] == 2 && limite[1] == 3);

    numero[0]++; // Incrementar unidades
    if (numero[0] > 9) {
        numero[0] = 0;
        numero[1]++; // Incrementar decenas
    }

    if (is_hours) {
        // Para horas: 23 -> 00 (pero 24 nunca debe aparecer)
        if ((numero[1] == 2) && (numero[0] == 4)) {
            numero[0] = 0;
            numero[1] = 0;
        }
    } else {
        // Para minutos/segundos: cuando llega a 60 -> 00
        if ((numero[1] == 6) && (numero[0] == 0)) {
            numero[0] = 0;
            numero[1] = 0;
        }
    }
}

void DecreaseBCD(uint8_t * numero, const uint8_t limite[2]) {
    // Detectar si son horas por el límite
    bool is_hours = (limite[0] == 2 && limite[1] == 3);

    if (numero[0] == 0) {     // Si unidades es 0 (ahora en posición [0])
        if (numero[1] == 0) { // Si decenas es 0 (ahora en posición [1])
            if (is_hours) {
                // CASO ESPECIAL: 00:xx -> 23:xx
                numero[1] = 2; // decenas = 2
                numero[0] = 3; // unidades = 3
            } else {
                // Para minutos: 00 -> 59
                numero[1] = 5; // decenas = 5
                numero[0] = 9; // unidades = 9
            }
        } else {
            // Decrementar decenas y poner unidades a 9
            numero[1]--;
            numero[0] = 9;
        }
    } else {
        // Simplemente decrementar unidades
        numero[0]--;
    }
}

void ModeChange(clock_mode_t actual) {
    clock_mode = actual;

    switch (clock_mode) {
    case CLOCK_MODE_UNSET_TIME:
        ScreenFlashDigits(board->screen, 0, 3, 100);
        ScreenFlashDots(board->screen, 1, 1, 100);
        memset(&time_to_display, 0, sizeof(clock_time_t));
        break;

    case CLOCK_MODE_DISPLAY:
        ScreenFlashDigits(board->screen, 0, 3, 0);
        ScreenFlashDots(board->screen, 1, 1, 500);
        // ClockUpdateAlarmVisual(clock, board, alarm_ringing);
        break;

    case CLOCK_MODE_SET_HOURS:
        ScreenFlashDigits(board->screen, 0, 1, 100);
        ScreenSetDots(board->screen, 1, 1); // Mostrar separador
        break;

    case CLOCK_MODE_SET_MINUTES:
        ScreenFlashDigits(board->screen, 2, 3, 100);
        ScreenSetDots(board->screen, 1, 1); // Mostrar separador
        break;

    case CLOCK_MODE_SET_ALARM_HOURS:
        ScreenFlashDots(board->screen, 0, 0, 0);
        ScreenClearDots(board->screen);
        ScreenSetDots(board->screen, 0, 3); // Todos los puntos para indicar modo alarma
        ScreenFlashDigits(board->screen, 0, 1, 100);
        break;

    case CLOCK_MODE_SET_ALARM_MINUTES:
        ScreenFlashDots(board->screen, 0, 0, 0);
        ScreenClearDots(board->screen);
        ScreenSetDots(board->screen, 0, 3); // Todos los puntos para indicar modo alarma
        ScreenFlashDigits(board->screen, 2, 3, 100);
        break;

    default:
        break;
    }

    // Actualizar el contenido de la pantalla
    UpdateDisplayContent();
}

void UpdateDisplayContent(void) {
    uint8_t value[4] = {0, 0, 0, 0};

    switch (clock_mode) {
    case CLOCK_MODE_DISPLAY:
        ClockGetTime(clock, &time_to_display);
        ClockTimeToBCD(&time_to_display, value);
        break;

    case CLOCK_MODE_SET_HOURS:
        // Mostrar el tiempo que se está editando
        ClockTimeToBCD(&time_to_display, value);
        break;

    case CLOCK_MODE_SET_MINUTES:
        // Mostrar el tiempo que se está editando
        ClockTimeToBCD(&time_to_display, value);
        break;

    case CLOCK_MODE_SET_ALARM_HOURS:
        // Mostrar el tiempo de alarma que se está editando
        ClockTimeToBCD(&time_to_display, value);
        break;

    case CLOCK_MODE_SET_ALARM_MINUTES:
        // Mostrar el tiempo de alarma que se está editando
        ClockTimeToBCD(&time_to_display, value);
        break;

    case CLOCK_MODE_UNSET_TIME:
        // Mostrar "--:--" o tiempo por defecto
        value[0] = 0;
        value[1] = 0;
        value[2] = 0;
        value[3] = 0;
        break;

    default:
        break;
    }

    ScreenWriteBCD(board->screen, value, 4);
}

bool IsLongPress(digital_input_t input, uint32_t * press_duration, bool * flag) {
    if (DigitalInputGetState(input)) {
        (*press_duration)++;
        // Si alcanza el umbral y no se ha detectado antes
        if (*press_duration >= LONG_PRESS_THRESHOLD_TICKS && !(*flag)) {
            *flag = true;
            return true;
        }
    } else {
        // Resetear cuando se suelta la tecla
        *press_duration = 0;
        *flag = false;
    }
    return false;
}

void ResetConfigTimeout(void) {
    timeout_count = 0;
}

bool IsInConfigMode(void) {
    return (clock_mode == CLOCK_MODE_SET_HOURS || clock_mode == CLOCK_MODE_SET_MINUTES ||
            clock_mode == CLOCK_MODE_SET_ALARM_HOURS || clock_mode == CLOCK_MODE_SET_ALARM_MINUTES);
}

/* === Public function implementation ========================================================= */

/**
 * @brief Función pricipal del programa.
 *
 * @return int
 */
int main(void) {

    SysTickInit(TICKS_PER_SECOND);         // Inicializar timer del sistema
    clock = ClockCreate(TICKS_PER_SECOND); // Crear el reloj
    board = BoardCreate();                 // Inicializar la placa

    ModeChange(CLOCK_MODE_UNSET_TIME); // Cambiar al modo de configuración inicial

    while (true) {
        switch (clock_mode) {
        case CLOCK_MODE_UNSET_TIME:
            // Detectar si se completó una presión larga
            if (set_time_long_press_detected) {
                set_time_long_press_detected = false;  // Reset flag
                ClockGetTime(clock, &time_to_display); // Obtener tiempo actual
                ModeChange(CLOCK_MODE_SET_MINUTES);
            } else if (set_alarm_long_press_detected) {
                set_alarm_long_press_detected = false;  // Reset flag
                ClockGetAlarm(clock, &time_to_display); // Obtener alarma actual
                ModeChange(CLOCK_MODE_SET_ALARM_MINUTES);
            }
            break;

        case CLOCK_MODE_DISPLAY:
            // Detectar presiones largas
            if (set_time_long_press_detected) {
                set_time_long_press_detected = false;
                ClockGetTime(clock, &time_to_display);
                ModeChange(CLOCK_MODE_SET_MINUTES);
            } else if (set_alarm_long_press_detected) {
                set_alarm_long_press_detected = false;
                ClockGetAlarm(clock, &time_to_display);
                ModeChange(CLOCK_MODE_SET_ALARM_MINUTES);
            }

            // Actualizar estado de alarma
            alarm_ringing = ClockCheckAlarm(clock);

            if (alarm_ringing) {
                // ALARMA ESTÁ SONANDO
                if (DigitalInputWasActivated(board->accept)) {
                    ClockPostponeAlarm(clock, 5);           // Posponer 5 minutos
                    alarm_ringing = ClockCheckAlarm(clock); // Actualizar estado
                }

                if (DigitalInputWasActivated(board->cancel)) {
                    ClockEnableAlarm(clock, false); // Desactivar alarma
                    alarm_ringing = false;          // Ya no suena
                }
            } else {
                // ALARMA NO ESTÁ SONANDO
                if (DigitalInputWasActivated(board->accept)) {
                    ClockEnableAlarm(clock, true); // Activar alarma
                }

                if (DigitalInputWasActivated(board->cancel)) {
                    ClockEnableAlarm(clock, false); // Desactivar alarma
                }
            }

            // Actualizar estado visual
            ClockUpdateAlarmVisual(clock, board, alarm_ringing);
            break;

        case CLOCK_MODE_SET_HOURS:
            if (DigitalInputWasActivated(board->increase)) {
                ResetConfigTimeout();
                IncreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->decrease)) {
                ResetConfigTimeout();
                DecreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->accept)) {
                ResetConfigTimeout();
                ClockSetTime(clock, &time_to_display);
                ModeChange(CLOCK_MODE_DISPLAY);
            } else if (DigitalInputWasActivated(board->cancel)) {
                ResetConfigTimeout();

                clock_time_t current_time;
                if (ClockGetTime(clock, &current_time)) {
                    ModeChange(CLOCK_MODE_DISPLAY); // Volver a mostrar hora actual
                } else {
                    ModeChange(CLOCK_MODE_UNSET_TIME); // Volver a estado sin configurar
                }
            }
            break;

        case CLOCK_MODE_SET_MINUTES:
            if (DigitalInputWasActivated(board->increase)) {
                ResetConfigTimeout();
                IncreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->decrease)) {
                ResetConfigTimeout();
                DecreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->accept)) {
                ResetConfigTimeout();
                ClockSetTime(clock, &time_to_display);
                ModeChange(CLOCK_MODE_SET_HOURS);
            } else if (DigitalInputWasActivated(board->cancel)) {
                ResetConfigTimeout();

                clock_time_t current_time;
                if (ClockGetTime(clock, &current_time)) {
                    ModeChange(CLOCK_MODE_DISPLAY); // Volver a mostrar hora actual
                } else {
                    ModeChange(CLOCK_MODE_UNSET_TIME); // Volver a estado sin configurar
                }
            }
            break;

        case CLOCK_MODE_SET_ALARM_HOURS:
            if (DigitalInputWasActivated(board->increase)) {
                ResetConfigTimeout();
                IncreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->decrease)) {
                ResetConfigTimeout();
                DecreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->accept)) {
                ResetConfigTimeout();
                ClockSetAlarm(clock, &time_to_display);
                ModeChange(CLOCK_MODE_DISPLAY);
            } else if (DigitalInputWasActivated(board->cancel)) {
                ResetConfigTimeout();
                ModeChange(CLOCK_MODE_DISPLAY);
            }
            break;

        case CLOCK_MODE_SET_ALARM_MINUTES:
            if (DigitalInputWasActivated(board->increase)) {
                ResetConfigTimeout();
                IncreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->decrease)) {
                ResetConfigTimeout();
                DecreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                UpdateDisplayContent();
            } else if (DigitalInputWasActivated(board->accept)) {
                ResetConfigTimeout();
                ClockSetAlarm(clock, &time_to_display);
                ClockEnableAlarm(clock, true);
                ModeChange(CLOCK_MODE_SET_ALARM_HOURS);
            } else if (DigitalInputWasActivated(board->cancel)) {
                ResetConfigTimeout();
                ModeChange(CLOCK_MODE_DISPLAY);
            }
            break;

        default:
            break;
        }

        for (int delay = 0; delay < 25000; delay++) {
            __asm("NOP");
        }
    }
}

void SysTick_Handler(void) {
    static uint32_t count = 0;

    // Siempre refrescar la pantalla (multiplexado)
    ScreenRefresh(board->screen);

    // Incrementar el reloj
    ClockNewTick(clock);

    // Detectar presiones largas y activar flags
    if (IsLongPress(board->set_time, &set_time_press_duration, &set_time_long_pressed)) {
        set_time_long_press_detected = true;
    }
    if (IsLongPress(board->set_alarm, &set_alarm_press_duration, &set_alarm_long_pressed)) {
        set_alarm_long_press_detected = true;
    }

    count++;

    if (IsInConfigMode()) {
        timeout_count++;
        if (timeout_count >= CONFIG_TIMEOUT_TICKS) {
            timeout_count = 0;

            clock_time_t current_time;
            if (ClockGetTime(clock, &current_time)) {
                // Tiene hora válida, volver a mostrarla
                ModeChange(CLOCK_MODE_DISPLAY);
            } else {
                // No tiene hora válida, mantener estado sin configurar
                ModeChange(CLOCK_MODE_UNSET_TIME);
            }
        }
    }

    // Solo actualizar la pantalla cuando estamos en modo DISPLAY
    if (clock_mode == CLOCK_MODE_DISPLAY && (count % 100) == 0) {
        uint8_t value[4];
        ClockGetTime(clock, &time);
        ClockTimeToBCD(&time, value);
        ScreenWriteBCD(board->screen, value, 4);
    }
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
