#ifndef RGB_LED_H
#define RGB_LED_H

#include <stdint.h>

#define LED_NUMBER      	64
#define RESET_SLOTS      	50
#define LED_BUFFER_SIZE  	(24*LED_NUMBER + RESET_SLOTS)

// Avec ARR = 39 (32 MHz -> 800 kHz)
#define HIGH_DUTY  26   // ≈ 0.8 µs (logique "1")
#define LOW_DUTY   13   // ≈ 0.4 µs (logique "0")

static volatile uint8_t ws2812_transfer_complete = 0;

typedef struct {
	uint8_t r, g, b;
} Color;


void led_update_buffer(uint16_t *pwm_data, Color colors[]);
void led_set(uint8_t index, Color new_color, Color colors[], uint8_t brightness);
void leds_clear(Color colors[]);

#endif
