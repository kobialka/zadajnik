/**
  ******************************************************************************
  * @file    Templates/Src/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.1.0 modified by ARM
  * @date    26-June-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2014 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_it.h"
#include "stm32f4_discovery.h"
#include "stm32f4_discovery_accelerometer.h"
#include "uart.h"
#include "mpu9250_m.h"

#include "stm32f4xx_hal_uart.h"
#ifdef _RTE_
#include "RTE_Components.h"             // Component selection
#endif

/** @addtogroup STM32F4xx_HAL_Examples
  * @{
  */

/** @addtogroup Templates
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/* External variables --------------------------------------------------------*/
extern UART_HandleTypeDef		 huart4;
extern TIM_HandleTypeDef 		hTimer6;


/* Variables -----------------------------------------------------------------*/
volatile  uint32_t u32_SampleCounter;						// nieoptymalizowana, zachowuje wartość.

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void SVC_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
#ifndef RTE_CMSIS_RTOS_RTX
void SysTick_Handler(void)
{
  HAL_IncTick();
}
#endif



/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f4xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles UART 4 interrupt request.
  * @param  none
  * @retval None
  */
void UART4_IRQHandler(void){
//	HAL_UART_IRQHandler(&huart4);

	uint32_t tmp1 = 0, tmp2 = 0;


	/* UART Receiver INT ---------------------------------------------------*/
	tmp1 = __HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE);			// czy flaga jest ustawiona
	tmp2 = __HAL_UART_GET_IT_SOURCE(&huart4, UART_IT_RXNE);			// czy przerwanie RXNE jest włączone

	if((tmp1 != RESET) && (tmp2 != RESET)){
		Reciever_PutCharacterToBuffer(huart4.Instance->DR);
	}


	/* UART Transmitter INT ------------------------------------------------*/
	tmp1 = __HAL_UART_GET_FLAG(&huart4, UART_FLAG_TXE);				// czy flaga jest ustawiona. Flaga TXE jest ustawiona zawsze gdy TDR jest puste, więc zgłaszane będzie przerwanie. Dlatego po wysłaniu ostatniego bajtu wyłączamy przerwanie TXE(IE).
	tmp2 = __HAL_UART_GET_IT_SOURCE(&huart4, UART_IT_TXE);			// czy przerwanie TXE jest włączone

	if((tmp1 != RESET) && (tmp2 != RESET)){
		char cCharacter;

		if(0 == Transmiter_GetFrameID()){
			cCharacter = Transmiter_GetCharacterFromBuffer();
			if ( 0 != cCharacter ){
				huart4.Instance->DR = cCharacter;
			}
		}
		else{
			cCharacter = Transmiter_GetRawByteFromBuffer();
			huart4.Instance->DR = cCharacter;
		}
	}
}


void TIM6_DAC_IRQHandler(void){
	BSP_LED_Toggle(BLUE);
	__HAL_TIM_CLEAR_FLAG(&hTimer6,TIM_IT_UPDATE);
}



/**
  * @brief  This function handles External line 0 interrupt request.
  * @param  none
  * @retval None
  */
void EXTI0_IRQHandler(void){
	__HAL_GPIO_EXTI_CLEAR_IT(ACCELERO_INT1_PIN);
	BSP_LED_Toggle(ORANGE);
	MPU9250_WhoAmI();
}

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/


/**
  * @}
  */ 

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
