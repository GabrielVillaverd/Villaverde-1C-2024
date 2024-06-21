/*! @mainpage Proyecto integrador
 *
 * \section genDesc General Description
 *
 * El proyecto consiste en un sistema que controla
 * automáticamente las condiciones ambientales de una
 * unidad de terapia intensiva. Mediante un control por Bluetooth,
 * se gestionan los niveles de luminosidad, temperatura y humedad
 * para asegurar un entorno óptimo para los pacientes
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * |    Si7007	 	|   GPIO_21      |
 * | 	Vcc Si7007	| 	3V3		    |
 * | 	PWM_1	 	| 	CH1		    |
 * | 	PWM_2	 	| 	CH2		    |
 * | 	NeoPixel 	| 	GPIO_18	    |
 * | 	Vcc NeoPixel| 	5V		    |
 * | 	servo_sg90 	| 	GPIO_9	    |
 * | 	LDR     	| 	GPIO_3	    |
 * | 	Vcc LDR    	| 	3V3
 * | 	GND	 	    | 	GND		    |
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/05/2024 | Document creation		                         |
 *
 * @author Villaverde Gabriel (gabrielggv97@gmail.com)
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led.h"
#include "gpio_mcu.h"
#include "switch.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "l293.h"
#include "Si7007.h"
#include "servo_sg90.h"
#include "ble_mcu.h"
#include "neopixel_stripe.h"
#include "ws2812b.h"

/*==================[macros and definitions]=================================*/
/** @brief Tiempos definidos para cada una de las tareas
 */
#define FRECUENCIA_AD 4000000
#define FRECUENCIA_TyH 4000000
#define FRECUENCIA_HUM 4000000
#define BUFFER_SIZE 231

/*==================[internal data definition]===============================*/
TaskHandle_t DIGT_ANALOG_TASKHANDLE = NULL;
TaskHandle_t ANALOG_DIGIT_TASKHANDLE = NULL;
TaskHandle_t Bluet_taskhande = NULL;
TaskHandle_t Calefaccion_taskhandle = NULL;
TaskHandle_t Humedad_taskhandle = NULL;
TaskHandle_t Luces_taskhandle = NULL;
TaskHandle_t Cortinas_taskhandle = NULL;
TaskHandle_t main_task_handle = NULL;
servo_out_t servo = SERVO_0;
/** @brief Parametro de referencia para comparar con los valores de luz ambiente*/
float parametro_luz = 0;
/** @brief Parametro de referencia para comparar con los valores de humedad ambiente*/
float parametro_humedad = 0;
/** @brief Parametro de referencia para comparar con los valores de temperatura ambiente*/
float parametro_temperatura = 0;
/** @brief Umbrales alto y bajo para definir un rango de temperatura ideal*/
float umbral_temperatura_bajo = 0;
float umbral_temperatura_alto = 0;
float umbral_humedad_alto = 0;

/*==================[internal functions declaration]=========================*/
/** @brief Función invocada en la interrupción del la tarea que transforma los valores analogicos de luz en digitales*/
static void AnalogtoDigit()
{
    vTaskNotifyGiveFromISR(ANALOG_DIGIT_TASKHANDLE, pdFALSE);
}
/** @brief Función invocada en la interrupción del la tarea dedicada al maejor de datos por bluetooth*/
void Bluet()
{
    vTaskNotifyGiveFromISR(Bluet_taskhande, pdFALSE);
}
/** @brief Función invocada en la interrupción del las tareas de temperatura y humedad*/
void Calefaccion_notif()
{
    vTaskNotifyGiveFromISR(Calefaccion_taskhandle, pdFALSE);
    vTaskNotifyGiveFromISR(Humedad_taskhandle, pdFALSE);
}

/** @brief Función dedicada al manejo de los datos que se envian y reciben por bluetooth
 * @param data      Puntero a arreglo de datos recibidos
 * @param length    Longitud del arreglo de datos recibido
 */
void Manejo_Datos(uint8_t *data, uint8_t length)
{
    uint8_t i = 1;
    static float temperat = 0, luz = 0, humedad = 0;

    if (data[0] == 't')
    {
        /** El slidebar temperatura envía los datos con el formato "t" + value + "A" */
        temperat = 0;
        while (data[i] != 'A')
        {
            /** Convertir el valor ASCII a un valor entero */
            temperat = temperat * 10;
            temperat = temperat + (data[i] - '0'); // Resto el valor ASCII del 0 para obtener el numero
            i++;
        }
    }
    else if (data[0] == 'h')
    {
        /** El slidebar Humedad envía los datos con el formato "h + value + "A" */
        humedad = 0;
        while (data[i] != 'A')
        {
            /** Convertir el valor ASCII a un valor entero */
            humedad = humedad * 10;
            humedad = humedad + (data[i] - '0');
            i++;
        }
    }
    else if (data[0] == 'l')
    {
        /** El slidebar luz envía los datos con el formato "l" + value + "A" */
        luz = 0;
        while (data[i] != 'A')
        {
            /** Convertir el valor ASCII a un valor entero */
            luz = luz * 10;
            luz = luz + (data[i] - '0');
            i++;
        }
    }
    /**Los datos recubidos por bluettoth se asignan a variables globales*/
    parametro_temperatura = temperat;
    parametro_humedad = humedad;
    parametro_luz = luz;
    umbral_temperatura_bajo = parametro_temperatura - 2.0;
    umbral_temperatura_alto = parametro_temperatura + 2.0;
    umbral_humedad_alto = parametro_humedad;

    char cadena_hum[128];
    strcpy(cadena_hum, "");
    sprintf(cadena_hum, "*r%.2f\n*", parametro_humedad); // muestro el parametro humedad por ble utilizando la letra 'r'
    BleSendString(cadena_hum);

    char cadena_temp[128];
    strcpy(cadena_temp, "");
    sprintf(cadena_temp, "*c%.2f\n*", parametro_temperatura); // muestro el parametro temperatura por ble utilizando la letra 'c'
    BleSendString(cadena_temp);

    char cadena_luz[128];
    strcpy(cadena_luz, "");
    sprintf(cadena_luz, "*u%.2f\n*", parametro_luz); // muestro el parametro humedad por ble utilizando la letra 'u'
    BleSendString(cadena_luz);
}

/**
 * @brief Tarea encargada de convertir los datos analogicos (de luminocidad) a valores degitales a finde
 * poder trabajar con estos datos
 */
void AnalogicoAdigital(void *pvParameter)
{
    while (1)
    {
        uint16_t ValorAnalogico = 0;
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        AnalogInputReadSingle(CH3, &ValorAnalogico); /**Los valores analogicos que ingresan a CH3 pasan a digitales y
                                                         se guaran en ValorAnalogico*/
        char cadena_luz[128];
        strcpy(cadena_luz, "");
        sprintf(cadena_luz, "*L%d\n*", ValorAnalogico); /**Enviamos valores medidos de luminocidad por bluettoth*/
        BleSendString(cadena_luz);
        if (ValorAnalogico >= parametro_luz)
        {
            ServoMove(servo, 90);
            NeoPixelAllOff();
        } /**En caso de que el parametro de luz sea mayor o igual al valor seteado se
                              apagan las luces y se abren las cortinas*/
        else
        {
            ServoMove(servo, 0);
            NeoPixelAllColor(NEOPIXEL_COLOR_WHITE);
        } /**En caso de que el parametro de luz sea menor al valor seteado se
                              prenden las luces y se cierran las cortinas*/
    }
}
/**
 * @brief Tarea encargada de medir humedad mediante sensor Si7007
 * y enviarlos por bluetooth
 */
void Humedad(void *pvParameter)
{
    float humedad_tarea = 0;
    ;
    int i = 0;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        humedad_tarea += Si7007MeasureHumidity();

        i++;
        if (i == 3)
        {
            humedad_tarea /= 3; /**Promedio de 3 valores de humedad medidos */
            char cadena_hum[128];
            strcpy(cadena_hum, "");
            sprintf(cadena_hum, "*H%.2f\n*", humedad_tarea);
            BleSendString(cadena_hum); /**Se envian por bluetooth los valores de humedad medidos  */

            if (humedad_tarea > umbral_humedad_alto)

                printf("\nSe prende aire en modo seco");
            else
                printf("\nEl aire en modo seco esta apagado");
            i = 0;
            humedad_tarea = 0;
        }
    }
}

/**
 * @brief Tarea encargada de medir temperatura mediante sensor Si7007
 * y enviarlos por bluetooth
 */
void Calefaccion(void *pvParameter)
{
    float temperatura_tarea = 0;
    int i = 0;
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        temperatura_tarea += Si7007MeasureTemperature();
        i++;
        if (i == 3)
        {
            temperatura_tarea = (temperatura_tarea / 3); /**Promedio de 3 valores de temperatura medidos*/
            char cadena_temp[128];
            strcpy(cadena_temp, "");
            sprintf(cadena_temp, "*T%.2f\n*", temperatura_tarea);
            BleSendString(cadena_temp); /**Se envian por bluetooth los valores de humedad medidos  */
            LedOn(LED_2);               /**    Led que nos avisa que la temperatura esta dentro del rango aceptado*/
            if (temperatura_tarea < umbral_temperatura_bajo)
            {
                LedOff(LED_2);
                LedOn(LED_3);
                printf("\nSe prende aire en calor\n"); /**Si la temperatura es menor al umbral de temperatura
                                                        tolerado se prende el led 3 simulando aire en calor y se apaga el
                                                        led que simula temperatura en el rango aceptado*/
            }
            else
                LedOff(LED_3);
            if (temperatura_tarea > umbral_temperatura_alto)
            {
                LedOff(LED_2);
                LedOn(LED_1);
                printf("\nSe prende aire en frio"); /**Si la temperatura es menor al umbral de temperatura
                                                         tolerado se prende el led 3 simulando aire en calor y se apaga el
                                                         led que simula temperatura en el rango aceptado*/
            }
            else
                LedOff(LED_1);

            i = 0;
            temperatura_tarea = 0;
        }
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
    /**Configuro la entrada analogica para medicion de luz*/
    analog_input_config_t entrada_analogica = {
        .input = CH3,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};
    AnalogInputInit(&entrada_analogica);
    AnalogOutputInit();

    /**Confuguro sensor de humedad y temperatura*/
    Si7007_config sensorTyH = {
        .select = GPIO_21,
        .PWM_1 = CH1,
        .PWM_2 = CH2};
    Si7007Init(&sensorTyH);

    /**Creo timer para tarea medicion de luz */
    timer_config_t timerAnalog_digital = {
        .timer = TIMER_B,
        .period = FRECUENCIA_AD,
        .func_p = &AnalogtoDigit,
        .param_p = NULL};
    TimerInit(&timerAnalog_digital);

    /**Creo timer para tarea medicion de humedad y temperatura */
    timer_config_t Calefaccion_timer = {
        .timer = TIMER_A,
        .period = FRECUENCIA_TyH,
        .func_p = Calefaccion_notif,
        .param_p = NULL};
    TimerInit(&Calefaccion_timer);

    /**Creacion de las tareas*/
    xTaskCreate(&Humedad, "Tarea humedad", 4096, NULL, 5, &Humedad_taskhandle);
    xTaskCreate(&Calefaccion, "Tarea calefaccion", 4096, NULL, 5, &Calefaccion_taskhandle);
    xTaskCreate(&AnalogicoAdigital, "Analogico a digital", 4096, NULL, 5, &ANALOG_DIGIT_TASKHANDLE);

    /**Inicializo los timers */
    TimerStart(timerAnalog_digital.timer);
    TimerStart(Calefaccion_timer.timer);

    /**Configuro e inicializo el servo*/
    servo_out_t servo =
        SERVO_0;
    ServoInit(servo, GPIO_9);

    /**Inicializo Leds */
    LedsInit();

    /**Inicializo los Leds necesarios para prender luz interior (anillo led)*/
    static neopixel_color_t AnilloLeds[12];
    NeoPixelInit(GPIO_18, 12, &AnilloLeds);

    /**Configuracion e iniciacion de bluetooth */
    ble_config_t configuracionBle = {
        .device_name = "Centro de control",
        .func_p = Manejo_Datos};
    BleInit(&configuracionBle);
}