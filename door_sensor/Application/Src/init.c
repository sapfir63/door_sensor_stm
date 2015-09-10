/*
 * init.c
 *
 *  Created on: 27 авг. 2015 г.
 *      Author: develop2
 */
#include <stdbool.h>

#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "init.h"


RTC_HandleTypeDef hrtc;

I2C_HandleTypeDef hi2c1;

PCD_HandleTypeDef hpcd_USB_FS;
uint32_t count = 0;

volatile bool sleep = false;


FIRMWARE_T firmware;
SETTINGS_T settings;
DOOR_T door;

DEVICE_T device =
	{ &settings, &door };

static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
#ifdef __RTC__
static void MX_RTC_Init(void);
#endif

uint8_t lastDoorState;
uint8_t PwrState;


void DEVICE_Init(void)
{
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_RTC_Init();

	MX_USB_DEVICE_Init();

	strcpy(device.settings->firmware.VERSION, "1.00");
	strcpy(device.settings->DEVICE, "door_sensor");
	uint8_t buff[20];
	uint32_t marker_eeprom = EEPROM_MARKER;
//	HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4,
//	I2C_MEMADD_SIZE_8BIT, (uint8_t*) &count, 4, 1000);
	device.door->COUNT = 0;
	HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM, I2C_MEMADD_SIZE_8BIT, (uint8_t*) buff, 4,
			1000);

//	HAL_I2C_Master_Receive(&hi2c1, 0xa0, (uint8_t*) buff, 4, 10000);
	uint32_t bf = (buff[3] << 24) + (buff[2] << 16) + (buff[1] << 8) + buff[0];
	uint8_t n = memcmp((uint8_t*) (&bf), (uint8_t*) (&marker_eeprom), 4);
	if (n) //нет записи
	{
		//count = 0;
		//	HAL_I2C_Master_Transmit(&hi2c1, 0xa0, (uint8_t*) &marker_eeprom, 4, 10000);
		HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM,
		I2C_MEMADD_SIZE_8BIT, (uint8_t*) &marker_eeprom, 4, 1000);
		HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4,
		I2C_MEMADD_SIZE_8BIT, (uint8_t*) &device.door->COUNT, 4, 1000);
	}
	else
	{
		HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
				(uint8_t*) device.door, sizeof(DOOR_T), 1000);

//		HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4,
//		I2C_MEMADD_SIZE_8BIT, (uint8_t*) &device.door->COUNT, 4, 1000);
//		HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 8, I2C_MEMADD_SIZE_8BIT,
//				(uint8_t*) &device.door->TIME, 3, 1000);
//		HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 11, I2C_MEMADD_SIZE_8BIT,
//				(uint8_t*) &device.door->DATE, 4, 1000);

	}

    /* Enable Power Control clock */
  //  __HAL_RCC_PWR_CLK_ENABLE();


	lastDoorState = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);

	/* проверка подключения к USB */
	PwrState = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8);

	if (PwrState == 0)
		sleep= true;
	else
		HAL_PCD_DevConnect(&hpcd_USB_FS);


}

/** System Clock Configuration
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_PeriphCLKInitTypeDef PeriphClkInit;

#ifdef __RTC__
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
#else
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;

#endif
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

//	  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_USB;
//	/*  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;*/
#ifdef __RTC__
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USB;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
#else
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
#endif

	PeriphClkInit.UsbClockSelection = RCC_USBPLLCLK_DIV1_5;
	HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);

	HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
}

/* RTC init function */
static void MX_RTC_Init(void)
{

	RTC_TimeTypeDef sTime;
	RTC_DateTypeDef DateToUpdate;

	/**Initialize RTC and set the Time and Date
	 */
	hrtc.Instance = RTC;
	hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
	hrtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
	/*hrtc.*/DateToUpdate.WeekDay = RTC_WEEKDAY_THURSDAY;
	/*hrtc.*/DateToUpdate.Month = RTC_MONTH_SEPTEMBER;
	/*hrtc.*/DateToUpdate.Date = 1;
	/*hrtc.*/DateToUpdate.Year = 15;
	HAL_RTC_Init(&hrtc);

	sTime.Hours = 12;
	sTime.Minutes = 59;
	sTime.Seconds = 59;
	HAL_RTC_SetTime(&hrtc, &sTime, FORMAT_BIN);

	HAL_RTC_SetDate(&hrtc, &DateToUpdate, FORMAT_BIN);
	/* запускаем прерывание таймера */
//	HAL_RTCEx_SetSecond_IT(&hrtc);
}
/** Configure pins as
 * Analog
 * Input
 * Output
 * EVENT_OUT
 * EXTI
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE()
	;
	__GPIOD_CLK_ENABLE()
	;
	__GPIOA_CLK_ENABLE()
	;
	__GPIOB_CLK_ENABLE()
	;


	/* иницализация порта для опроса замыкания/размыкания геркона */
	GPIO_InitStruct.Pin = GPIO_PIN_3;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
//	GPIO_InitStruct.Pull = GPIO_NOPULL ;//PULLDOWN;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI3_IRQn);

	/* иницализация индикаторного светодиода */
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* иницализация пина определения USB питания */
	/*Configure GPIO pin : PB8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);


	HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

/* I2C1 init function */
static void MX_I2C1_Init(void)
{
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLED;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLED;
	HAL_I2C_Init(&hi2c1);

}
