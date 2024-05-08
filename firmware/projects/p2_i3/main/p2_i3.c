/*! @mainpage Blinking
 *
 * \section genDesc General Description
 *
 * This example makes LED_1, LED_2 and LED_3 blink at different rates, using FreeRTOS tasks.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Gabriel Villaverde (gabrielggv97@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "hc_sr04.h"
#include "gpio_mcu.h"
#include "lcditse0803.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
#define REFRESCO_MEDICION 1000000
#define REFRESCO_DISPLAY 100000
/*==================[internal data definition]===============================*/
TaskHandle_t Medir_handle = NULL;
TaskHandle_t Mostrar_handle = NULL;
uint16_t distancia; // Declaro la variable global distancia, que utilizaran las demas tareas
bool on;
bool hold;
/*==================[internal functions declaration]=========================*/

void TeclaOn()
{
    on = !on;
}
void TeclaHold()
{
    hold = !hold;
}

void FuncTimerMostrar()
{
    vTaskNotifyGiveFromISR(Mostrar_handle, pdFALSE); // Envia una notificacion asociada a la tarea msotrar
}
void FuncTimerMedir()
{
    vTaskNotifyGiveFromISR(Medir_handle, pdFALSE);
}

void FuncTeclas()
{
    uint8_t teclas;
     UartReadByte(UART_PC, &teclas);
    switch (teclas)
    {
    case 'O':
        on = !on;
        break;

    case 'H':
        hold = !hold;
        break;
    }
}
static void Medir_task(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters(); // Utilizo la funcion del sensor para medir y le asigno el valor a la variable distancia
        }
    }
}

static void Mostrar_task(void *pvParameter)
{ // TAREA PARA MOSTRAR EN DISPLAY Y ENCENDER LEDS SEGUN LA DISTANCIA
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (on)
        {
            if (distancia < 10)
            {
                LedsOffAll();
            }
            else if ((distancia > 10) & (distancia < 20))
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if ((distancia > 20) & (distancia < 30))
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOff(LED_3);
            }
            else if (distancia > 30)
            {
                LedOn(LED_1);
                LedOn(LED_2);
                LedOn(LED_3);
            }
            // AHORA TENGO QUE MOSTRAR EN PANTALLA
            if (!hold)
            {
                LcdItsE0803Write(distancia);
                UartSendString(UART_PC,"La distancia es ");
                UartSendString(UART_PC,(char*)UartItoa(distancia,10));
                UartSendString(UART_PC," cm\r\n ");
            }
        }
        else
        {
            LedsOffAll();
            LcdItsE0803Off();
        }
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();
    timer_config_t timer_medicion = {
        .timer = TIMER_A,
        .period = REFRESCO_MEDICION,
        .func_p = FuncTimerMedir,
        .param_p = NULL};
    TimerInit(&timer_medicion);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = REFRESCO_DISPLAY,
        .func_p = FuncTimerMostrar,
        .param_p = NULL};
    TimerInit(&timer_mostrar);

    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = &FuncTeclas,
        .param_p = NULL};
    UartInit(&puerto_serie);

    //  En este caso, switch_1 inicia una interruocion y elijo que funcion estadoteclas se va a ejecutar
    SwitchActivInt(SWITCH_1, &TeclaOn, NULL);
    SwitchActivInt(SWITCH_2, &TeclaHold, NULL);

    xTaskCreate(&Medir_task, "Medir", 1024, NULL, 5, &Medir_handle);
    xTaskCreate(&Mostrar_task, "Mostrar", 1024, NULL, 5, &Mostrar_handle);

    TimerStart(timer_medicion.timer);
    TimerStart(timer_mostrar.timer);
    UartSendString(UART_PC,"INICIO ");