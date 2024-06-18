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
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 * | 18/06/2024 | Document creation		                         |
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
#include "gpio_mcu.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "hc_sr04.h"

/*==================[macros and definitions]=================================*/
#define FRECUENCIA_AD 1000
#define REFRESCO_MEDICION 10000

/*==================[internal data definition]===============================*/
TaskHandle_t ANALOG_DIGIT_TASKHANDLE = NULL;
TaskHandle_t Medir_handle = NULL;
/**

 * @param distancia Varaible global a la que le asigno la medicion de distancia en cm
 * @param temperatura_promedio variable global a la que le asigno el promedio de 10 temperaturas
 * @param verificar_temp booleano utilizado para que no se muestren mas valores cuando se esta midiendo la misma persona
*/
uint16_t distancia = 0;
float temperatura_promedio = 0;
bool verificar_temp = false; 
/*==================[internal functions declaration]=========================*/

static void AnalogtoDigit()
{
    vTaskNotifyGiveFromISR(ANALOG_DIGIT_TASKHANDLE, pdFALSE);
}

void FuncTimerMedir()
{
    vTaskNotifyGiveFromISR(Medir_handle, pdFALSE);
}

/**
 * @brief Mide la distancia y en base a ella (si esta en el rango de +/- 10cm)mustra la temperatura por la UART
 * en caso de que la temperatura sea matyor a un limite (37.5ºC) se enciende una alarma en el gpio9
 * @param distancnia Distancia en cm medida por el sensor
 * @param temperatura_promedio Promedio de 10 temperaturas

*/
static void Medir_task(void *pvParameter)
{
    while (true)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        distancia = HcSr04ReadDistanceInCentimeters(); /*!< Utilizo la funcion del sensor para medir y le asigno el valor a la variable distancia */
        if (distancia > 140)
        {
             /*! Reinicio medidas*/
            temperatura_promedio = 0;
            GPIOOff(GPIO_9);
            LedOff(LED_1);
            LedOff(LED_2);
            LedOff(LED_3);

            
            verificar_temp = false; /*! Reinicio la variable de temperatura mostrada*/
        }
            else if (distancia < 8)
            {
                LedOn(LED_1);
                LedOff(LED_2);
                LedOff(LED_3);
            }
            else if ((distancia > 8) & (distancia < 12))
            {
                LedOn(LED_2);
                LedOff(LED_1);
                LedOff(LED_3);
                if (!verificar_temp)
                {
                    UartSendString(UART_PC, (char *)UartItoa(temperatura_promedio, 10));
                    UartSendString(UART_PC, "ºC a ");
                    UartSendString(UART_PC, (char *)UartItoa(distancia, 10));
                    UartSendString(UART_PC, " cm \r");
                    verificar_temp = true; /*! Marcar que la temperatura se ha mostrado */
                }

                    if (temperatura_promedio > 37.5) /*!< Activo la alarma en caso de superar el limite de temperatura*/
                        GPIOOn(GPIO_9);
                    else
                        GPIOOff(GPIO_9);
                }
                else if (distancia > 12)
                {
                    LedOff(LED_1);
                    LedOff(LED_2);
                    LedOn(LED_3);
                }
            }
        }
        /**
         * @brief Pasa de un valor analogico (en mV) que nos da el sensor y lo pasa a un valor digital (en ºC)
         * @param temperatura variable local para calcular el promedio de 10 mediciones
         * @param temperatura_promedio variable global que nos da el promedio de 10 temperaturas

        */
        void AnalogicoAdigital(void *pvParameter)
        {
            uint16_t ValorAnalogico = 0;
            float temperatura = 0;
            int i = 0;
            while (1)
            {
                ulTaskNotifyTake(pdTRUE, portMAX_DELAY);     /*!< Paso de mV a grados C */
                AnalogInputReadSingle(CH1, &ValorAnalogico); /*!< En este punto tengo el valor de entrada en mV */

                i++;
                temperatura += (ValorAnalogico / 110) + 20; /*!< Paso de mV a grados C */
                if (i == 10)
                {
                    temperatura /= 10;
                    i = 0;
                    temperatura_promedio = temperatura;
                    temperatura = 0;
                }
            }
        }

        /*==================[external functions definition]==========================*/
        void app_main(void)
        { /*!< Inicializo  el GPIO para la alarma */
            GPIOInit(GPIO_9, GPIO_OUTPUT);
            /*!< Inicializo  los leds */
            LedsInit();
            /*!< Inicializo  el sensor de ultrasonido para la distancia  */
            HcSr04Init(GPIO_3, GPIO_2);

            /*!<Configuro la entrada analogica */
            analog_input_config_t entrada_analogica = {
                .input = CH1,
                .mode = ADC_SINGLE,
                .func_p = NULL,
                .param_p = NULL,
                .sample_frec = 0};
            AnalogInputInit(&entrada_analogica);
            /*!< Inicializo  la entrada analogica */
            AnalogOutputInit();

            /*!<Configuro el puerto serie */
            serial_config_t puerto_serie = {
                .port = UART_PC,
                .baud_rate = 115200,
                .func_p = NULL,
                .param_p = NULL};
            /*!< Inicializo  el puerto serie */
            UartInit(&puerto_serie);

            /*!< Configuro timer medicion */
            timer_config_t timer_medicion = {
                .timer = TIMER_A,
                .period = REFRESCO_MEDICION,
                .func_p = FuncTimerMedir,
                .param_p = NULL};
            TimerInit(&timer_medicion);

            /*!< Configuro timer para el sensor de temperatura */
            timer_config_t timerAnalog_digital = {
                .timer = TIMER_B,
                .period = FRECUENCIA_AD,
                .func_p = &AnalogtoDigit,
                .param_p = NULL};
            TimerInit(&timerAnalog_digital);

            xTaskCreate(&AnalogicoAdigital, "Analogico a digital", 4096, NULL, 5, &ANALOG_DIGIT_TASKHANDLE);
            xTaskCreate(&Medir_task, "Medir", 1024, NULL, 5, &Medir_handle);

            TimerStart(timerAnalog_digital.timer);
            TimerStart(timer_medicion.timer);
        }
