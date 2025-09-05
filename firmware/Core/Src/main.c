/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  * @author					: Thomas Stäheli
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
#include "core_cm0plus.h" // pour __NOP() si dispo
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// Handle all the pins from schematics (same name in the schematic)
typedef enum {
	ROW0, ROW1, ROW2, COL0, COL1, COL2, READ, BUTTON
} Pins;

// Game states
typedef enum {
	INIT_ANIMATION, 			// INIT
	INIT_BOARD, 				 	// Wait until the board is init (32 pieces placed)
	STARTING_ANIMATION,  	// Before start
	IN_GAME, 						 	// Reading UART port for app command
	GAME_END						 	// Handle the restart of a new game
} States;

// Store the differents pins name and port
typedef struct {
	GPIO_TypeDef *port;   // Pointer to the GPIO Block (GPIOA, GPIOB…)
	uint16_t pin;         // Pin number
} GPIO_pin;

// A Matrix Square
typedef struct {
	uint8_t column;
	uint8_t line;
} Square;

// Store all the settings
typedef struct {
	uint8_t grid_brightness;
	uint8_t possible_move_brightness;
	Color   board_theme;
} Settings;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// DEFINE
#define BOARD_WIDTH									8
#define BOARD_HEIGHT								8
#define BOARD_MIDDLE								BOARD_WIDTH * BOARD_HEIGHT / 2
#define BOARD_IS_NOT_READY					0
#define BOARD_IS_READY							1
#define BOARD_DEFAULT_GRID_BRIGHT		255
#define BOARD_DEFAULT_MOVE_BRIGHT   255

#define PIN_NUMBER_FOR_COLUMN 			3
#define PIN_NUMBER_FOR_LINE	 				3
#define NO_INDEX_FOUND							255

#define COLOR_PANNEL_SIZE						256
#define MAX_COMMAND_SIZE						64
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
// MACRO
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim17;
DMA_HandleTypeDef hdma_tim17_ch1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
// PRIVATE VARIABLE
// Pinout
static const GPIO_pin gpio_pins[] = {
	{GPIOA, GPIO_PIN_4},  // ROW0
	{GPIOA, GPIO_PIN_5},  // ROW1
	{GPIOA, GPIO_PIN_6},  // ROW2
	{GPIOC, GPIO_PIN_15}, // COL0
	{GPIOA, GPIO_PIN_11}, // COL1
	{GPIOA, GPIO_PIN_12}, // COL2
	{GPIOB, GPIO_PIN_0},  // READ
	{GPIOB, GPIO_PIN_7}   // BUTTON
};

// To generate the animation
Color color_pannel[COLOR_PANNEL_SIZE];

// Bitmap to display on the 64 leds
uint64_t displayed_numbers[3] = {
		0x1838181818187E00ULL, 	// Number : 1
		0x3C66060C30607E00ULL, 	// Number : 2
		0x3C66061C06663C00ULL		// Number : 3
};

static uint8_t 		 	rx_data;     			// To store the received uart byte
static uart_fifo_t 	uart_fifo;   			// UART FIFO
static Settings 		settings = { 			// User settings
		.grid_brightness = BOARD_DEFAULT_GRID_BRIGHT,							// Grid brightness
		.possible_move_brightness = BOARD_DEFAULT_MOVE_BRIGHT,		// Possibles moves brightness
		.board_theme = {0, 0, 0}					// Grid color
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
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
uint8_t is_a_piece_lift(uint64_t current, uint64_t old);
uint8_t is_a_piece_placed(uint64_t current, uint64_t old);
void generate_frame(uint8_t frame, Color colors[]);
void init_palette();
void led_show_grid(Color colors[]);
void led_show_win(Color colors[], uint8_t side);
void led_show_draw(Color colors[]);
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
	Color     colors[LED_NUMBER] = {0};
	// Save the board state to detect piece lift or place
  uint64_t  old_board_bitmap = 0;
  uint64_t  board_bitmap = 0;
  // UART TX buffer to send command
  char uart_tx_buffer[MAX_COMMAND_SIZE] = {0};
  // To decode UART command
  char string_command[MAX_COMMAND_SIZE];
  cb_cmd_t decoded_command;
  // Reed variable
  uint8_t lift_or_place_index;
  uint8_t status;

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
  MX_USART2_UART_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */
  // UART_Flush(&huart2);
  // Start the interruption on the UART RX bus
  HAL_UART_Receive_IT(&huart2, &rx_data, 1);
  uart_fifo_init(&uart_fifo);
  game_state = INIT_ANIMATION;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  	// States machines
  	switch(game_state) {
  	  case INIT_ANIMATION:
  	  	// Init the color pannel to display
  	  	init_palette();
  	  	// Generate 255 differents to display on the 8x8 matrix
  	  	for(uint8_t frame = 0; frame < 255; ++frame) {
  	  		generate_frame(frame, colors);
  	  		led_update_buffer(pwm_data, colors);
					HAL_TIM_PWM_Send_To_DMA(pwm_data);
  	  		HAL_Delay(20);
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
				if(status == BOARD_IS_READY) {
					// Send board init to app ?
					// HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buffer, strlen(uart_tx_buffer), HAL_MAX_DELAY);
					game_state = STARTING_ANIMATION;
					led_show_grid(colors);
					led_update_buffer(pwm_data, colors);
					HAL_TIM_PWM_Send_To_DMA(pwm_data);
				}

				// For all the LED's
				for(uint8_t i = 0; i < LED_NUMBER; ++i){
					// Convert the reed index to the led index, because they aren't not connected the same way (see schematic)
					uint8_t led_index = convert_reed_index_to_led_index(i);
					// Take the bitmap value from the current index
					if(bitmap_get_bit(board_bitmap, i)) {
						// if the sensor is closed, led will be green
						Color new_color = {0, 255, 0};
						led_set(led_index, new_color, colors, settings.grid_brightness);
					} else {
						// If the sensor is open, led will be red
						Color new_color = {255, 0, 0};
						led_set(led_index, new_color, colors, settings.grid_brightness);
					}
				}

				// TODO DEBUG PURPOSE
			  // game_state = STARTING_ANIMATION;
  			break;
  		case STARTING_ANIMATION:
  			Color white = {255, 255, 255};
  			Color black = {0, 0, 0};
  			// Displaying the countdown on the 8x8 LED matrix : 3, 2, 1
  			for(int8_t index_number = 3; index_number >= 1; --index_number) {
  			    for(uint8_t i = 0; i < BOARD_HEIGHT * BOARD_WIDTH; ++i) {
  			        if(bitmap_get_bit(displayed_numbers[index_number - 1], i)) {
  			            led_set(convert_reed_index_to_led_index(i), white, colors, settings.grid_brightness);
  			        } else {
  			            led_set(convert_reed_index_to_led_index(i), black, colors, settings.grid_brightness);
  			        }
  			    }
  			    led_update_buffer(pwm_data, colors);
  			    HAL_TIM_PWM_Send_To_DMA(pwm_data);
  			    HAL_Delay(1000);
  			}
  			// Game start because init section is done
				led_show_grid(colors);
  			game_state = IN_GAME;
				break;
  		case IN_GAME:
  			// Saving the old state of the bitmap
  			old_board_bitmap = board_bitmap;
				// Reading the state of every reed sensors
				read_full_board(&board_bitmap);
				// Detect if a piece was lifted
				lift_or_place_index = is_a_piece_lift(board_bitmap, old_board_bitmap);
				if(lift_or_place_index != NO_INDEX_FOUND) {
					// Send a UART cmd to trigger the application
					cb_fmt_evt_lift(uart_tx_buffer, 64, lift_or_place_index, HAL_GetTick());
					HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buffer, strlen(uart_tx_buffer), HAL_MAX_DELAY);
				}
				// Detect if a piece was placed
				lift_or_place_index = is_a_piece_placed(board_bitmap, old_board_bitmap);
				if(lift_or_place_index != NO_INDEX_FOUND) {
					led_show_grid(colors);
					// Send a UART cmd to trigger the application
					cb_fmt_evt_place(uart_tx_buffer, 64, lift_or_place_index, HAL_GetTick());
					HAL_UART_Transmit(&huart2, (uint8_t*)uart_tx_buffer, strlen(uart_tx_buffer), HAL_MAX_DELAY);
				}

				// See if there is a command waiting in the uart_fifo
  			int r = uart_fifo_get_command(&uart_fifo, string_command, sizeof(string_command));
				if (r > 0) {
					// Parse the string to get the command and get the command data
					cb_parse_cmd(string_command, &decoded_command);
					// After parse, execute the command
					switch(decoded_command.type) {
					  // General command
						// Ping command (unused)
						case CB_CMD_PING:
							uart_write("OK PING\r\n");
							break;
						// Check the firmware version (unused)
						case CB_CMD_VER_Q:
							uart_write("OK FW=FW1.0.0 HW=PCBv1\r\n");
							break;
						// Get the actual time (unused)
						case CB_CMD_TIME_Q:
							char o[48];
							int n=snprintf(o,sizeof o,"OK TIME %lu\r\n",(unsigned long)t_ms());
							uart_write_n(o,(size_t)n);
							break;
						// Soft Reset (unused)
						case CB_CMD_RST:
							NVIC_SystemReset();
							break;
						// Saving the board state (unused and not implemented)
						case CB_CMD_SAVE:
							uart_write("OK SAVE\r\n");
							break;
						// LED
						// Set the color of a RGB LED
						case CB_CMD_LED_SET:
							Color new_color = {decoded_command.u.led_set.r, decoded_command.u.led_set.g, decoded_command.u.led_set.b};
							led_set(decoded_command.u.led_set.idx, new_color, colors, settings.possible_move_brightness);
							uart_write("OK\r\n");
							break;
						// Set all LED with RGB value = 0 0 0
					  case CB_CMD_LED_OFF_ALL:
					  	led_show_grid(colors);
					  	uart_write("OK\r\n");
					  	break;
					  // WIN
					  case CB_CMD_WIN:
					  	// Show the winning animation
					  	led_show_win(colors, decoded_command.u.led_set.idx);
					  	uart_write("OK\r\n");
					  	game_state = GAME_END;
					  	break;
					  // DRAW
					  case CB_CMD_DRAW:
					  	// Show the draw animation
					  	led_show_draw(colors);
					  	uart_write("OK\r\n");
					  	game_state = GAME_END;
					  	break;
					  // BRIGHTNESS
					  case CB_CMD_LED_BRIGHT:
					  	// Set the global brightness
					  	settings.grid_brightness = decoded_command.u.led_bright.bright;
					  	led_show_grid(colors);

					  	uart_write("OK\r\n");
					  	break;
					  // Color theme
					  case CB_CMD_COLOR_SET:
					  	// Set the color of the outer grid
					  	settings.board_theme.r = decoded_command.u.color_set.r;
					  	settings.board_theme.g = decoded_command.u.color_set.g;
					  	settings.board_theme.b = decoded_command.u.color_set.b;
					  	led_show_grid(colors);
					  	uart_write("OK\r\n");
					  	break;
					  // In case of unknown command
						default:
							uart_write("ERR CMD\r\n");
							break;
					}
				} else if (r == -1) {
						// Trunked command
						uart_write("ERR CMD\r\n");;
				}
  			break;
  		case GAME_END:
  			// Display WIN or DRAW for 3 seconds
  			HAL_Delay(5000);
  			led_show_grid(colors);
  			// Back to init board
  			game_state = INIT_BOARD;
  			break;
  	}

  	// Updating LED throw DMA
  	led_update_buffer(pwm_data, colors);
  	HAL_TIM_PWM_Send_To_DMA(pwm_data);

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
 * UART Callback
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        // Store the received caracter in the UART fifo
    		uart_fifo_push_isr(&uart_fifo, rx_data);

        // Rearm the UART interrupt
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

/*
 * Check if the board is a the init setup (32 pieces placed a the correct spot)
 */
uint8_t is_board_at_init_setup(uint64_t board_bitmap) {

	// Verify the white side
	for(uint8_t index = 0; index < 16; ++index) {
		if(bitmap_get_bit(board_bitmap, index) == 0) {
				return BOARD_IS_NOT_READY;
		}
	}

	// Verify the black side
	for(uint8_t index = 48; index < BOARD_WIDTH * BOARD_HEIGHT; ++index) {
		if(bitmap_get_bit(board_bitmap, index) == 0) {
				return BOARD_IS_NOT_READY;
		}
	}

	return BOARD_IS_READY;
}

/*
 * Check if a piece has been lifted
 */
uint8_t is_a_piece_lift(uint64_t current, uint64_t old) {

	for(uint8_t index = 0; index < BOARD_WIDTH * BOARD_HEIGHT; ++index) {

		// Old state was active and new state is off
		if(bitmap_get_bit(old, index) == 1 && bitmap_get_bit(current, index) == 0) {
				return index;
		}
	}

	return NO_INDEX_FOUND;
}

/*
 * Check if a piece has been placed
 */
uint8_t is_a_piece_placed(uint64_t current, uint64_t old) {

	for(uint8_t index = 0; index < BOARD_WIDTH * BOARD_HEIGHT; ++index) {

		// Old state was off and new state is active
		if(bitmap_get_bit(old, index) == 0 && bitmap_get_bit(current, index) == 1) {
				return index;
		}
	}


	return NO_INDEX_FOUND;
}

/*
 * Set the GPIO column for the decoder
 */
void set_gpio_column(uint8_t column) {

	uint8_t mask = 1;

	for(uint8_t i = COL0; i < PIN_NUMBER_FOR_COLUMN + COL0; ++i) {
		HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_RESET);
	}

	for(uint8_t i = COL0; i < PIN_NUMBER_FOR_COLUMN + COL0; ++i) {
		if(column & mask) {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_SET);
		}
		mask *= 2;
	}
}

/*
 * Set the GPIO line for the decoder
 */
void set_gpio_line(uint8_t line) {

	uint8_t mask = 1;

	for(uint8_t i = ROW0; i < PIN_NUMBER_FOR_LINE; ++i) {
		HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_RESET);
	}

	for(uint8_t i = ROW0; i < PIN_NUMBER_FOR_LINE; ++i) {
		if(line & mask) {
			HAL_GPIO_WritePin(gpio_pins[i].port, gpio_pins[i].pin, GPIO_PIN_SET);
		}
		mask *= 2;
	}
}

/*
 * Small delay to wait for propagation time
 */
static inline void tiny_delay_us(uint32_t us)
{
    // boucle NOP très courte (~ à ajuster). Ou utilise un timer si tu en as un.
    while (us--) { for (volatile int i = 0; i < 16; ++i) __NOP(); }
}

/*
 * Get the state of one reed on the led matrix
 */
uint8_t read_reed_value(Square square)
{
		// Select the correct line and column to read
    set_gpio_column(square.column);
    set_gpio_line(square.line);
    tiny_delay_us(5);  // Tiny delay to wait for propagation time
    // Get the value on the READ pin (see schematics)
    return HAL_GPIO_ReadPin(gpio_pins[READ].port, gpio_pins[READ].pin);
}

/*
 * Fill the bitmap board depending on the reeds sensors states
 */
void read_full_board(uint64_t *board_bitmap) {

	Square square = {0, 0};
	// For all the board's squares
	for(uint8_t column = 0; column < BOARD_WIDTH; ++column) {
		for(uint8_t line = 0; line < BOARD_HEIGHT; ++line) {
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
 * Init the color pannel for the starting animation
 */
void init_palette() {
  for (int i = 0; i < COLOR_PANNEL_SIZE; i++) {
    if (i < 85) {
      color_pannel[i].r = i * 3;
      color_pannel[i].g = 255 - i * 3;
      color_pannel[i].b = 0;
    } else if (i < 170) {
      int j = i - 85;
      color_pannel[i].r = 255 - j * 3;
      color_pannel[i].g = 0;
      color_pannel[i].b = j * 3;
    } else {
      int j = i - 170;
      color_pannel[i].r = 0;
      color_pannel[i].g = j * 3;
      color_pannel[i].b = 255 - j * 3;
    }
  }
}

/*
 * Generate a frame on the led matrix
 */
void generate_frame(uint8_t frame, Color colors[]) {

	uint8_t color_index;

	for (uint8_t y = 0; y < BOARD_WIDTH; ++y) {
    for (uint8_t x = 0; x < BOARD_HEIGHT; ++x) {
      color_index = (x * x + y * y + frame * BOARD_WIDTH) & 0xFF;
      Color color = {color_pannel[color_index].r, color_pannel[color_index].g, color_pannel[color_index].b};
      led_set(convert_reed_index_to_led_index(y * BOARD_WIDTH + x), color, colors, settings.grid_brightness);
    }
  }
}

/*
 * Function that display the grid
 */
void led_show_grid(Color colors[]) {

	Color white  = {255, 255, 255};

	// Set the white part of the grid (half of the board)
	for(uint8_t index = 0; index < BOARD_WIDTH * BOARD_HEIGHT; index+=2) {
		led_set(index, white, colors, settings.grid_brightness);
	}

	// Set all the others squares
	for(uint8_t index = 1; index < BOARD_WIDTH * BOARD_HEIGHT; index+=2) {
		led_set(index, settings.board_theme, colors, settings.grid_brightness);
	}
}

/*
 * Function that display the draw game state
 */
void led_show_win(Color colors[], uint8_t side) {

	// side = 0 => black win || side = 1 => white win
	Color white_side_color = {side ? 0 : 255, side ? 255 : 0, 0};
	Color black_side_color = {side == 0 ? 0 : 255, side == 0? 255 : 0, 0};

	// Fill the LED matrix
	for(uint8_t index = 0; index < BOARD_WIDTH * BOARD_HEIGHT / 2; ++index) {
		led_set(index, white_side_color, colors, settings.grid_brightness);
	}

	for(uint8_t index = BOARD_WIDTH * BOARD_HEIGHT / 2; index < BOARD_WIDTH * BOARD_HEIGHT; ++index) {
		led_set(index, black_side_color, colors, settings.grid_brightness);
	}
}

/*
 * Function that display the draw game state
 */
void led_show_draw(Color colors[]) {

	Color white = {255, 255, 255};

	leds_clear(colors);

	for(uint8_t index = 16; index < 24; ++index) {
		led_set(index, white, colors, settings.grid_brightness);
	}

	for(uint8_t index = 40; index < 48; ++index) {
		led_set(index, white, colors, settings.grid_brightness);
	}
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

