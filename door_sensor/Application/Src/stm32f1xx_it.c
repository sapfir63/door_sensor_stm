/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 *
 * COPYRIGHT(c) 2015 STMicroelectronics
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
#include <stdbool.h>
#include "stm32f1xx_hal.h"
#include "stm32f1xx.h"
#include "stm32f1xx_it.h"

#include "init.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c1;

extern DEVICE_T device;
extern uint8_t PwrState;
extern bool sleep;

/******************************************************************************/
/*            Cortex-M3 Processor Interruption and Exception Handlers         */
/******************************************************************************/

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
	/* USER CODE BEGIN SysTick_IRQn 0 */

	/* USER CODE END SysTick_IRQn 0 */
	HAL_IncTick();
	HAL_SYSTICK_IRQHandler();
	/* USER CODE BEGIN SysTick_IRQn 1 */

	/* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
 * @brief This function handles RTC global interrupt.
 */
void RTC_IRQHandler(void)
{
	HAL_RTCEx_RTCIRQHandler(&hrtc);
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
}

/**
 * @brief This function handles USB low priority or CAN RX0 interrupts.
 */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

/**
 * @brief This function handles EXTI line3 interrupt.
 */
void EXTI3_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
 * @brief This function handles EXTI line[9:5] interrupts.
 */
void EXTI9_5_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	/* обработка пина питания USBs */
	if (GPIO_Pin == GPIO_PIN_8)
	{
		if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0) && PwrState == 1)
		{
			PwrState = 0;
			sleep = true;
//			HAL_PCD_DevDisconnect(&hpcd_USB_FS);
		}
		else if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) != 0) && PwrState == 0)
		{
			PwrState = 1;
			sleep = false;
		}
	}

	/* обработка замыкания датчика двери */
	if (GPIO_Pin == GPIO_PIN_3)
	{
		for (int i = 0; i < 100000; i++)
			;
		//	{
		uint8_t _st = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

		if (_st)
		{
			if (device.door->STATUS != _st)
			{
				device.door->STATUS = _st;
//				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
				device.door->COUNT++;
				HAL_RTC_GetTime(&hrtc, &device.door->TIME, RTC_FORMAT_BIN);
				HAL_RTC_GetDate(&hrtc, &device.door->DATE, RTC_FORMAT_BIN);
				HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
						(uint8_t*) device.door, sizeof(DOOR_T), 1000);

//					HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
				//todo
#ifdef __DEDUG__
				printf("Открыто door->STATUS %u\n", device.door->STATUS);
#endif
			}

		}
		else
		{
			if (device.door->STATUS != _st)
			{
				device.door->STATUS = _st;
//				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
				//todo
#ifdef __DEDUG__
				printf("Закрыто door->STATUS %u\n", device.door->STATUS);
#endif
			}
		}
//			if (sleep)
//				HAL_SuspendTick();
		//		HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	}
	//}

}
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
