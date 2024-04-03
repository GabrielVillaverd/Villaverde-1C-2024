/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 20/03/2024 | Document creation		                         |
 *
 * @author Villaverde Gabriel (gabrielggv97@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
#define OFF 0
#define ON 1
#define TOGGLE 2
#define CONFIG_BLINK_PERIOD 100
/*==================[internal data definition]===============================*/
struct leds
{
      uint8_t mode;       //ON, OFF, TOGGLE
	uint8_t n_led;        //indica el número de led a controlar
	uint8_t n_ciclos;   //indica la cantidad de ciclos de ncendido/apagado
	uint16_t periodo;    //indica el tiempo de cada ciclo
} my_leds; 

/*==================[internal functions declaration]=========================*/
void ControlLeds(struct leds *led)
{
	switch (led->mode)
	{
	case ON:
		switch (led->n_led)
		{
		case 1:
			LedOn(LED_1);

			break;
		case 2:
			LedOn(LED_2);
			break;
		case 3:
			LedOn(LED_3);
			break;
		}
		break;
	case OFF:
		switch (led->n_led)
		{
		case 1:
			LedOff(LED_1);

			break;
		case 2:
			LedOff(LED_2);
			break;
		case 3:
			LedOff(LED_3);
			break;
		}
		break;
	case TOGGLE:

		for (uint8_t ciclos = 0; ciclos < led->n_ciclos; ciclos++)
		{
			switch (led->n_led)
			{
			case 1:
				LedToggle(LED_1);

				break;
			case 2:
				LedToggle(LED_2);
				break;
			case 3:
				LedToggle(LED_3);
				break;
			}
			for (uint16_t retardo = 0; retardo < led->periodo / CONFIG_BLINK_PERIOD; retardo++)
			{
				vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
			}
		}
		break;
	}
}
/*==================[external functions definition]==========================*/
void app_main(void){
	LedsInit();
	 my_leds.mode=TOGGLE;
	 my_leds.n_ciclos=10;
	 my_leds.periodo=500;
	 my_leds.n_led=1;

	 ControlLeds(&my_leds);
}
/*==================[end of file]============================================*/