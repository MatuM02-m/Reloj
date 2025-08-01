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

/**
 * @brief                   Definición de la estructura interna del reloj.
 * @param clock_ticks       Cantidad de ticks del reloj.
 * @param current_time      Tiempo actual del reloj.
 * @param alarm_time        Hora de la alarma.
 * @param alarm_posponed    Hora de la alarma pospuesta.
 * @param alarm_enabled     Indica si la alarma está habilitada.
 * @param valid             Indica si el reloj tiene un tiempo válido.
 *
 */
struct clock_s {
    uint16_t clock_ticks;
    clock_time_t current_time;
    clock_time_t alarm_time;
    clock_time_t alarm_posponed;
    bool alarm_enabled;
    bool valid;
};

/* === Private function declarations =============================================================================== */

/* === Private variable definitions ================================================================================ */

/* === Public variable definitions ================================================================================= */

/* === Private function definitions ================================================================================ */

/* === Public function definitions ============================================================================== */

clock_t ClockCreate(uint16_t ticks_per_seconds) {
    static struct clock_s self[1]; // Si uso malloc, cada test creará uno nuevo
    memset(self, 0, sizeof(struct clock_s));
    self->valid = false;
    self->alarm_enabled = false;
    self->clock_ticks = ticks_per_seconds;
    return self;
}

bool ClockTimeIsValid(clock_time_t * self) {
    // Validar horas: 00-23
    if (self->time.hours[0] > 2) {
        return false; // Decena de horas no puede ser mayor a 2
    }
    if ((self->time.hours[0] == 2) && (self->time.hours[1] > 3)) {
        return false; // Si decena es 2, unidad no puede ser mayor a 3 (máximo 23)
    }
    if (self->time.hours[1] > 9) {
        return false; // Unidad de horas no puede ser mayor a 9
    }

    // Validar minutos: 00-59
    if (self->time.minutes[0] > 5) {
        return false; // Decena de minutos no puede ser mayor a 5
    }
    if (self->time.minutes[1] > 9) {
        return false; // Unidad de minutos no puede ser mayor a 9
    }
    
    // Validar segundos: 00-59
    if (self->time.seconds[0] > 5) {
        return false; // Decena de segundos no puede ser mayor a 5
    }
    if (self->time.seconds[1] > 9) {
        return false; // Unidad de segundos no puede ser mayor a 9
    }
    return true;
}

bool ClockGetTime(clock_t self, clock_time_t * result) {
    memcpy(result, &self->current_time, 6);
    return self->valid;
}

bool ClockSetTime(clock_t self, const clock_time_t * new_time) {
    self->valid = true;
    memcpy(&self->current_time, new_time, sizeof(clock_time_t));
    if (ClockTimeIsValid(new_time)) {
        self->valid = true;
    } else {
        self->valid = false;
    }
    return self->valid;
}

void ClockNewTick(clock_t self) {
    self->clock_ticks++;
    
    if (self->clock_ticks >= 1000) {  // 1000 ticks = 1 segundo
        self->clock_ticks = 0;
        
        // USAR LA MISMA LÓGICA QUE IncreaseBCD()
        // seconds[1] = unidades, seconds[0] = decenas
        
        // Incrementar segundos (unidades)
        self->current_time.time.seconds[1]++;
        if (self->current_time.time.seconds[1] > 9) {
            self->current_time.time.seconds[1] = 0;
            // Incrementar segundos (decenas)
            self->current_time.time.seconds[0]++;
            if (self->current_time.time.seconds[0] > 5) {
                self->current_time.time.seconds[0] = 0;
                
                // Incrementar minutos (unidades)
                self->current_time.time.minutes[1]++;
                if (self->current_time.time.minutes[1] > 9) {
                    self->current_time.time.minutes[1] = 0;
                    // Incrementar minutos (decenas)
                    self->current_time.time.minutes[0]++;
                    if (self->current_time.time.minutes[0] > 5) {
                        self->current_time.time.minutes[0] = 0;
                        
                        // Incrementar horas (unidades)
                        self->current_time.time.hours[1]++;
                        if (self->current_time.time.hours[1] > 9) {
                            self->current_time.time.hours[1] = 0;
                            // Incrementar horas (decenas)
                            self->current_time.time.hours[0]++;
                        }
                        
                        // Verificar límite de 24 horas (23:59 -> 00:00)
                        if ((self->current_time.time.hours[0] == 2) && 
                            (self->current_time.time.hours[1] == 4)) {
                            self->current_time.time.hours[0] = 0;
                            self->current_time.time.hours[1] = 0;
                        }
                    }
                }
            }
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
    if (minutes_postpone == 0) {
        return false; // No hay nada que posponer
    }
    
    // Copiar la hora actual de la alarma
    memcpy(&self->alarm_posponed, &self->alarm_time, sizeof(clock_time_t));
    
    // Convertir minutos BCD a decimal para facilitar el cálculo
    uint8_t total_minutes = (self->alarm_posponed.time.minutes[0] * 10) + 
                           self->alarm_posponed.time.minutes[1] + 
                           minutes_postpone;
    
    uint8_t total_hours = (self->alarm_posponed.time.hours[0] * 10) + 
                         self->alarm_posponed.time.hours[1];
    
    // Manejar overflow de minutos
    while (total_minutes >= 60) {
        total_minutes -= 60;
        total_hours++;
    }
    
    // Manejar overflow de horas (formato 24h)
    if (total_hours >= 24) {
        total_hours = total_hours % 24;
    }
    
    // Convertir de vuelta a BCD
    self->alarm_posponed.time.minutes[0] = total_minutes / 10;
    self->alarm_posponed.time.minutes[1] = total_minutes % 10;
    self->alarm_posponed.time.hours[0] = total_hours / 10;
    self->alarm_posponed.time.hours[1] = total_hours % 10;
    
    // Establecer la nueva alarma
    ClockSetAlarm(self, &self->alarm_posponed);
    return true;
}

void ClockTimeToBCD(clock_time_t * self, uint8_t * value) {
    value[0] = self->time.hours[1];
    value[1] = self->time.hours[0];  
    value[2] = self->time.minutes[1];
    value[3] = self->time.minutes[0];
}

bool ClockGetAlarm(clock_t self, clock_time_t *alarm_time) {
    bool result = false;
    if ((self) && (alarm_time)) {
        memcpy(alarm_time, &self->alarm_time, sizeof(clock_time_t));
        result = true;
    }
    return result;
}

/* === End of documentation ======================================================================================== */
