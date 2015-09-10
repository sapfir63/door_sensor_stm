readme.txt
 Created on: 26 авг. 2015 г.
     Author: develop2

     26/08/15
     
      проект создан на основе 
      базового проекта usb_hal
      
      1. подключил геркон
      2. заменил ИК-диод на обычный диод для проверки срабатывания геркона
      
      3. подключил i2c микросхему памяти
      
      
      01/09/15
      добавил boot_loader
      пока без обновления
   
   05/09/15
    заменил библиотеки на версию 1.2.0
    
    07/09/15
    запустил STOP режим
    в void EXTI9_5_IRQHandler(void)
    идет определение наличия питания от USB
     
        /* обработка пина питания USBs */
    if (GPIO_Pin == GPIO_PIN_8)
    {
        if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) == 0) && PwrState == 1)
        {
            PwrState = 0;
            sleep = true;
            HAL_PCD_DevDisconnect(&hpcd_USB_FS);
        }
        else if ((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_8) != 0) && PwrState == 0)
        {
            PwrState = 1;
            sleep = false;
        }
    }
     
    в void DEVICE_Handler(void) вход и выход из режмиа STOP 
     
        if (sleep)
        {
            /* отключить SysTick */
            HAL_SuspendTick();
            /* Enters Stop mode */
            HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

            /* выход из STOPMode и инициализация USB */
            /* включить SysTick */
            HAL_ResumeTick();
            /* Configure the system clock */
            SystemClock_Config();

            HAL_PCD_DevConnect(&hpcd_USB_FS);

        }
     
     
   