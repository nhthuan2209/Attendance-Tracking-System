/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "st7789.h"
#include "rc522.h"
#include "fonts.h"
#include "string.h"
#include "stdio.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct 
{
	uint8_t uid[4];
	char name[30];
	uint16_t count_card_input;
	uint8_t in_out_state;
	char time_in[10];
	char time_out[10];
}Human_Card;

Human_Card list_user[] = {
	{{0x1A,0x01,0x02,0x02}, "Nguyen Minh", 0, 0}, {{0x07,0xF6,0x29,0x07}, "Nguyen Hoc", 0, 0}
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define ST7789_RST_PORT 			GPIOB
#define ST7789_RST_PIN  			GPIO_PIN_1
#define ST7789_DC_PORT  			GPIOB
#define ST7789_DC_PIN   			GPIO_PIN_0

#define ST7789_CS_PORT  			GPIOA
#define ST7789_CS_PIN   			GPIO_PIN_4
#define FONT_MENU							Font_16x26
#define LETTER_MENU						WHITE
#define BACKGROUND_MENU				BLACK
#define TIME_DATE_FONT				Font_11x18 
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

/* USER CODE BEGIN PV */
uint8_t uid[4];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Real_Time_Waiting()
{
	RTC_TimeTypeDef realtime;
	RTC_DateTypeDef realdate;
	
	char time_save[20];
	char date_save[20];
	
	HAL_RTC_GetTime(&hrtc, &realtime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &realdate, RTC_FORMAT_BIN);
	
	sprintf(time_save, "%02d:%02d:%02d", realtime.Hours, realtime.Minutes, realtime.Seconds);
	sprintf(date_save, "%02d/%02d/20%02d", realdate.Date, realdate.Month, realdate.Year);
	
	ST7789_WriteString(25, 130, time_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
	ST7789_WriteString(180, 130, date_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
}

void Cham_Cong_Menu()
{
	ST7789_WriteString(32, 20,"SMART ATTENDANCE", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	ST7789_DrawLine(25, 45, 290, 45, LETTER_MENU);
	ST7789_DrawLine(25, 50, 290, 50, LETTER_MENU);
	
	ST7789_WriteString(55, 60, "PLEASE SCAN", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	ST7789_WriteString(55, 95, " YOUR CARD", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	
	Real_Time_Waiting();
}
int8_t Find_Card(uint8_t *uid)
{
	for(int i = 0; i < 2; i++)
	{
		if(list_user[i].uid[0] == uid[0] && list_user[i].uid[1] == uid[1] && list_user[i].uid[2] == uid[2] && list_user[i].uid[3] == uid[3])
		{
			return i;
		}
	}
	return -1;
}
uint8_t Read_Card(void)
{
	if(MFRC522_Request(PICC_REQIDL, uid) == MI_OK)
	{
		if(MFRC522_Anticoll(uid) == MI_OK)
		{
			return 1;
		}
	}
	return 0;
}
void Infor_Card(int8_t card_input)
{
	list_user[card_input].count_card_input++;
	if(list_user[card_input].in_out_state == 0)
	{
		list_user[card_input].in_out_state = 1;
		ST7789_Fill_Color(BLACK);
		ST7789_WriteString(32, 20,"CHECK IN SUCCESS", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
		
		char name_save[30];
		char count_save[10];
		RTC_TimeTypeDef time;
		char time_save[20];
		HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
		
		sprintf(name_save, "%s", list_user[card_input].name);
		sprintf(count_save, "%d", list_user[card_input].count_card_input);
		sprintf(time_save, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
		sprintf(list_user[card_input].time_in, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
		
		ST7789_WriteString(25, 45, "Name: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(100, 45, name_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 70, "Time in: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(140, 70, list_user[card_input].time_in, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 95, "Time out: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(140, 95, list_user[card_input].time_out, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 120, "Trang thai: Da vao", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 145, "So lan cham cong: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(240, 145, count_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
	}
	else
	{
		list_user[card_input].in_out_state = 0;
		ST7789_Fill_Color(BLACK);
		ST7789_WriteString(32, 20,"CHECK IN SUCCESS", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
		
		char name_save[30];
		char count_save[10];
		RTC_TimeTypeDef time;
		char time_save[20];
		HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
		
		sprintf(name_save, "%s", list_user[card_input].name);
		sprintf(count_save, "%d", list_user[card_input].count_card_input);
		sprintf(time_save, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
		sprintf(list_user[card_input].time_out, "%02d:%02d:%02d", time.Hours, time.Minutes, time.Seconds);
		
		ST7789_WriteString(25, 45, "Name: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(100, 45, name_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 70, "Time in: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(140, 70, list_user[card_input].time_in, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 95, "Time out: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(140, 95, list_user[card_input].time_out, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 120, "Trang thai: Da ra", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		
		ST7789_WriteString(25, 145, "So lan cham cong: ", TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
		ST7789_WriteString(240, 145, count_save, TIME_DATE_FONT, LETTER_MENU, BACKGROUND_MENU);
	}
}
void Not_Worker_In_Company(void)
{
	ST7789_Fill_Color(BLACK);
	ST7789_WriteString(32, 20,"CHECK IN FAILED", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	ST7789_DrawLine(25, 45, 290, 45, LETTER_MENU);
	ST7789_DrawLine(25, 50, 290, 50, LETTER_MENU);
	
	ST7789_WriteString(30, 70,"THE KHONG HOP LE", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	ST7789_WriteString(30, 95,"VUI LONG THU LAI", FONT_MENU, LETTER_MENU, BACKGROUND_MENU);
	ST7789_InvertColors(CYAN);
}

void Check_Real_Card()
{
	if(Read_Card())
	{
		int8_t card_check = Find_Card(uid);
		if(card_check >= 0)
		{
			Infor_Card(card_check);
			HAL_Delay(2000);
			ST7789_Fill_Color(BLACK);
			Cham_Cong_Menu();
		}
		else
		{
			Not_Worker_In_Company();
			HAL_Delay(2000);
			ST7789_Fill_Color(BLACK);
			ST7789_InvertColors(RED);
			Cham_Cong_Menu();		
		}
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
	ST7789_Fill_Color(BLACK);
	ST7789_Init();
	MFRC522_Init();
	Cham_Cong_Menu();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		Real_Time_Waiting();
		Check_Real_Card();
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x15;
  sTime.Minutes = 0x30;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_AUGUST;
  DateToUpdate.Date = 0x12;
  DateToUpdate.Year = 0x26;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0x0;
  sAlarm.AlarmTime.Minutes = 0x0;
  sAlarm.AlarmTime.Seconds = 0x0;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA3 PA4 PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
