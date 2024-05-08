/**! @mainpage Template
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
/**
 * @struct gpioConf_t 
 * @brief Contiene la informacion de los pines de GPIO
 * 
*/
typedef struct
{
	gpio_t pin; /*!< Numero de pin GPIO */
	io_t dir;	/*!< Direccion de GPIO '0' ENTRADA; '1' SALIDA*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/
/**
 * @fn void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
 * @brief Convierte un numero decimal a un arreglo BCD
 * @param data El numero decimsl a convertir
 * @param digits El numero de digitos en el arreglo BCD
 * @param bcd_number Puntero a un arreglo donde se almacenaran los digitos en BCD
*/
void convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{

	for (uint32_t i = 0; i < digits; i++)
	{
		bcd_number[i] = data % 10;
		data = data / 10;
	}
}
/**
 * @fn void CambiarEstados(uint8_t data, gpioConf_t *arreglogpio)
 * @brief Cambia los estados de los pines para mostrar un numero BCD
  * Esta función recibe un número BCD y cambia los estados de los pines conectados al display
 * para mostrar los dígitos correspondientes
 * @param data El numero BCD a mostrar
 * @param arreglogpio Arreglo de estructuras gpioConf_t que contiene la configuracion de los pines del display

*/
void CambiarEstados(uint8_t data, gpioConf_t *arreglogpio)
{
	uint8_t mascara = 1; /*!< Utilizo mascara para saber si tengo un 1 o un 0 en cada bit del numero guardado en data */
	uint8_t comparardigito = 0;
	for (int j = 0; j < 4; j++) /*!<Con este ciclo recorro los bits del numero guardado en data para saber si tengo un '0' o un '1'*/
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

/**
 * @fn void MostrarEnDisplay(uint32_t data, uint8_t digits, gpioConf_t *arreglogpio, gpioConf_t *DigitosSalida)
 * @brief Muestra el numero decimal en el display de segmentos
 * Esta funcion recibe un numero en BCD y cambia los pines conectados al display 
 * @param data Numero decimal a msotrar
 * @param digits La cantidad de digitos que tiene el numero (Los que se mostrararn en el display)
 * @param arreglogpio Arreglo de estructuras tipo gpioConf_t que contiene la configuracion de los pines del display
 * @param DigitosSalida Arreglo de estructuras tipo gpioConf_t que contiene la configuracion de los
 * pines que seleccionan los digitos del display
*/
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
	/*!< UInicializo los GPIO para los pines de segmentos */
	gpioConf_t mis_pines[4] = {{GPIO_20, GPIO_OUTPUT}, {GPIO_21, GPIO_OUTPUT}, {GPIO_22, GPIO_OUTPUT}, {GPIO_23, GPIO_OUTPUT}};
	for (int i = 0; i < 4; i++) 
	{
		GPIOInit(mis_pines[i].pin,mis_pines[i].dir); \
	}

	/*!< UInicializo los GPIO para los pines de seleccion de digitos */
	gpioConf_t digitos[3] = {{GPIO_9, GPIO_OUTPUT}, {GPIO_18, GPIO_OUTPUT}, {GPIO_19, GPIO_OUTPUT}};
	for (int i = 0; i < 3; i++)
	{
		GPIOInit(digitos[i].pin,digitos[i].dir);
	}

MostrarEnDisplay(138,3,mis_pines,digitos);
}
/*==================[end of file]============================================*/