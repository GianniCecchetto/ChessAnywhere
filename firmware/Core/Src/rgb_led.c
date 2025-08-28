// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void updateBuffer(void)
{
    for(int led = 0; led < LED_NUMBER; led++)
    {
        uint32_t color = ((uint32_t)colors[led][1] << 16) |  // R
        				 ((uint32_t)colors[led][0] << 8) |  // G
                         ((uint32_t)colors[led][2] << 0);  // R
        for(int i = 0; i < 24; i++)
        {
            if(color & (1 << (23-i)))
                pwmData[led*24 + i] = HIGH_DUTY;
            else
                pwmData[led*24 + i] = LOW_DUTY;
        }
    }

    // Ajoute les zéros pour le reset (>50µs)
    for(int i = 24*LED_NUMBER; i < LED_BUFFER_SIZE; i++)
    {
        pwmData[i] = 0;
    }
}

// -------------------------------------------------------------------
// Start sending data
// -------------------------------------------------------------------
void WS2812_Send(void)
{
    ws2812_transfer_complete = 0;
    HAL_TIM_PWM_Start_DMA(&htim17, TIM_CHANNEL_1, (uint32_t*)pwmData, LED_BUFFER_SIZE);

    while(!ws2812_transfer_complete) {}
}


// -------------------------------------------------------------------
// Callback
// -------------------------------------------------------------------
void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == TIM17)
    {
        HAL_TIM_PWM_Stop_DMA(htim, TIM_CHANNEL_1);
        ws2812_transfer_complete = 1;
    }
}
