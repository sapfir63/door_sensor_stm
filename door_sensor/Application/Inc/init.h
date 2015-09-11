/*
 * init.h
 *
 *  Created on: 27 авг. 2015 г.
 *      Author: develop2
 */

#ifndef APPLICATION_INC_INIT_H_
#define APPLICATION_INC_INIT_H_

#include <stdint.h>

#define EEPROM_MARKER		(uint32_t)(0x23856634)
#define INIT_EEPROM			(0) 			// код инициализации EEPROM 4 байта
#define EEPROM_CODE			(0xa0) 			// код инициализации EEPROM 4 байта

#define __RTC__								// флаг подключения RTC
//#define __DEDUG__
/*
 * Параметры прошивки
 */
typedef struct
{
	uint32_t UPDATE;						// флаг обновления
	uint32_t SIZE;							// размер новой прошивки
	uint32_t CRC32;							// crc 32 новой прошивки
	char VERSION[5];						// версия ПО
} FIRMWARE_T;

typedef struct settings_t
{
	FIRMWARE_T firmware;					// параметры прошивки
	char DEVICE[15];						// навзвание
} SETTINGS_T;

typedef struct door_t
{
	uint32_t COUNT;							// кол-во вскрытий корпуса
	RTC_TimeTypeDef TIME;					// время последнего вскрытия корпуса
	RTC_DateTypeDef DATE;					// дата последнего вскрытия корпуса
	uint8_t STATUS;							// текущий статус датчика(замкнут/разомкнут)
} DOOR_T;

typedef struct device_t
{
	SETTINGS_T *settings;
	DOOR_T *door;
} DEVICE_T;


void DEVICE_Init(void);
void SystemClock_Config(void);

#endif /* APPLICATION_INC_INIT_H_ */
