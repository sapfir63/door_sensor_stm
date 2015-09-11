<<<<<<< HEAD
/*
 * handler.c
 *
 *  Created on: 27 авг. 2015 г.
 *      Author: develop2
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "init.h"
#include "handler.h"

extern PCD_HandleTypeDef hpcd_USB_FS;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

extern DEVICE_T device;

extern uint8_t UserRxBufferFS[];
extern uint8_t UserTxBufferFS[];
//extern uint32_t count;

extern bool sleep;

struct SettingsPacket
{
	uint8_t startByte;
	uint8_t command;
	uint8_t data[16];
};

enum Commands
{
	READ_ID = 0x01, 		//
	SET_TIME = 0x02, 		//
	GET_TIME = 0x03, 		//
	GET_DOOR_EVENT = 0x04,	//
	RESET_DOOR_EVENT = 0x05, //

	START_DOWNLOADER = 0x41, 		//
	STOP_DOWNLOADER = 0x42,			//
	WRITE_IMAGE_BLOCK = 0x43, 		//
	IMAGE_CRC = 0x44,				//
	SOFTWARE_RESET = 0x50,			//
};

//static GPIO_PinState state = 0;

/* USER CODE END 0 */
struct SettingsPacket settingsPacket;

void USB_Handler(uint8_t* Buf, uint32_t *Len);

void DEVICE_Handler(void)
{
	char buf[10];
	static uint8_t _start = 1;
//	device.door->NEW = 1;

	while (1)
	{
		if (sleep)
		{
//			HAL_PCD_DevDisconnect(&hpcd_USB_FS);
			/* отключить SysTick */
			HAL_SuspendTick();

			/* Enters Stop mode */
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

			/* выход из STOPMode и инициализация USB */
			/* включить SysTick */
			HAL_ResumeTick();
			/* Configure the system clock */
			SystemClock_Config();

//			HAL_PCD_DevConnect(&hpcd_USB_FS);
		}
		else
		{
//			HAL_Delay(200);
//			printf("test\n");
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
		}
	}
}

void USB_Handler(uint8_t* Buf, uint32_t *Len)
{
	int sz;
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;
	if (UserRxBufferFS[0] == 0xAB)
	{
		settingsPacket.startByte = UserRxBufferFS[0];
		settingsPacket.command = UserRxBufferFS[1];
		for (int i = 0; i < 16; i++)
			settingsPacket.data[i] = UserRxBufferFS[i + 2];
		switch (settingsPacket.command)
		{
			case READ_ID:
				settingsPacket.data[0] = 1;
				strcpy((char *) &settingsPacket.data[1],
						(char *) &device.settings->firmware.VERSION);
				break;
			case SET_TIME:
				sDate.Year = settingsPacket.data[0];
				sDate.Month = settingsPacket.data[1];
				sDate.Date = settingsPacket.data[2];
				sTime.Hours = settingsPacket.data[3];
				sTime.Minutes = settingsPacket.data[4];
				sTime.Seconds = settingsPacket.data[5];
				settingsPacket.data[0] = 1;
#ifdef __RTC__
				HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

#endif
				break;
			case GET_DOOR_EVENT:
				settingsPacket.data[0] = 1;
				HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
						(uint8_t*) device.door, sizeof(DOOR_T), 1000);
				device.door->STATUS = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
				settingsPacket.data[1] = device.door->STATUS;
#ifdef __DEDUG__
				//		printf("door->STATUS %u\n", device.door->STATUS);
#endif

				settingsPacket.data[2] = device.door->COUNT;
				settingsPacket.data[3] = device.door->TIME.Hours;
				settingsPacket.data[4] = device.door->TIME.Minutes;
				settingsPacket.data[5] = device.door->TIME.Seconds;
				settingsPacket.data[6] = device.door->DATE.Date;
				settingsPacket.data[7] = device.door->DATE.Month;
				settingsPacket.data[8] = device.door->DATE.Year;

				break;

//			case READ_VERSION:
//				settingsPacket.data[0] = 1;
//				strcpy(&settingsPacket.data[1], &device.settings->firmware.VERSION[0]);
//				break;

			case START_DOWNLOADER:
				settingsPacket.data[0] = 1;
#ifdef __DEDUG__
				printf("START_DOWNLOADER\n");
#endif

				//		strcpy(&settingsPacket.data[1], &device.settings->firmware.VERSION[0]);
				break;

			case WRITE_IMAGE_BLOCK:
				settingsPacket.data[0] = 1;
				//		strcpy(&settingsPacket.data[1], &device.settings->firmware.VERSION[0]);
#ifdef __DEDUG__
				//		printf("WRITE_IMAGE_BLOCK\n");;
#endif
				break;

			case STOP_DOWNLOADER:
				settingsPacket.data[0] = 1;
#ifdef __DEDUG__
				printf("STOP_DOWNLOADER\n");
#endif
				//		strcpy(&settingsPacket.data[1], &device.settings->firmware.VERSION[0]);
				break;

			case RESET_DOOR_EVENT:
				memset(device.door, 0, sizeof(DOOR_T));
				HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
						(uint8_t*) device.door, sizeof(DOOR_T), 1000);
//				HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
//						(uint8_t*) device.door, sizeof(DOOR_T), 1000);

				break;
			case SOFTWARE_RESET:
				HAL_NVIC_SystemReset();

				break;
		}

//	for (int i = 0; i < *Len; i++)
//		UserTxBufferFS[i] = UserRxBufferFS[i];
//
//	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, &UserTxBufferFS[0], *Len);
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*) &settingsPacket, sizeof(settingsPacket));
		USBD_CDC_TransmitPacket(&hUsbDeviceFS);

		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &UserRxBufferFS[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);
//		if (settingsPacket.command == STOP_DOWNLOADER)
//		{
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
//		//	HAL_Delay(4000);
//			for(uint64_t i = 0; i < 0xfffffffff;i++);
//			/* перезагрузка */
//			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
//
//		}
	}
	else
	{
		settingsPacket.startByte = UserRxBufferFS[0];
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*) &settingsPacket, 1);
		USBD_CDC_TransmitPacket(&hUsbDeviceFS);
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &UserRxBufferFS[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	}
}

=======
/*
 * handler.c
 *
 *  Created on: 27 авг. 2015 г.
 *      Author: develop2
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "stm32f1xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "init.h"
#include "handler.h"

extern PCD_HandleTypeDef hpcd_USB_FS;
extern I2C_HandleTypeDef hi2c1;
extern RTC_HandleTypeDef hrtc;

extern DEVICE_T device;

extern uint8_t UserRxBufferFS[];
extern uint8_t UserTxBufferFS[];
//extern uint32_t count;

extern bool sleep;

struct SettingsPacket
{
	uint8_t startByte;
	uint8_t command;
	uint8_t data[16];
};

enum Commands
{
	READ_ID = 0x01, 		//
	SET_TIME = 0x02, 		//
	GET_TIME = 0x03, 		//
	GET_DOOR_EVENT = 0x04,	//
	RESET_DOOR_EVENT = 0x05, //

	WRITE_SETTINGS = 0x10, READ_SETTINGS = 0x11, READ_VERSION = 0x12, READ_COUNTERS = 0x22,
	START_DOWNLOADER = 0x41, STOP_DOWNLOADER = 0x42, WRITE_IMAGE_BLOCK = 0x43, IMAGE_CRC = 0x44,
	SOFTWARE_RESET = 0x50,
};

//static GPIO_PinState state = 0;

/* USER CODE END 0 */
struct SettingsPacket settingsPacket;

void USB_Handler(uint8_t* Buf, uint32_t *Len);

void DEVICE_Handler(void)
{
	char buf[10];
	static uint8_t _start = 1;
//	device.door->NEW = 1;

	while (1)
	{
		if (sleep)
		{
//			HAL_PCD_DevDisconnect(&hpcd_USB_FS);
			/* отключить SysTick */
			HAL_SuspendTick();

			/* Enters Stop mode */
			HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

			/* выход из STOPMode и инициализация USB */
			/* включить SysTick */
			HAL_ResumeTick();
			/* Configure the system clock */
			SystemClock_Config();

//			HAL_PCD_DevConnect(&hpcd_USB_FS);
		}
		else
		{
//			HAL_Delay(200);
//			printf("test\n");
//			HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
		}
	}
}

void USB_Handler(uint8_t* Buf, uint32_t *Len)
{
	int sz;
	RTC_DateTypeDef sDate;
	RTC_TimeTypeDef sTime;
	if (UserRxBufferFS[0] == 0xAB)
	{
		settingsPacket.startByte = UserRxBufferFS[0];
		settingsPacket.command = UserRxBufferFS[1];
		for (int i = 0; i < 16; i++)
			settingsPacket.data[i] = UserRxBufferFS[i + 2];
		switch (settingsPacket.command)
		{
			case READ_ID:
				settingsPacket.data[0] = 1;
				strcpy((char *) &settingsPacket.data[1],
						(char *) &device.settings->firmware.VERSION);
				break;
			case SET_TIME:
				sDate.Year = settingsPacket.data[0];
				sDate.Month = settingsPacket.data[1];
				sDate.Date = settingsPacket.data[2];
				sTime.Hours = settingsPacket.data[3];
				sTime.Minutes = settingsPacket.data[4];
				sTime.Seconds = settingsPacket.data[5];
				settingsPacket.data[0] = 1;
#ifdef __RTC__
				HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
				HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

#endif
				break;
			case GET_DOOR_EVENT:
				settingsPacket.data[0] = 1;
				HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
						(uint8_t*) device.door, sizeof(DOOR_T), 1000);
				device.door->STATUS = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3);
				settingsPacket.data[1] = device.door->STATUS;
#ifdef __DEDUG__
				printf("door->STATUS %u\n", device.door->STATUS);
#endif

				settingsPacket.data[2] = device.door->COUNT;
				settingsPacket.data[3] = device.door->TIME.Hours;
				settingsPacket.data[4] = device.door->TIME.Minutes;
				settingsPacket.data[5] = device.door->TIME.Seconds;
				settingsPacket.data[6] = device.door->DATE.Date;
				settingsPacket.data[7] = device.door->DATE.Month;
				settingsPacket.data[8] = device.door->DATE.Year;

				break;

			case READ_VERSION:
				settingsPacket.data[0] = 1;
				strcpy(&settingsPacket.data[1], &device.settings->firmware.VERSION[0]);
				break;

			case RESET_DOOR_EVENT:
				memset(device.door, 0, sizeof(DOOR_T));
				HAL_I2C_Mem_Write(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
						(uint8_t*) device.door, sizeof(DOOR_T), 1000);
//				HAL_I2C_Mem_Read(&hi2c1, EEPROM_CODE, INIT_EEPROM + 4, I2C_MEMADD_SIZE_8BIT,
//						(uint8_t*) device.door, sizeof(DOOR_T), 1000);

							break;
		}

//	for (int i = 0; i < *Len; i++)
//		UserTxBufferFS[i] = UserRxBufferFS[i];
//
//	USBD_CDC_SetTxBuffer(&hUsbDeviceFS, &UserTxBufferFS[0], *Len);
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*) &settingsPacket, sizeof(settingsPacket));
		USBD_CDC_TransmitPacket(&hUsbDeviceFS);

		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &UserRxBufferFS[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	}
	else
	{
		settingsPacket.startByte = UserRxBufferFS[0];
		USBD_CDC_SetTxBuffer(&hUsbDeviceFS, (uint8_t*) &settingsPacket, 1);
		USBD_CDC_TransmitPacket(&hUsbDeviceFS);
		USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &UserRxBufferFS[0]);
		USBD_CDC_ReceivePacket(&hUsbDeviceFS);
	}
}

>>>>>>> master
