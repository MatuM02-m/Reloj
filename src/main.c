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

#include "config.h"
#include "digital.h"
#include "bsp.h"
#include "clock.h"
#include "screen.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* === Macros definitions ====================================================================== */

/* === Private data type declarations ========================================================== */

typedef enum {
    CLOCK_MODE_UNSET_TIME,        // Modo para establecer la hora inicial
    CLOCK_MODE_DISPLAY,           // Modo de visualización normal
    CLOCK_MODE_SET_HOURS,         // Modo para establecer horas
    CLOCK_MODE_SET_MINUTES,       // Modo para establecer minutos
    CLOCK_MODE_SET_ALARM_HOURS,   // Modo para establecer horas de la alarma
    CLOCK_MODE_SET_ALARM_MINUTES, // Modo para establecer minutos de la alarma
} clock_mode_t;

typedef enum {
    MSG_BUTTON_SET_TIME_LONG,
    MSG_BUTTON_SET_ALARM_LONG,
    MSG_BUTTON_ACCEPT,
    MSG_BUTTON_CANCEL,
    MSG_BUTTON_INCREASE,
    MSG_BUTTON_DECREASE,
    MSG_CLOCK_TICK,
    MSG_CONFIG_TIMEOUT,
    MSG_UPDATE_DISPLAY
} message_type_t;

typedef struct {
    message_type_t type;
    uint32_t data; // Datos adicionales si son necesarios
} task_message_t;

/* === Private variable declarations =========================================================== */

static board_t board;

static clock_t clock;

// static clock_time_t time;

static clock_time_t time_to_display;

static clock_mode_t clock_mode;

static const uint8_t MINUTES_LIMIT[] = {5, 9};

static const uint8_t HOURS_LIMIT[] = {2, 3};

static uint32_t set_time_press_duration = 0;

static bool set_time_long_pressed = false;

static uint32_t set_alarm_press_duration = 0;

static bool set_alarm_long_pressed = false;

// static bool set_time_long_press_detected = false;

// static bool set_alarm_long_press_detected = false;

static bool alarm_ringing = false;

static uint32_t timeout_count = 0;

static QueueHandle_t main_queue; // Cola para MainTask

static QueueHandle_t display_queue; // Cola para DisplayTask

static QueueHandle_t clock_queue; // Cola para ClockTask

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

/**
 * @brief Tarea principal que maneja la lógica del reloj
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void MainTask(void * pvParameters);

/**
 * @brief Tarea para el manejo del display multiplexado
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void DisplayTask(void * pvParameters);

/**
 * @brief Tarea para el manejo del reloj y timeouts
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void ClockTask(void * pvParameters);

/**
 * @brief Tarea para el manejo de botones y presiones largas
 * @param pvParameters Parámetros de la tarea (no utilizados)
 */
static void ButtonTask(void * pvParameters);

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
        ClockUpdateAlarmVisual(clock, board, alarm_ringing);
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

    SysTickInit(TICKS_PER_SECOND);
    clock = ClockCreate(TICKS_PER_SECOND);
    board = BoardCreate();
    ModeChange(CLOCK_MODE_UNSET_TIME);

    main_queue = xQueueCreate(10, sizeof(task_message_t));
    display_queue = xQueueCreate(5, sizeof(task_message_t));
    clock_queue = xQueueCreate(5, sizeof(task_message_t));

    if (main_queue == NULL || display_queue == NULL || clock_queue == NULL) {
        // Error: no se pudieron crear las colas
        while (1);
    }

    // Crear todas las tareas
    xTaskCreate(DisplayTask, // Tarea de display (alta prioridad)
                "Display",
                128, // Stack pequeño
                NULL,
                3, // Prioridad alta
                NULL);

    xTaskCreate(ClockTask, // Tarea de reloj
                "Clock", 256, NULL,
                2, // Prioridad media
                NULL);

    xTaskCreate(ButtonTask, // Tarea de botones
                "Buttons", 128, NULL,
                2, // Prioridad media
                NULL);

    xTaskCreate(MainTask, // Tarea principal (lógica)
                "MainTask",
                512, // Stack más grande para lógica
                NULL,
                1, // Prioridad baja
                NULL);

    // Iniciar el scheduler de FreeRTOS
    vTaskStartScheduler();

    // Si llegamos aquí, algo salió mal
    while (1);
}

static void DisplayTask(void * pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    task_message_t message;

    while (true) {
        // Siempre refrescar pantalla para multiplexado
        ScreenRefresh(board->screen);

        // Verificar si hay mensajes (sin esperar)
        if (xQueueReceive(display_queue, &message, 0) == pdTRUE) {
            switch (message.type) {
            case MSG_UPDATE_DISPLAY:
                // Actualizar contenido del display en modo normal
                if (clock_mode == CLOCK_MODE_DISPLAY) {
                    uint8_t value[4];
                    clock_time_t current_time;
                    ClockGetTime(clock, &current_time);
                    ClockTimeToBCD(&current_time, value);
                    ScreenWriteBCD(board->screen, value, 4);
                }
                break;

            default:
                break;
            }
        }

        // Refrescar cada 1ms para multiplexado suave
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void ClockTask(void * pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t count = 0;
    task_message_t message;

    while (true) {
        // Incrementar el reloj cada segundo (1000 ticks de 1ms)
        ClockNewTick(clock);
        count++;

        // Verificar timeout de configuración
        if (IsInConfigMode()) {
            timeout_count++;
            if (timeout_count >= CONFIG_TIMEOUT_TICKS) {
                timeout_count = 0;

                // Enviar mensaje de timeout a MainTask
                message.type = MSG_CONFIG_TIMEOUT;
                message.data = 0;
                xQueueSend(main_queue, &message, 0);
            }
        }

        // Actualizar pantalla cada 100ms cuando estamos en modo DISPLAY
        if (clock_mode == CLOCK_MODE_DISPLAY && (count % 100) == 0) {
            // Enviar mensaje para actualizar display
            message.type = MSG_UPDATE_DISPLAY;
            message.data = 0;
            xQueueSend(display_queue, &message, 0);
        }

        // Esperar 1ms
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void ButtonTask(void * pvParameters) {
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    task_message_t message;

    while (true) {
        // Detectar presiones largas
        if (IsLongPress(board->set_time, &set_time_press_duration, &set_time_long_pressed)) {
            message.type = MSG_BUTTON_SET_TIME_LONG;
            message.data = 0;
            xQueueSend(main_queue, &message, 0); // Enviar sin esperar
                                                 
        }

        if (IsLongPress(board->set_alarm, &set_alarm_press_duration, &set_alarm_long_pressed)) {
            message.type = MSG_BUTTON_SET_ALARM_LONG;
            message.data = 0;
            xQueueSend(main_queue, &message, 0);
            
        }

        // Detectar presiones normales de botones
        if (DigitalInputWasActivated(board->accept)) {
            message.type = MSG_BUTTON_ACCEPT;
            message.data = 0;
            xQueueSend(main_queue, &message, 0);
            
        }

        if (DigitalInputWasActivated(board->cancel)) {
            message.type = MSG_BUTTON_CANCEL;
            message.data = 0;
            xQueueSend(main_queue, &message, 0);
            
        }

        if (DigitalInputWasActivated(board->increase)) {
            message.type = MSG_BUTTON_INCREASE;
            message.data = 0;
            xQueueSend(main_queue, &message, 0);
            
        }

        if (DigitalInputWasActivated(board->decrease)) {
            message.type = MSG_BUTTON_DECREASE;
            message.data = 0;
            xQueueSend(main_queue, &message, 0);
            
        }

        // Verificar botones cada 10ms para buena responsividad
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1));
    }
}

static void MainTask(void * pvParameters) {
    // Eliminar el parámetro no utilizado
    (void)pvParameters;

    task_message_t message;
    TickType_t timeout = pdMS_TO_TICKS(50); // Timeout de 50ms para recibir mensajes

    while (true) {
        // Recibir mensaje (esperar hasta 50ms)
        if (xQueueReceive(main_queue, &message, timeout) == pdTRUE) {
            // Procesar mensaje recibido
            switch (message.type) {
            case MSG_BUTTON_SET_TIME_LONG:
                if (clock_mode == CLOCK_MODE_UNSET_TIME || clock_mode == CLOCK_MODE_DISPLAY) {
                    ClockGetTime(clock, &time_to_display);
                    ModeChange(CLOCK_MODE_SET_MINUTES);
                }
                break;

            case MSG_BUTTON_SET_ALARM_LONG:
                if (clock_mode == CLOCK_MODE_UNSET_TIME || clock_mode == CLOCK_MODE_DISPLAY) {
                    ClockGetAlarm(clock, &time_to_display);
                    ModeChange(CLOCK_MODE_SET_ALARM_MINUTES);
                }
                break;

            case MSG_BUTTON_ACCEPT:
                switch (clock_mode) {
                case CLOCK_MODE_DISPLAY:
                    if (alarm_ringing) {
                        ClockPostponeAlarm(clock, 5);
                        alarm_ringing = ClockCheckAlarm(clock);
                    } else {
                        ClockEnableAlarm(clock, true);
                    }
                    break;
                case CLOCK_MODE_SET_HOURS:
                    ResetConfigTimeout();
                    ClockSetTime(clock, &time_to_display);
                    ModeChange(CLOCK_MODE_DISPLAY);
                    break;
                case CLOCK_MODE_SET_MINUTES:
                    ResetConfigTimeout();
                    ClockSetTime(clock, &time_to_display);
                    ModeChange(CLOCK_MODE_SET_HOURS);
                    break;
                case CLOCK_MODE_SET_ALARM_HOURS:
                    ResetConfigTimeout();
                    ClockSetAlarm(clock, &time_to_display);
                    ModeChange(CLOCK_MODE_DISPLAY);
                    break;
                case CLOCK_MODE_SET_ALARM_MINUTES:
                    ResetConfigTimeout();
                    ClockSetAlarm(clock, &time_to_display);
                    ClockEnableAlarm(clock, true);
                    ModeChange(CLOCK_MODE_SET_ALARM_HOURS);
                    break;
                default:
                    break;
                }
                break;

            case MSG_BUTTON_CANCEL:
                switch (clock_mode) {
                case CLOCK_MODE_DISPLAY:
                    if (alarm_ringing) {
                        ClockStopAlarm(clock);
                        ClockEnableAlarm(clock, false);
                        alarm_ringing = false;
                    } else {
                        ClockEnableAlarm(clock, false);
                    }
                    break;
                case CLOCK_MODE_SET_HOURS:
                case CLOCK_MODE_SET_MINUTES:
                    ResetConfigTimeout();
                    {
                        clock_time_t current_time;
                        if (ClockGetTime(clock, &current_time)) {
                            ModeChange(CLOCK_MODE_DISPLAY);
                        } else {
                            ModeChange(CLOCK_MODE_UNSET_TIME);
                        }
                    }
                    break;
                case CLOCK_MODE_SET_ALARM_HOURS:
                case CLOCK_MODE_SET_ALARM_MINUTES:
                    ResetConfigTimeout();
                    ModeChange(CLOCK_MODE_DISPLAY);
                    break;
                default:
                    break;
                }
                break;

            case MSG_BUTTON_INCREASE:
                switch (clock_mode) {
                case CLOCK_MODE_SET_HOURS:
                case CLOCK_MODE_SET_ALARM_HOURS:
                    ResetConfigTimeout();
                    IncreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                    UpdateDisplayContent();
                    break;
                case CLOCK_MODE_SET_MINUTES:
                case CLOCK_MODE_SET_ALARM_MINUTES:
                    ResetConfigTimeout();
                    IncreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                    UpdateDisplayContent();
                    break;
                default:
                    break;
                }
                break;

            case MSG_BUTTON_DECREASE:
                switch (clock_mode) {
                case CLOCK_MODE_SET_HOURS:
                case CLOCK_MODE_SET_ALARM_HOURS:
                    ResetConfigTimeout();
                    DecreaseBCD(time_to_display.time.hours, HOURS_LIMIT);
                    UpdateDisplayContent();
                    break;
                case CLOCK_MODE_SET_MINUTES:
                case CLOCK_MODE_SET_ALARM_MINUTES:
                    ResetConfigTimeout();
                    DecreaseBCD(time_to_display.time.minutes, MINUTES_LIMIT);
                    UpdateDisplayContent();
                    break;
                default:
                    break;
                }
                break;

            case MSG_CONFIG_TIMEOUT: {
                clock_time_t current_time;
                if (ClockGetTime(clock, &current_time)) {
                    ModeChange(CLOCK_MODE_DISPLAY);
                } else {
                    ModeChange(CLOCK_MODE_UNSET_TIME);
                }
            } break;

            default:
                // Mensaje no reconocido
                break;
            }
        }

        // Si estamos en modo DISPLAY, manejar alarma
        if (clock_mode == CLOCK_MODE_DISPLAY) {
            alarm_ringing = ClockCheckAlarm(clock);
            ClockUpdateAlarmVisual(clock, board, alarm_ringing);
        }
    }

    vTaskDelete(NULL);
}

/* === End of documentation ==================================================================== */

/** @} End of module definition for doxygen */
