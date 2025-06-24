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
    clock_time_t alarm_time;
    clock_time_t alarm_posponed;
    bool alarm_enabled;
    bool valid;
};

/* === Private function declarations =============================================================================== */

clock_time_t ConvertTime(uint8_t time);

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

clock_time_t ConvertTime(uint8_t time) {
    clock_time_t converted_time = {0};
    converted_time.time.minutes[0] = time % 10;       // Unidades de minutos
    converted_time.time.minutes[1] = (time / 10) % 6; // Decenas de minutos
    return converted_time;
}

/* === Public function definitions ============================================================================== */

clock_t ClockCreate(uint16_t ticks_per_seconds) {
    static struct clock_s self[1]; // Si uso malloc, cada test creará uno nuevo
    memset(self, 0, sizeof(struct clock_s));
    self->valid = false;
    (void)ticks_per_seconds;
    return self;
}

bool ClockTimeIsValid(clock_t clock) {
    (void)clock;
    return false;
}

bool ClockGetTime(clock_t self, clock_time_t * result) {
    memcpy(result, &self->current_time, 6);
    return self->valid;
}

bool ClockSetTime(clock_t self, const clock_time_t * new_time) {
    self->valid = true;
    memcpy(&self->current_time, new_time, sizeof(clock_time_t));
    (void)new_time;
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
        } else if (self->current_time.time.seconds[1] > 5) {
            self->current_time.time.seconds[1] = 0;
            self->current_time.time.minutes[0]++;
        } else if (self->current_time.time.minutes[0] > 9) {
            self->current_time.time.minutes[0] = 0;
            self->current_time.time.minutes[1]++;
        } else if (self->current_time.time.minutes[1] > 5) {
            self->current_time.time.minutes[1] = 0;
            self->current_time.time.hours[0]++;
        } else if (self->current_time.time.hours[0] > 9 && self->current_time.time.hours[1] < 2) {
            self->current_time.time.hours[0] = 0;
            self->current_time.time.hours[1]++;
        } else if (self->current_time.time.hours[1] > 2 && self->current_time.time.hours[0] > 3) {
            self->current_time.time.hours[0] = 0;
            self->current_time.time.hours[1] = 0;
        }
    }
}

bool ClockEnableAlarm(clock_t self, bool enable) {
    self->alarm_enabled = enable;
    return self->alarm_enabled;
}

bool ClockSetAlarm(clock_t self, const clock_time_t * new_alarm_time) {
    // bool result = false;
    memcpy(&self->alarm_time, new_alarm_time, sizeof(clock_time_t));
    return true;
}

bool ClockCheckAlarm(clock_t self) {
    if (self->alarm_enabled) {
        if (memcmp(&self->current_time, &self->alarm_time, sizeof(clock_time_t)) == 0) {
            return true; // Alarma sonó
        }
    }
    return false;
}

bool ClockPostponeAlarm(clock_t self, uint16_t minutes_postpone) {
    memcpy(&self->alarm_posponed, &self->alarm_time, sizeof(clock_time_t));
    if (minutes_postpone == 0) {
        return false; // No hay nada que posponer
    } else {
        self->alarm_posponed.time.minutes[0] += minutes_postpone % 10;
        self->alarm_posponed.time.minutes[1] += (minutes_postpone / 10) % 6;
        if (self->alarm_posponed.time.minutes[0] > 9) {
            self->alarm_posponed.time.minutes[0] = 0;
            self->alarm_posponed.time.minutes[1]++;
        } else if (self->alarm_posponed.time.minutes[1] > 5) {
            self->alarm_posponed.time.minutes[1] = 0;
            self->alarm_posponed.time.hours[0]++;
        } else if (self->alarm_posponed.time.hours[0] > 9 && self->alarm_posponed.time.hours[1] < 2) {
            self->alarm_posponed.time.hours[0] = 0;
            self->alarm_posponed.time.hours[1]++;
        } else if (self->alarm_posponed.time.hours[1] > 2 && self->alarm_posponed.time.hours[0] > 3) {
            self->alarm_posponed.time.hours[0] = 0;
            self->alarm_posponed.time.hours[1] = 0;
        }
        ClockSetAlarm(self, &self->alarm_posponed);
        return true;
    }
}

/* === End of documentation ======================================================================================== */
