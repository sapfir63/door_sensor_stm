#include "stm32f1xx_hal.h"

#define NEW_FW_FLAG			0x4567
#define PAGE_SIZE			((uint32_t)1024)
//////////////////////////////////
// Корректировать под нужды проекта
#define MAIN_PROGRAM_BASE_PAGE	((uint32_t)5)
#define IMAGE_BASE_PAGE 		((uint32_t)32)
#define IMAGE_PAGE_COUNT		((uint32_t)25)
///////////////////////////////////
// Корректировать не нужно
#define MAIN_PROGRAM_OFFSET		((uint32_t)(MAIN_PROGRAM_BASE_PAGE*PAGE_SIZE))
#define MAIN_PROGRAM_BASE_ADDR	((uint32_t)((uint32_t)0x08000000+MAIN_PROGRAM_OFFSET))
#define IMAGE_BASE_ADDR 		((uint32_t)((uint32_t)0x08000000+IMAGE_BASE_PAGE*PAGE_SIZE))
#define IMAGE_SIZE				(IMAGE_PAGE_COUNT*PAGE_SIZE)
#define MAIN_PROGRAM_MAX_SIZE	((IMAGE_BASE_PAGE-MAIN_PROGRAM_BASE_PAGE)*PAGE_SIZE)
////////////////////////////////////

struct Image
{
	uint32_t lengt;
	uint8_t  data[IMAGE_SIZE-10];
	uint32_t crc;
	uint16_t newFwFlag;
};

const struct Image* image=(struct Image*)IMAGE_BASE_ADDR;

struct VectorTable
{
	uint32_t stackPointer;
	uint32_t resetHandler;
};
typedef void (*func) (void);
#define VECTOR_TABLE	((VectorTable*)(MAIN_PROGRAM_BASE_ADDR))

//
//#define ApplicationAddress    0x08010000                                 //Начало нашего приложения
//typedef  void (*pFunction)(void);
//pFunction Jump_To_Application;
//uint32_t JumpAddress;
//
//if (((*(__IO uint32_t*)ApplicationAddress) & 0x2FFE0000 ) == 0x20000000) //Проверяем, есть ли что-нибудь по адресу (там должно лежать значение SP для приложения, его кладет линкер)
//    {
//      JumpAddress = *(__IO uint32_t*) (ApplicationAddress + 4);  //Адрес перехода из вектора Reset
//      Jump_To_Application = (pFunction) JumpAddress;                 //Указатель на функцию перехода
//      __set_MSP(*(__IO uint32_t*) ApplicationAddress);                //Устанавливаем SP приложения
//      Jump_To_Application();                                                       //Запускаем приложение
//    }
//    while(1);

uint32_t JumpAddress;
func Jump_To_Application;

int main()
{
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
//	HAL_Init();

	// Проверка наличия образа
	if(image->newFwFlag==NEW_FW_FLAG)
	{
	//	FLASH_Status status;
		// Коприруем образ в область основной програмы
		// Готовим FLASH память
		HAL_FLASH_Unlock();
		// ожидаем готовности FLASH
	//	while (FLASH_GetStatus() != FLASH_COMPLETE);
		// Стирание памяти под образ
		// выполняем очистку всех страниц, закреплённых за настройками
		for (uint32_t i=0; i<IMAGE_PAGE_COUNT; i++)
		{
//			do
//			{
//			//	FLASH_Erase_Sector(MAIN_SECTOR_1, FLASH_VOLTAGE_RANGE_3);
//	//			FLASH_PageErase(MAIN_PROGRAM_BASE_ADDR + i*PAGE_SIZE);
//			}
//			while (status != FLASH_COMPLETE);
		}
		uint32_t* flashAddr=(uint32_t*)MAIN_PROGRAM_BASE_ADDR;
		for (uint32_t i=0; i<image->lengt; i+=4)
		{
	//		FLASH_ProgramWord((uint32_t)flashAddr++, *((uint32_t*)(&image->data[i])));
		}
		// Стираем последнюю страницу (вместе с флагом)
	//	FLASH_ErasePage(((uint32_t)&image->newFwFlag)&0xFFFFFF00);

		HAL_FLASH_Lock();
	}

//	HAL_DeInit();
	JumpAddress=*(__IO uint32_t*) (MAIN_PROGRAM_BASE_ADDR + 4);
	Jump_To_Application = (func) JumpAddress;
	SCB->VTOR=MAIN_PROGRAM_OFFSET;
	__set_MSP(*(__IO uint32_t*) MAIN_PROGRAM_BASE_ADDR);
	Jump_To_Application();
	for (;;);
}
