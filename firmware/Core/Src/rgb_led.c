#include "rgb_led.h"

// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void rgb_update_buffer(uint16_t *pwm_data, ColorName *colors) {
    for(int led = 0; led < LED_NUMBER; led++)
    {
        uint32_t color = ((uint32_t)colors_values[colors[led]].r << 16) |  // R
        				 ((uint32_t)colors_values[colors[led]].g << 8) |  // G
                         ((uint32_t)colors_values[colors[led]].b << 0);  // R
        for(int i = 0; i < 24; i++)
        {
            if(color & (1 << (23-i)))
            	pwm_data[led*24 + i] = HIGH_DUTY;
            else
            	pwm_data[led*24 + i] = LOW_DUTY;
        }
    }

    // Ajoute les zéros pour le reset (>50µs)
    for(int i = 24*LED_NUMBER; i < LED_BUFFER_SIZE; i++)
    {
    	pwm_data[i] = 0;
    }
}

/* OLD
// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void rgb_update_buffer(uint16_t *pwm_data, uint8_t colors[][3])
{
    for(int led = 0; led < LED_NUMBER; led++)
    {
        uint32_t color = ((uint32_t)colors[led][1] << 16) |  // R
        				 ((uint32_t)colors[led][0] << 8) |  // G
                         ((uint32_t)colors[led][2] << 0);  // R
        for(int i = 0; i < 24; i++)
        {
            if(color & (1 << (23-i)))
            	pwm_data[led*24 + i] = HIGH_DUTY;
            else
            	pwm_data[led*24 + i] = LOW_DUTY;
        }
    }

    // Ajoute les zéros pour le reset (>50µs)
    for(int i = 24*LED_NUMBER; i < LED_BUFFER_SIZE; i++)
    {
    	pwm_data[i] = 0;
    }
}
*/
