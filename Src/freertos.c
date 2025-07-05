/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdio.h>
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
osThreadId adcTaskHandle;
osThreadId pwmTaskHandle;
osThreadId rxW5500TaskHandle;
osThreadId txW5500TaskHandle;

/* USER CODE END Variables */
osThreadId defaultTaskHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void StartPwmTask(void const * argument);
void StartAdcTask(void const * argument);
void StartRxW5500Task(void const * argument);
void StartTxW5500Task(void const * argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  // osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  // defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 128);
  pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

  osThreadDef(adcTask, StartAdcTask, osPriorityNormal, 0, 128);
  adcTaskHandle = osThreadCreate(osThread(adcTask), NULL);

  osThreadDef(rxW5500Task, StartRxW5500Task, osPriorityRealtime, 0, 128);
  rxW5500TaskHandle = osThreadCreate(osThread(rxW5500Task), NULL);

  osThreadDef(txW5500Task, StartTxW5500Task, osPriorityRealtime, 0, 128);
  txW5500TaskHandle = osThreadCreate(osThread(txW5500Task), NULL);

  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
//  /* Infinite loop */
//  for(;;)
//  {
//    osDelay(1);
//  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartPwmTask(void const * argument) {
  // Inicia o canal PWM. Substitua TIM2 e TIM_CHANNEL_1 pelos seus.
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

  uint32_t duty_cycle = 2100; // Começa com ~50% (metade de ARR=4199)

  for(;;) {
    // Define o duty cycle do PWM
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, duty_cycle);

    // Apenas para teste, mude o duty cycle ao longo do tempo
    // duty_cycle += 100;
    // if (duty_cycle > 4199) {
    //   duty_cycle = 0;
    // }
    osDelay(100); // Atraso de 100ms
  }
}

void StartAdcTask(void const * argument)
{
  /* USER CODE BEGIN vTaskADC */
  uint32_t adc_value;
  char msg[32]; // Buffer para guardar a mensagem a ser enviada

  /* Infinite loop */
  for(;;)
  {
    // Inicia a conversão ADC
    HAL_ADC_Start(&hadc1);

    // Espera a conversão ser finalizada
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
      // Lê o valor convertido
      adc_value = HAL_ADC_GetValue(&hadc1);

      // Formata a string com o valor do ADC
      sprintf(msg, "Valor do ADC: %lu\r\n", adc_value);

      // Envia a mensagem pela UART2
      // O handle 'huart2' foi gerado pelo CubeMX e está em main.h
      HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
    }

    // Para a conversão para economizar energia
    HAL_ADC_Stop(&hadc1);

    // Aguarda 1 segundo antes da próxima leitura
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  /* USER CODE END vTaskADC */
}

void StartRxW5500Task(void const * argument) {

  for(;;)
  {}
}

void StartTxW5500Task(void const * argument) {

  for(;;)
  {}

}


/* USER CODE END Application */
