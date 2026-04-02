/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
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
#include "rtc.h"

/* USER CODE BEGIN 0 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

RTC_TimeTypeDef sTime = {0};
RTC_DateTypeDef sDate = {0};

/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

//  RTC_TimeTypeDef sTime = {0};
//  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255; // 249 LSI
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  //sTime.Hours = 10;
  //sTime.Minutes = 48;
  //sTime.Seconds = 30;
  //sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  //sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  //if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  //{
  //  Error_Handler();
  //}
  //sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
  //sDate.Month = RTC_MONTH_SEPTEMBER;
  //sDate.Date = 11;
  //sDate.Year = 24;
  //if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  //{
  //  Error_Handler();
  //}
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

RTC_TimeTypeDef current_time()
{
		HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
		return sTime;
}
RTC_DateTypeDef current_date()
{
		HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN); // RTC_FORMAT_BIN , RTC_FORMAT_BCD
		return sDate;
}

void update_time(RTC_TimeTypeDef youTime)
{
		sTime.Hours          = youTime.Hours;
		sTime.Minutes        = youTime.Minutes;
		sTime.Seconds        = youTime.Seconds;
		sTime.DayLightSaving = youTime.DayLightSaving;
		sTime.StoreOperation = youTime.StoreOperation;
	
		if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
		{
				Error_Handler();
		}
}
void update_date(RTC_DateTypeDef youDate)
{
		sDate.WeekDay = youDate.WeekDay;
		sDate.Month   = youDate.Month;
		sDate.Date    = youDate.Date;
		sDate.Year    = youDate.Year;
	
		if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
		{
				Error_Handler();
		}
}

		/*
		RTC_TimeTypeDef newTime = {0};
		RTC_DateTypeDef newDate = {0};

		newTime.Hours = 9;
		newTime.Minutes = 55;
		newTime.Seconds = 0;
		newTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		newTime.StoreOperation = RTC_STOREOPERATION_RESET;
		update_time(newTime);
		
		newDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
		newDate.Month = RTC_MONTH_SEPTEMBER;
		newDate.Date = 11;
		newDate.Year = 24;
		update_date(newDate);
		
		myTime = current_time();
		myDate = current_date();
		snprintf(trans_str, 64, "Time %d:%d:%d\n", myTime.Hours, myTime.Minutes, myTime.Seconds);
		HAL_UART_Transmit(&huart3, (uint8_t*)trans_str, strlen(trans_str), 1000);
		snprintf(trans_str, 64, "Date %d-%d-20%d\n", myDate.Date, myDate.Month, myDate.Year);
		HAL_UART_Transmit(&huart3, (uint8_t*)trans_str, strlen(trans_str), 1000);
		
		|---------------------------------------------------------------|
		|31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16 | 
		|X   X   X   X   X   X   X   X   X   PM  HT  HT  HU  HU  HU  HU |
		|---------------------------------------------------------------|
		|15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00 |
		|X   MNT MNT MNT MNU MNU MNU MNU X   ST  ST  ST  SU  SU  SU  SU |
		|---------------------------------------------------------------|
		Бит  22. PM: AM/PM – значение  (0: AM или 24-часовой формат, 1:PM)
		Биты 21:20  HT[1:0]  Десятки часов  в BCD формате
		Биты 19:16  HU[3:0]  Единицы часов  в BCD формате
		Биты 14:12  MNT[2:0] Десятки минут  в BCD формате
		Биты 11:8   MNU[3:0] Единицы минут  в BCD формате
		Биты 6:4    ST[2:0]  Десятки секунд в BCD формате
		Биты 3:0    SU[3:0]  Единицы секунд в BCD формате
		
		|---------------------------------------------------------------|
		|31  30  29  28  27  26  25  24  23  22  21  20  19  18  17  16 | 
		|X   X   X   X   X   X   X   X   YT  YT  YT  YT  YU  YU  YU  YU |
		|---------------------------------------------------------------|
		|15  14  13  12  11  10  09  08  07  06  05  04  03  02  01  00 |
		|WDU WDU WDU MT  MU  MU  MU  MU  X   X   DT  DT  DU  DU  DU  DU |
		|---------------------------------------------------------------|	
		Биты 23:20 YT[3:0] 	Десятки лет в BCD формате
		Биты 19:16 YU[3:0] 	Единицы лет в BCD формате
		Биты 15:13 WDU[2:0] День недели
                               - 000: не используется
                               - 001: понедельник
                               …
                               -111: воскресенье
		Бит 12     MT      Десятки месяца в BCD формате
		Биты 11:8  MU[3:0] Единицы месяца в BCD формате
		Биты 5:4   DT[1:0] Десятки даты в BCD формате
		Биты 3:0   DU[3:0] Единицы даты в BCD формате		

		                                               
		uint32_t t = RTC->TR;
		uint32_t d = RTC->DR;
							
		USART3_Write((uint8_t)(t >> 24));
		USART3_Write((uint8_t)(t >> 16));
		USART3_Write((uint8_t)(t >> 8));
		USART3_Write((uint8_t)(t >> 0));
							
		USART3_Write((uint8_t)(d >> 24));
		USART3_Write((uint8_t)(d >> 16));
		USART3_Write((uint8_t)(d >> 8));
		USART3_Write((uint8_t)(d >> 0));
		*/

/* USER CODE END 1 */
