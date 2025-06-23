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

/** @file clock.c
 ** @brief Código fuente del módulo para la gestión de un reloj
 **/

/* === Headers files inclusions ==================================================================================== */

#include "clock.h"
#include <stddef.h>
#include <string.h>

/* === Macros definitions ========================================================================================== */

/* === Private data type declarations ============================================================================== */

struct clock_s {
    uint16_t clock_ticks;
    clock_time_t current_time;
    bool valid;
};

/* === Private function declarations =============================================================================== */

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

/* === Public function definitions ============================================================================== */

clock_t ClockCreate(uint16_t ticks_per_seconds) {
    static struct clock_s self[1];    // Si uso malloc, cada test creará uno nuevo
    memset(self, 0, sizeof(struct clock_s));
    self->valid = false;
    (void) ticks_per_seconds;
    return self;
}

bool ClockTimeIsValid(clock_t clock) {
    (void) clock;
    return false;
}

bool ClockGetTime(clock_t self, clock_time_t * result) {
    memcpy(result, &self->current_time, 6);
    return self->valid;
}

bool ClockSetTime(clock_t self, const clock_time_t * new_time) {
    self->valid = true;
    memcpy(&self->current_time, new_time, sizeof(clock_time_t));
    (void) new_time;
    return true;
}

void ClockNewTick(clock_t self) {
    self->clock_ticks++;
    if (self->clock_ticks == 5) {
        self->clock_ticks = 0;
        self->current_time.time.seconds[0]++;
        if (self->current_time.time.seconds[0] > 9) {
            self->current_time.time.seconds[0] = 0;
            self->current_time.time.seconds[1] = 1;
        }
    }
}

/* === End of documentation ======================================================================================== */
