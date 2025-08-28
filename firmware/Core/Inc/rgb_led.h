#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void updateBuffer(void);

// -------------------------------------------------------------------
// Start sending data
// -------------------------------------------------------------------
void WS2812_Send(void);


// -------------------------------------------------------------------
// Callback
// -------------------------------------------------------------------
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim);

#endif
