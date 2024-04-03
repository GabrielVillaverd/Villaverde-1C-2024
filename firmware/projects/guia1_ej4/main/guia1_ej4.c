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
 * @author Gabriel Villaverde (gabrielggv97@gmail.com)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{

	for (uint32_t i = 0; i < digits; i++)
	{
		bcd_number[i] = data % 10;
		data = data / 10;
	}
}

void CambiarEstados(uint8_t data, gpioConf_t *arreglogpio)
{
	uint8_t mascara = 1;
	uint8_t comparardigito = 0;
	for (int j = 0; j < 4; j++)
	{
		comparardigito = data & mascara;
		if (comparardigito > 0)
		{
			GPIOOn(arreglogpio[j].pin);
		}
		else
		{
			GPIOOff(arreglogpio[j].pin);
		}

		mascara = mascara << 1;
	}
}

void MostrarEnDisplay(uint32_t data, uint8_t digits, gpioConf_t *arreglogpio, gpioConf_t *DigitosSalida)
{
	uint8_t bcd_number[digits];
	convertToBcdArray(data, digits, bcd_number);
	for (uint8_t i = 0; i < digits; i++)
	{
		CambiarEstados(bcd_number[i], arreglogpio);
		GPIOOn(DigitosSalida[i].pin);
		GPIOOff(DigitosSalida[i].pin);
	}
}


/*==================[external functions definition]==========================*/
void app_main(void)
{
	gpioConf_t mis_pines[4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	for (int i = 0; i < 4; i++)
	{
		GPIOInit(mis_pines[i].pin,mis_pines[i].dir);
	}

	gpioConf_t digitos[3] = {{GPIO_9, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_19, GPIO_OUTPUT}};
	for (int i = 0; i < 3; i++)
	{
		GPIOInit(digitos[i].pin,digitos[i].dir);
	}

MostrarEnDisplay(138,3,mis_pines,digitos);
}
/*==================[end of file]============================================*/