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
/*==================[macros and definitions]=================================*/
#define REFRESCO_TECLAS 50
#define REFRESCO_MEDICION 1000
#define REFRESCO_DISPLAY 100
/*==================[internal data definition]===============================*/

uint16_t distancia; // Declaro la variable global distancia, que utilizaran las demas tareas
bool on;
bool hold;
/*==================[internal functions declaration]=========================*/

static void Medir_task(void *pvParameter)
{
    while (true)
    {
        if (on)
        {
            distancia = HcSr04ReadDistanceInCentimeters(); // Utilizo la funcion del sensor para medir y le asigno el valor a la variable distancia
        }
        vTaskDelay(REFRESCO_MEDICION / portTICK_PERIOD_MS);
    }
}

static void Mostrar_task(void *pvParameter)
{ // TAREA PARA MOSTRAR EN DISPLAY Y ENCENDER LEDS SEGUN LA DISTANCIA
    while (true)
    {
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
            }
        }
        else
        {
            LedsOffAll();
            LcdItsE0803Off();
        }

        vTaskDelay(REFRESCO_DISPLAY / portTICK_PERIOD_MS);
    }
}

static void Teclas_task()
{
    uint8_t teclas;
    while (true)
    {
        teclas = SwitchesRead();
        switch (teclas)
        {
        case SWITCH_1:
            on = !on;
            break;

        case SWITCH_2:
            hold = !hold;
            break;
        }

        vTaskDelay(REFRESCO_TECLAS / portTICK_PERIOD_MS);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    LedsInit();
    HcSr04Init(GPIO_3, GPIO_2);
    LcdItsE0803Init();
    SwitchesInit();
    xTaskCreate(&Medir_task, "Medir", 512, NULL, 5, NULL);
    xTaskCreate(&Mostrar_task, "Mostrar", 512, NULL, 5, NULL);
    xTaskCreate(&Teclas_task, "Teclas", 512, NULL, 5, NULL);

}
