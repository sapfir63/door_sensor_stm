/**
 ******************************************************************************
 * File Name          : main.c
 * Description        : Main program body
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32f1xx_hal.h"
//#include "usb_device.h"
//#include "usbd_cdc_if.h"

#include "init.h"
#include "handler.h"

#ifdef __DEDUG__
extern void initialise_monitor_handles(void);
#endif

int main(void)
{
#ifdef __DEDUG__
	initialise_monitor_handles();
#endif

	SCB->VTOR = ((uint32_t) 0x1400); 			// установка нового указателя на прерывания

	DEVICE_Init();
	DEVICE_Handler();
}
