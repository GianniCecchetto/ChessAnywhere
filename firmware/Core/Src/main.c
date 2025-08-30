/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "rgb_led.h"
#include "bitmap.h"
#include "board_com.h"
#include "fifo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// ENUM qui permet de sélectionner le PORT correct correspondant à la ligne ou colonne
typedef enum {
	NO, NE, SO, SE
} Direction;

typedef struct {
	Direction dir;
	uint8_t index;
} Led_Index;

typedef enum {
	ROW0, ROW1, ROW2, COL0, COL1, COL2, READ, BUTTON
} Pins;

typedef enum {
	STARTING_ANIMATION, INIT_BOARD, IN_GAME, GAME_END
} States;

// Permettra de faire un tableau pour savoir quel pin est associé à quel port et numéro de GPIO
typedef struct {
    GPIO_TypeDef *port;   // pointeur vers le bloc GPIO (GPIOA, GPIOB…)
    uint16_t pin;         // Numéro pin
} GPIO_pin;

typedef struct {
	uint8_t column;
	uint8_t line;
} Square;

// Put this define here, so I can use it in the structure
#define UART_BUFFER_CAPACITY		32

typedef struct {
	uint8_t data[UART_BUFFER_CAPACITY]; // capacity
	uint8_t size;
	uint8_t capacity;
} Array;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// DEFINE
#define BOARD_WIDTH							8
#define BOARD_HEIGHT						8
#define PIN_NUMBER_FOR_COLUMN 	3
#define PIN_NUMBER_FOR_LINE	 		3
#define NO_INDEX_FOUND					255
#define GLOBAL_BRIGHTNESS				64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// MACRO
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim17;
DMA_HandleTypeDef hdma_tim17_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// PRIVATE VARIABLE
// LED RGB control variable

static const GPIO_pin gpio_pins[] = {
		{GPIOA, GPIO_PIN_4},  // ROW0
		{GPIOA, GPIO_PIN_5},  // ROW1
		{GPIOA, GPIO_PIN_6},  // ROW2
		{GPIOC, GPIO_PIN_15}, // COL0
		{GPIOA, GPIO_PIN_11}, // COL1
		{GPIOA, GPIO_PIN_12}, // COL2
		{GPIOB, GPIO_PIN_0},  // READ
		{GPIOB, GPIO_PIN_7}   // BUTTON
}; // ROW0
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM17_Init(void);
/* USER CODE BEGIN PFP */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PWM_Send_To_DMA(uint16_t *pwm_data);
void UART_Flush(UART_HandleTypeDef *huart);
void set_gpio_column(uint8_t column);
void set_gpio_line(uint8_t line);
uint8_t read_reed_value(Square square);
void read_full_board(uint64_t *board_bitmap);
uint8_t convert_reed_index_to_led_index(uint8_t reed_index);
uint8_t are_bitamps_the_same(uint64_t bitmap_a, uint64_t bitmap_b);
uint8_t is_a_piece_lift(uint64_t current, uint64_t old);
uint8_t is_a_piece_placed(uint64_t current, uint64_t old);
void led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t colors[][3], uint8_t brightness);
void leds_clear(uint8_t colors[][3]);
void hsv_to_rgb(int h, int s, int v, uint8_t colors[][3], int from_index);
uint8_t is_board_at_init_setup(uint64_t board_bitmap);


/* --- helpers UART --- */
static inline uint32_t t_ms(void){ return HAL_GetTick(); }
static void uart_write(const char *s)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 100);
}
static void uart_write_n(const char *s, size_t n){ HAL_UART_Transmit(&huart2,(uint8_t*)s,(uint16_t)n,100); }

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// GLOBAL VARIABLE
static uint8_t rx_data;                  // Octet reçu
uart_fifo_t uartFifo;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
	// State machine variable
	States 		game_state = STARTING_ANIMATION;
	uint16_t  pwm_data[LED_BUFFER_SIZE] = {0};
	uint8_t   colors[LED_NUMBER][3] = {0};
	// Save the board state for led
  uint64_t  old_board_bitmap = 0;
  uint64_t  board_bitmap = 0;

  char msg[64] = {0};
  char command[128];
  cb_cmd_t cmd;
  uint8_t idx;
  uint8_t status;
  static int hue = 0;

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
  MX_DMA_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */
  // UART_Flush(&huart2);
  // Lancer la réception du premier octet en interruption
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);
  uart_fifo_init(&uartFifo);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  	switch(game_state) {
  	  case STARTING_ANIMATION:
				for(uint8_t j = 0; j < 255; ++j) {

					// Only display when color are everywhere on the colors table
					if(colors[63][0] != 0 || colors[63][1] != 0 || colors[63][2] != 0) {
						// Prepare data for DMA
						rgb_update_buffer(pwm_data, colors);
						HAL_TIM_PWM_Send_To_DMA(pwm_data);
						HAL_Delay(50);
					}
					// Scroll down the color, row per row
					for(int i = 7; i > 0; --i) {
						for(int k = 0; k < 8; ++k) {
							colors[(i)*8+k][0] = colors[(i-1)*8+k][0];
							colors[(i)*8+k][1] = colors[(i-1)*8+k][1];
							colors[(i)*8+k][2] = colors[(i-1)*8+k][2];
						}
					}
					// Generate the new line of rgb
					hsv_to_rgb(hue, 255, 255, colors, 0);
					// Change color
					hue = (hue + 4) % 256;
				}
				// End of init animation
				leds_clear(colors);
  	  	game_state = INIT_BOARD;
  	  	break;
  		case INIT_BOARD:
  			// old_board_bitmap = board_bitmap;
  			// Reading the state of every reed sensors
				read_full_board(&board_bitmap);
				status = is_board_at_init_setup(board_bitmap);
				if(status == 1) {
					/* cb_fmt_evt_lift(msg, 64, idx, HAL_GetTick());
					HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY); */
					game_state = IN_GAME;
				}
				// For all the LED's
				for(uint8_t i = 0; i < LED_NUMBER; ++i){
					// Convert the reed index to the led index, because they aren't not connected the same way (see schematic)
					uint8_t led_index = convert_reed_index_to_led_index(i);
					// Take the bitmap value from the current index
					if(bitmap_get_bit(board_bitmap, i)) {
						// if the sensor is closed, led will be green
						led_set(led_index, 0, 255, 0, colors, GLOBAL_BRIGHTNESS);
					} else {
						// If the sensor is open, led will be red
						led_set(led_index, 255, 0, 0, colors, GLOBAL_BRIGHTNESS);
					}
				}
				// Prepare data for DMA
				rgb_update_buffer(pwm_data, colors);
				HAL_TIM_PWM_Send_To_DMA(pwm_data);
  			break;
  		case IN_GAME:

  			old_board_bitmap = board_bitmap;
				// Reading the state of every reed sensors
				read_full_board(&board_bitmap);
				idx = is_a_piece_lift(board_bitmap, old_board_bitmap);
				if(idx != NO_INDEX_FOUND) {
					cb_fmt_evt_lift(msg, 64, idx, HAL_GetTick());
					HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
				}

				idx = is_a_piece_placed(board_bitmap, old_board_bitmap);
				if(idx != NO_INDEX_FOUND) {
					leds_clear(colors);
					cb_fmt_evt_place(msg, 64, idx, HAL_GetTick());
					HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), HAL_MAX_DELAY);
				}

  			int r = uart_fifo_get_command(&uartFifo, command, sizeof(command));
				if (r > 0) {
						// commande complète reçue (cmd contient la commande sans CR/LF)
					uart_write(command);
					cb_parse_cmd(command, &cmd);
					memset(command, 0, 64);
					switch(cmd.type) {
						case CB_CMD_PING:      uart_write("OK PING\r\n"); break;
						case CB_CMD_VER_Q:     uart_write("OK FW=FW1.0.0 HW=PCBv1\r\n"); break;
						case CB_CMD_TIME_Q:   { char o[48]; int n=snprintf(o,sizeof o,"OK TIME %lu\r\n",(unsigned long)t_ms()); uart_write_n(o,(size_t)n); } break;
						case CB_CMD_RST:       NVIC_SystemReset(); break;
						case CB_CMD_SAVE:      uart_write("OK SAVE\r\n"); break;
						case CB_CMD_STREAM:    /* cmd.u.stream.on */ uart_write("OK STREAM\r\n"); break;

						/* READ */
						case CB_CMD_READ_ALL:  uart_write("OK READ ALL 0x0000000000000000\r\n"); break;
						case CB_CMD_READ_SQ:  { char sq[3]; cb_sq_to_str(cmd.u.read_sq.idx,sq);
																		char o[32]; int n=snprintf(o,sizeof o,"OK READ SQ %s 0\r\n",sq); uart_write_n(o,(size_t)n); } break;
						/* LED */
						case CB_CMD_LED_SET:       led_set(cmd.u.led_set.idx, cmd.u.led_set.r, cmd.u.led_set.g, cmd.u.led_set.b, colors, GLOBAL_BRIGHTNESS); uart_write("OK\r\n"); break;
					//	case CB_CMD_LED_OFF_ALL:   led_off_all(); uart_write("OK\r\n"); break;
					//	case CB_CMD_LED_FILL:      led_fill(cmd.u.led_fill.r, cmd.u.led_fill.g, cmd.u.led_fill.b); uart_write("OK\r\n"); break;
					//	case CB_CMD_LED_BITBOARD:  led_bitboard(cmd.u.led_bitboard.bits); uart_write("OK\r\n"); break;
						case CB_CMD_LED_MAP_HEX:   /* cmd.u.led_map_hex.hex192 */ uart_write("OK\r\n"); break;
						case CB_CMD_LED_MOVES:     uart_write("OK\r\n"); break;
						case CB_CMD_LED_OK:        uart_write("OK\r\n"); break;
						case CB_CMD_LED_FAIL:      uart_write("OK\r\n"); break;

						/* MOVE */
						case CB_CMD_MOVE_ACK:      uart_write("OK\r\n"); break;
						case CB_CMD_MOVE_NACK:     uart_write("OK\r\n"); break;

						/* CFG */
						case CB_CMD_CFG_Q:         uart_write("OK CFG\r\n"); break;
						case CB_CMD_CFG_GET:       uart_write("OK CFG VAL\r\n"); break;
						//case CB_CMD_CFG_SET_KV:    for(int i=0;i<cmd.u.cfg_set_kv.n_pairs;i++) cfg_set_kv(cmd.u.cfg_set_kv.pairs[i]); uart_write("OK\r\n"); break;

						default: uart_write("ERR CMD\r\n"); break;
					}
					rgb_update_buffer(pwm_data, colors);
					HAL_TIM_PWM_Send_To_DMA(pwm_data);
				} else if (r == -1) {
						// commande reçue mais tronquée dans buffer 'cmd'
						// handle_truncated_command(command);
						uart_write(command);
				}
  			break;
  	}

  	// Exemple : traiter des commandes reçues


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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSIDiv = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00B07CB4;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM17 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM17_Init(void)
{

  /* USER CODE BEGIN TIM17_Init 0 */

  /* USER CODE END TIM17_Init 0 */

  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM17_Init 1 */

  /* USER CODE END TIM17_Init 1 */
  htim17.Instance = TIM17;
  htim17.Init.Prescaler = 0;
  htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim17.Init.Period = 39;
  htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim17.Init.RepetitionCounter = 0;
  htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim17) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim17, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim17, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM17_Init 2 */

  /* USER CODE END TIM17_Init 2 */
  HAL_TIM_MspPostInit(&htim17);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */
  // NVIC

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_11
                          |GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin : PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA11
                           PA12 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_11
                          |GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/*
 * Chroma
 */
// Fonction de conversion HSV → RGB
void hsv_to_rgb(int h, int s, int v, uint8_t colors[][3], int from_index) {
  float hh = h / 255.0 * 360;
  float ss = s / 255.0;
  float vv = v / 255.0;

  int i = (int)(hh / 60) % 6;
  float f = (hh / 60) - i;
  float p = vv * (1 - ss);
  float q = vv * (1 - f * ss);
  float t = vv * (1 - (1 - f) * ss);

  float rf, gf, bf;
  switch (i) {
    case 0: rf = vv; gf = t; bf = p; break;
    case 1: rf = q; gf = vv; bf = p; break;
    case 2: rf = p; gf = vv; bf = t; break;
    case 3: rf = p; gf = q; bf = vv; break;
    case 4: rf = t; gf = p; bf = vv; break;
    default: rf = vv; gf = p; bf = q; break;
  }

  int brightness = 64;

  for(uint8_t i = from_index; i < from_index + 8; ++i) {
  	int r = (uint8_t)(rf*255);
  	int g = (uint8_t)(gf*255);
  	int b = (uint8_t)(bf*255);
  	led_set(i, r, g, b, colors, brightness);
  }
}

/* USER CODE BEGIN 4 */
/*
 * Start sending data
 */
void HAL_TIM_PWM_Send_To_DMA(uint16_t *pwm_data)
{
    ws2812_transfer_complete = 0;
    HAL_TIM_PWM_Start_DMA(&htim17, TIM_CHANNEL_1, (uint32_t*)pwm_data, LED_BUFFER_SIZE);

    while(!ws2812_transfer_complete) {}
}

/*
 * DMA Callback
 */
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM17)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        ws2812_transfer_complete = 1;
    }
}

/*
 *
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // Stocke l'octet reçu dans le FIFO RX
    		// putCharInFifo(&usartFifoRx, rx_data);
    		uart_fifo_push_isr(&uartFifo, rx_data);

        // Relance la réception du prochain octet
        HAL_UART_Receive_IT(&huart2, &rx_data, 1);
    }
}

/*
 * Flush the TX UART
 */
void UART_Flush(UART_HandleTypeDef *huart)
{
    // Vider le registre RX tant qu’il reste des données
    __HAL_UART_FLUSH_DRREGISTER(huart);

    // Effacer les flags d’erreur éventuels (Overrun, Framing, Noise, Parity)
    __HAL_UART_CLEAR_OREFLAG(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);

    // Attendre que la transmission en cours (TX) soit terminée
    while(__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) == RESET);

}

uint8_t is_board_at_init_setup(uint64_t board_bitmap) {

	// Verify the white side
	for(uint8_t index = 0; index < 16; ++index) {
		if(bitmap_get_bit(board_bitmap, index) == 0) {
				return 0;
		}
	}

	// Verify the black side
	for(uint8_t index = 48; index < 64; ++index) {
		if(bitmap_get_bit(board_bitmap, index) == 0) {
				return 0;
		}
	}

	return 1;
}

/*
 *
 */
uint8_t is_a_piece_lift(uint64_t current, uint64_t old) {

	for(uint8_t index = 0; index < 64; ++index) {

		// Old state was activ and new state is open
		if(bitmap_get_bit(old, index) == 1 && bitmap_get_bit(current, index) == 0) {
				return index;
		}
	}

	return NO_INDEX_FOUND;
}

/*
 *
 */
uint8_t is_a_piece_placed(uint64_t current, uint64_t old) {

	for(uint8_t index = 0; index < 64; ++index) {

		if(bitmap_get_bit(old, index) == 0 && bitmap_get_bit(current, index) == 1) {
				return index;
		}
	}


	return NO_INDEX_FOUND;
}

/*
 * Compare the two bitmaps
 */
uint8_t are_bitamps_the_same(uint64_t bitmap_a, uint64_t bitmap_b) {

	for(uint8_t index = 0; index < 64; ++index) {
		if(bitmap_get_bit(bitmap_a, index) != bitmap_get_bit(bitmap_b, index)) {
			return index;
		}
	}

	return NO_INDEX_FOUND;
}

/* Set the GPIO column for the decoder
 * Return : void
 */
void set_gpio_column(uint8_t column) {

	uint8_t mask = 1;

	for(uint8_t i = COL0; i < PIN_NUMBER_FOR_COLUMN + COL0; ++i) {
		if(column & mask) {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_RESET);
		}
		mask *= 2;
	}
}

/* Set the GPIO line for the decoder
 * Return void
 */
void set_gpio_line(uint8_t line) {

	uint8_t mask = 1;

	for(uint8_t i = ROW0; i < PIN_NUMBER_FOR_LINE; ++i) {
		if(line & mask) {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_RESET);
		}
		mask *= 2;
	}
}

/* This function read one reed sensor, depending on the selected square
 * Return the : ON or OFF
 */
uint8_t read_reed_value(Square square) {

	// Set the value with the decodeur
	set_gpio_column(square.column);
	set_gpio_line(square.line);

	// Get the value on the READ pin (see schematics)
	return HAL_GPIO_ReadPin(gpio_pins[READ].port, gpio_pins[READ].pin);
}

/* This method will fill the bitmap depending on the reeds sensors states
 * Return : void
 */
void read_full_board(uint64_t *board_bitmap) {

	Square square = {0, 0};
	// For all the board's squares
	for(uint8_t line = 0; line < BOARD_WIDTH; ++line) {
		for(uint8_t column = 0; column < BOARD_HEIGHT; ++column) {
			// Init square (each column and lines)
			square.column = column;
			square.line = line;
			// If the reed sensor is closed, the bit is set
			if(read_reed_value(square)) {
				bitmap_set_bit(board_bitmap, line * BOARD_WIDTH + column);
			} else {
				// If the reed sensor is open, the bit is clear
				bitmap_clear_bit(board_bitmap, line * BOARD_WIDTH + column);
			}

		}
	}
}

/*
 * Clear Led
 */
void leds_clear(uint8_t colors[][3]) {
	for(uint8_t index = 0; index < LED_NUMBER; ++index) {
		colors[index][0] = 0;
		colors[index][1] = 0;
		colors[index][2] = 0;
	}
}

/*
 * set_led
 */
void led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b, uint8_t colors[][3], uint8_t brightness) {
	colors[index][0] = (uint8_t)(r * brightness / 255);
	colors[index][1] = (uint8_t)(g * brightness / 255);
	colors[index][2] = (uint8_t)(b * brightness / 255);
}

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
