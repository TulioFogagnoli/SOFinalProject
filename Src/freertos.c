/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "wizchip_conf.h"
#include "socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> // <-- FIX: Adicionado para a função atoi()

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

// Handles para as Queues do FreeRTOS
QueueHandle_t dutyCycleQueueHandle;
QueueHandle_t adcValueQueueHandle;

// Handle para o Mutex que protegerá o acesso ao socket
SemaphoreHandle_t socketMutexHandle;

// Variável global para compartilhar o estado da conexão
volatile int8_t g_ActiveSocket = -1;

// Handles para os periféricos (declarados em main.c)
extern UART_HandleTypeDef huart2;
extern TIM_HandleTypeDef htim2;
extern ADC_HandleTypeDef hadc1;
extern SPI_HandleTypeDef hspi1; // <-- FIX: Adicionado handle do SPI

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

// Protótipos para as funções de callback do W5500
void w5500_spi_select(void);
void w5500_spi_deselect(void);
void w5500_spi_write_byte(uint8_t tx_byte);
uint8_t w5500_spi_read_byte(void);
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

  // <-- FIX: As funções de criação devem estar DENTRO de uma função.
  // Criar a fila para receber o novo duty cycle do PC (tamanho 1, para um uint8_t)
  dutyCycleQueueHandle = xQueueCreate(1, sizeof(uint8_t));

  // Criar a fila para enviar o valor do ADC para o PC (tamanho 1, para um uint16_t)
  adcValueQueueHandle = xQueueCreate(1, sizeof(uint16_t));

  /* USER CODE END Init */

  /* Create the mutex(es) */
  /* definition and creation of defaultMutex */
  /* USER CODE BEGIN RTOS_MUTEX */
  // <-- FIX: Criar o Mutex aqui.
  socketMutexHandle = xSemaphoreCreateMutex();
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
  osThreadDef(defaultTask, StartDefaultTask, osPriorityIdle, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  osThreadDef(pwmTask, StartPwmTask, osPriorityNormal, 0, 128);
  pwmTaskHandle = osThreadCreate(osThread(pwmTask), NULL);

  osThreadDef(adcTask, StartAdcTask, osPriorityNormal, 0, 128);
  adcTaskHandle = osThreadCreate(osThread(adcTask), NULL);

  osThreadDef(rxW5500Task, StartRxW5500Task, osPriorityNormal, 0, 1028);
  rxW5500TaskHandle = osThreadCreate(osThread(rxW5500Task), NULL);

  osThreadDef(txW5500Task, StartTxW5500Task, osPriorityNormal, 0, 512);
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
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

// --- Thread do PWM ---
void StartPwmTask(void const * argument) {
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);

    uint32_t current_duty_cycle_val = 2100; // Valor inicial para o CCR
    uint8_t new_duty_cycle_percent;

    for(;;) {
        // Verifica se um novo valor (em porcentagem) chegou
        if (xQueueReceive(dutyCycleQueueHandle, &new_duty_cycle_percent, (TickType_t)10) == pdPASS) {
            if (new_duty_cycle_percent > 100) new_duty_cycle_percent = 100;
            // Converte a porcentagem (0-100) para o valor do registrador (0-4199)
            current_duty_cycle_val = (uint32_t)(new_duty_cycle_percent * 41.99);
        }

        __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, current_duty_cycle_val);
        osDelay(20);
    }
}

// --- Thread do ADC ---
void StartAdcTask(void const * argument)
{
    uint16_t adc_value; // O ADC do F4 é de 12 bits, então 16 bits é suficiente
    char msg[32];

    for(;;)
    {
        HAL_ADC_Start(&hadc1);
        if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
        {
            adc_value = HAL_ADC_GetValue(&hadc1);
            
            // Envia o valor para a thread de TX e para a UART de debug
            xQueueSend(adcValueQueueHandle, &adc_value, (TickType_t)10);

            uint8_t duty_cycle_percent = (uint8_t)((adc_value / 4095.0) * 100.0);

            sprintf(msg, "Duty Cicle: %u\r\n", duty_cycle_percent);
            HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
        }
        HAL_ADC_Stop(&hadc1);
        
        osDelay(pdMS_TO_TICKS(500)); // Lê a cada 500ms
    }
}

// --- Thread de RX (Recebimento) da Ethernet ---
// <-- FIX: Lógica corrigida para ser a de RX
void StartRxW5500Task(void const * argument) {
    // 1. Inicialização do W5500
    HAL_GPIO_WritePin(W5500_RST_GPIO_Port, W5500_RST_Pin, GPIO_PIN_SET);
    reg_wizchip_cs_cbfunc(w5500_spi_select, w5500_spi_deselect);
    reg_wizchip_spi_cbfunc(w5500_spi_read_byte, w5500_spi_write_byte);

    wiz_NetInfo net_info = {
        .mac = {0x00, 0x08, 0xDC, 0x12, 0x34, 0x56},
        .ip = {192, 168, 18, 177},
        .sn = {255, 255, 255, 0},
        .gw = {192, 168, 18, 1}
    };
    wizchip_init(NULL, NULL);
    wizchip_setnetinfo(&net_info);

    uint8_t rx_buffer[2048];
    uint8_t socket_num = 0;
    uint16_t port = 7;

    for(;;) {
        switch(getSn_SR(socket_num)) {
            case SOCK_INIT:
                listen(socket_num);
                break;
            case SOCK_ESTABLISHED:
                if(g_ActiveSocket == -1) {
                    if(xSemaphoreTake(socketMutexHandle, (TickType_t)10) == pdTRUE) {
                        g_ActiveSocket = socket_num;
                        xSemaphoreGive(socketMutexHandle);
                    }
                }
                int32_t len = getSn_RX_RSR(socket_num);
                if(len > 0) {
                    len = recv(socket_num, rx_buffer, sizeof(rx_buffer));
                    if(len > 0) {
                        uint8_t new_duty_cycle = (uint8_t)atoi((const char*)rx_buffer);
                        xQueueSend(dutyCycleQueueHandle, &new_duty_cycle, (TickType_t)10);
                    }
                }
                break;
            case SOCK_CLOSE_WAIT:
                disconnect(socket_num);
                break;
            case SOCK_CLOSED:
                if(g_ActiveSocket != -1) {
                    if(xSemaphoreTake(socketMutexHandle, (TickType_t)10) == pdTRUE) {
                        g_ActiveSocket = -1;
                        xSemaphoreGive(socketMutexHandle);
                    }
                }
                socket(socket_num, Sn_MR_TCP, port, 0);
                break;
            default:
                break;
        }
        osDelay(20);
    }
}

// --- Thread de TX (Transmissão) da Ethernet ---
// <-- FIX: Lógica corrigida para ser a de TX
void StartTxW5500Task(void const * argument) {
  uint16_t adc_val;
  int8_t current_socket = -1;

  for(;;) {
      // Espera bloqueante por um novo valor do ADC (isso não muda)
      if (xQueueReceive(adcValueQueueHandle, &adc_val, portMAX_DELAY) == pdPASS) {
          
          // Converte o valor do ADC (0-4095) para porcentagem (0-100)
          // Usamos 4095.0 para forçar a conta a ser feita com ponto flutuante
          uint8_t duty_cycle_percent = (uint8_t)((adc_val / 4095.0) * 100.0);

          if(xSemaphoreTake(socketMutexHandle, (TickType_t)10) == pdTRUE) {
              current_socket = g_ActiveSocket;
              xSemaphoreGive(socketMutexHandle);
          }
          if(current_socket != -1) {
              char tx_buffer[20]; // Aumente um pouco o buffer por segurança
              // Formata a string para enviar a porcentagem calculada
              sprintf(tx_buffer, "Leitura: %u%%\r\n", duty_cycle_percent); 
              send(current_socket, (uint8_t*)tx_buffer, strlen(tx_buffer));
          }
      }
    }
}

// --- Funções de Callback para o Driver W5500 ---
void w5500_spi_select(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_RESET);
}

void w5500_spi_deselect(void) {
    HAL_GPIO_WritePin(W5500_CS_GPIO_Port, W5500_CS_Pin, GPIO_PIN_SET);
}

void w5500_spi_write_byte(uint8_t tx_byte) {
    HAL_SPI_Transmit(&hspi1, &tx_byte, 1, HAL_MAX_DELAY);
}

uint8_t w5500_spi_read_byte(void) {
    uint8_t rx_byte;
    HAL_SPI_Receive(&hspi1, &rx_byte, 1, HAL_MAX_DELAY);
    return rx_byte;
}

/* USER CODE END Application */
