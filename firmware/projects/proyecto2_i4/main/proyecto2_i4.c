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
#include "gpio_mcu.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"

/*==================[macros and definitions]=================================*/
#define FRECUENCIA_AD 2000
#define FRECUENCIA_DA 4000
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/
TaskHandle_t DIGT_ANALOG_TASKHANDLE = NULL;
TaskHandle_t ANALOG_DIGIT_TASKHANDLE = NULL;


/*==================[internal data definition]===============================*/
TaskHandle_t main_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76,
    77,
    78,
    77,
    79,
    86,
    81,
    76,
    84,
    93,
    85,
    80,
    89,
    95,
    89,
    85,
    93,
    98,
    94,
    88,
    98,
    105,
    96,
    91,
    99,
    105,
    101,
    96,
    102,
    106,
    101,
    96,
    100,
    107,
    101,
    94,
    100,
    104,
    100,
    91,
    99,
    103,
    98,
    91,
    96,
    105,
    95,
    88,
    95,
    100,
    94,
    85,
    93,
    99,
    92,
    84,
    91,
    96,
    87,
    80,
    83,
    92,
    86,
    78,
    84,
    89,
    79,
    73,
    81,
    83,
    78,
    70,
    80,
    82,
    79,
    69,
    80,
    82,
    81,
    70,
    75,
    81,
    77,
    74,
    79,
    83,
    82,
    72,
    80,
    87,
    79,
    76,
    85,
    95,
    87,
    81,
    88,
    93,
    88,
    84,
    87,
    94,
    86,
    82,
    85,
    94,
    85,
    82,
    85,
    95,
    86,
    83,
    92,
    99,
    91,
    88,
    94,
    98,
    95,
    90,
    97,
    105,
    104,
    94,
    98,
    114,
    117,
    124,
    144,
    180,
    210,
    236,
    253,
    227,
    171,
    99,
    49,
    34,
    29,
    43,
    69,
    89,
    89,
    90,
    98,
    107,
    104,
    98,
    104,
    110,
    102,
    98,
    103,
    111,
    101,
    94,
    103,
    108,
    102,
    95,
    97,
    106,
    100,
    92,
    101,
    103,
    100,
    94,
    98,
    103,
    96,
    90,
    98,
    103,
    97,
    90,
    99,
    104,
    95,
    90,
    99,
    104,
    100,
    93,
    100,
    106,
    101,
    93,
    101,
    105,
    103,
    96,
    105,
    112,
    105,
    99,
    103,
    108,
    99,
    96,
    102,
    106,
    99,
    90,
    92,
    100,
    87,
    80,
    82,
    88,
    77,
    69,
    75,
    79,
    74,
    67,
    71,
    78,
    72,
    67,
    73,
    81,
    77,
    71,
    75,
    84,
    79,
    77,
    77,
    76,
    76,
};

/*==================[internal functions declaration]=========================*/
static void DigiToAnalog()
{
    vTaskNotifyGiveFromISR(DIGT_ANALOG_TASKHANDLE, pdFALSE);
}

static void AnalogtoDigit()
{
    vTaskNotifyGiveFromISR(ANALOG_DIGIT_TASKHANDLE, pdFALSE);
}

/*Void EscribirEnPc()
{
    UartSendString(UART_PC, "Valor");
    UartSendString(UART_PC, (char *)UartItoa(ValorAnalogico, 10));
    UartSendString(UART_PC, "\r");

    En caso de querer utilizar esta funcion, es necesario declarar ValorAnalogico como una 
    variable global, lo cual (por razones que no pude entender) me genera problemas
}*/

void AnalogicoAdigital(void *pvParameter)
{uint16_t ValorAnalogico = 0;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH1, &ValorAnalogico);
        //printf("%d\n", ValorAnalogico);
        //EscribirEnPc();
        UartSendString(UART_PC,(char *)UartItoa(ValorAnalogico,10));
        UartSendString(UART_PC,"\r");
    }
}

void DigitalAanalogico(void *pvParameter)

{uint8_t indice = 0;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); // La tarea espera en este punto hasta recibir una notific
        AnalogOutputWrite(ecg[indice]);
        indice++;
        //printf("%d\n",indice);
        if (indice == sizeof(ecg)){
            indice=0;
        }
        
    }
}

/*==================[external functions definition]==========================*/
void app_main(void)
{

    timer_config_t timerDigital_analogico = {
        .timer = TIMER_A,
        .period = FRECUENCIA_DA,
        .func_p = &DigiToAnalog,
        .param_p = NULL};
    TimerInit(&timerDigital_analogico);

    timer_config_t timerAnalog_digital = {
        .timer = TIMER_B,
        .period = FRECUENCIA_AD,
        .func_p = &AnalogtoDigit,
        .param_p = NULL};
    TimerInit(&timerAnalog_digital);

    analog_input_config_t entrada_analogica = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&entrada_analogica);
    AnalogOutputInit();

    serial_config_t puerto_serie = {
        .port = UART_PC,
        .baud_rate = 115200,
        .func_p = NULL,
        .param_p = NULL};
    UartInit(&puerto_serie);

    xTaskCreate(&DigitalAanalogico, "Digital a analogico", 4096, NULL, 5, &DIGT_ANALOG_TASKHANDLE);
    xTaskCreate(&AnalogicoAdigital, "Analogico a digital", 4096, NULL, 5, &ANALOG_DIGIT_TASKHANDLE);

    TimerStart(timerAnalog_digital.timer);
    TimerStart(timerDigital_analogico.timer);
    
}
