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

#define TICKS_PER_SECOND 1000

/* === Private data type declarations ========================================================== */

typedef enum {
    CLOCK_MODE_UNSET_TIME,
    CLOCK_MODE_DISPLAY,
    CLOCK_MODE_SET_HOURS,
    CLOCK_MODE_SET_MINUTES,
    CLOCK_MODE_SET_ALARM_HOURS,
    CLOCK_MODE_SET_ALARM_MINUTES,
} clock_mode_t;

/* === Private variable declarations =========================================================== */

static board_t board;

static clock_t clock;

static clock_time_t time;

static clock_mode_t clock_mode;

static const uint8_t MINUTES_LIMIT[] = {5, 9};

static const uint8_t HOURS_LIMIT[] = {2, 3};

/* === Private function declarations =========================================================== */

void IncreaseBCD(uint8_t * numero[2], const uint8_t limite[2]);

void DecreaseBCD(uint8_t * numero[2], const uint8_t limite[2]);

void ModeChange(clock_mode_t mode);

/* === Public variable definitions ============================================================= */

/* === Private variable definitions ============================================================ */

/* === Private function implementation ========================================================= */

void IncreaseBCD(uint8_t * numero[2], const uint8_t limite[2]) {
    numero[1]++;
    if (numero[1] > 9) {
        numero[1] = 0;
        numero[0]++;
    }
    if ((numero[0] == limite[0]) && (numero[1] == limite[1])) {
            numero[0] = 0;
            numero[1] = 0;
    }
}

void DecreaseBCD(uint8_t * numero[2], const uint8_t limite[2]) {
    numero[1]--;
    if (numero[1] > 9) {
        numero[1] = 9;
        numero[0]--;
    }
    if ((numero [0] == limite[0]) && (numero[1] > limite[1])) {
        numero[0] = 0;
        numero[1] = 0;
    }
    
}

void ModeChange(clock_mode_t actual) {
    clock_mode = actual;
    switch (clock_mode)
    {
    case CLOCK_MODE_UNSET_TIME:
        ScreenFlashDigits(board->screen, 0, 3, 100);
        break;
    
    case CLOCK_MODE_DISPLAY:
        ScreenFlashDigits(board->screen, 0, 3, 0);
        ScreenFlashDots(board->screen, 1, 1, 500);
        break;

    case CLOCK_MODE_SET_HOURS:
        ScreenFlashDigits(board->screen, 0, 1, 100);
        break;
    
    case CLOCK_MODE_SET_MINUTES:
        ScreenFlashDigits(board->screen, 2, 3, 100);
        break;
    
    case CLOCK_MODE_SET_ALARM_HOURS:
        ScreenSetDots(board->screen, 0, 3);
        ScreenFlashDigits(board->screen, 0, 1, 100);
        break;

    case CLOCK_MODE_SET_ALARM_MINUTES:
        ScreenSetDots(board->screen, 0, 3);
        ScreenFlashDigits(board->screen, 2, 3, 100);
        break;

    default:
        break;
    }
}

/* === Public function implementation ========================================================= */

/**
 * @brief Función pricipal del programa.
 *
 * @return int
 */
int main(void) {

    SysTickInit(TICKS_PER_SECOND);
    clock = ClockCreate(TICKS_PER_SECOND);
    board = BoardCreate();
    uint8_t value[4];
    
    ModeChange(CLOCK_MODE_UNSET_TIME);

    while (true) {
        switch (clock_mode)
        {
        
        case CLOCK_MODE_UNSET_TIME:
            if (DigitalInputRead(board->set_time)) {
                ModeChange(CLOCK_MODE_SET_HOURS);
            }
            break;

        case CLOCK_MODE_DISPLAY:
            ClockGetTime(clock, &time);
            if (ClockTimeIsValid(&time))
            {
                ClockTimeToBCD(&time, value);
                ScreenWriteBCD(board->screen, value, 4);
                ScreenFlashDots(board->screen, 1, 1, 500);
            }
            else
            {
                ModeChange(CLOCK_MODE_UNSET_TIME);
            }
            if (DigitalInputWasActivated(board->set_time)) {
                ModeChange(CLOCK_MODE_SET_ALARM_HOURS);
            }            
            break;
        
        case CLOCK_MODE_SET_HOURS:
            if (DigitalInputRead(board->increase)) {
                IncreaseBCD(time.time.hours, HOURS_LIMIT);
                ClockSetTime(clock, &time);
            } else if (DigitalInputRead(board->decrease)) {
                DecreaseBCD(time.time.hours, HOURS_LIMIT);
                ClockSetTime(clock, &time);
            } else if (DigitalInputRead(board->accept)) {
                ModeChange(CLOCK_MODE_SET_MINUTES);
            } else if (DigitalInputRead(board->cancel)) {
                ModeChange(CLOCK_MODE_UNSET_TIME);
            }
            ClockTimeToBCD(&time, value);
            ScreenWriteBCD(board->screen, value, 4);
            break;
        
        case CLOCK_MODE_SET_ALARM_HOURS:

            break;

        case CLOCK_MODE_SET_ALARM_MINUTES:

            break;

        default:
            break;
        }

        for (int delay = 0; delay < 25000; delay++) {
            __asm("NOP");
        }
        // }
    }
}

void SysTick_Handler(void) {
    static uint32_t count = 0;
    uint8_t value[4];

    ScreenRefresh(board->screen);
    ClockNewTick(clock);

    count = (count + 1) % 1000;
    if (clock_mode == CLOCK_MODE_DISPLAY) {
        ClockGetTime(clock, &time);
        ClockTimeToBCD(&time, value);
        ScreenWriteBCD(board->screen, value, 4);
        if (count > 500) {
            ScreenFlashDots(board->screen, 1, 1, 10);
        }
    } 
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
