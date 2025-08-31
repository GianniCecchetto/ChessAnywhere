#include "rgb_led.h"

<<<<<<< HEAD
/*
 * Fill the buffer PWM depending on the table "colors"
 */
void led_update_buffer(uint16_t *pwm_data, Color colors[])
=======
<<<<<<< HEAD
=======
/*
>>>>>>> 53-création-de-la-gui
// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void rgb_update_buffer(uint16_t *pwm_data, ColorName *colors) {
    for(int led = 0; led < LED_NUMBER; led++)
    {
<<<<<<< HEAD
        uint32_t color = ((uint32_t)colors_values[colors[led]].r << 16) |  // R
        				 ((uint32_t)colors_values[colors[led]].g << 8) |  // G
=======
        uint32_t color = ((uint32_t)colors_values[colors[led]].g << 16) |  // R
        				 ((uint32_t)colors_values[colors[led]].r << 8) |  // G
>>>>>>> 53-création-de-la-gui
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
<<<<<<< HEAD

/* OLD
=======
*/

>>>>>>> 53-création-de-la-gui
// -------------------------------------------------------------------
// Fill the buffer PWM depending on the table "colors"
// -------------------------------------------------------------------
void rgb_update_buffer(uint16_t *pwm_data, uint8_t colors[][3])
>>>>>>> main
{
    for(int led = 0; led < LED_NUMBER; led++)
    {
        uint32_t color = ((uint32_t)colors[led].g << 16) |  // R
        				 ((uint32_t)colors[led].r << 8) |  // G
                         ((uint32_t)colors[led].b << 0);  // R
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
<<<<<<< HEAD

/*
 * Clear all the led state
 */
void leds_clear(Color colors[]) {
	for(uint8_t index = 0; index < LED_NUMBER; ++index) {
		colors[index].r = colors[index].g = colors[index].b = 0 ;
	}
}

/*
 * Set the led color and brightness
 */
void led_set(uint8_t index, Color new_color, Color colors[], uint8_t brightness) {
	colors[index].r = (uint8_t)(new_color.r * brightness / 255);
	colors[index].g = (uint8_t)(new_color.g * brightness / 255);
	colors[index].b = (uint8_t)(new_color.b * brightness / 255);
}

=======
<<<<<<< HEAD
*/
=======
>>>>>>> 53-création-de-la-gui
>>>>>>> main
