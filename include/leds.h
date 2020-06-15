#ifndef LEDS_H
#define LEDS_H

typedef enum LED_COLOR { RED = 0, BLUE = 1, GREEN = 2, GREEN2 = 3 } LED_COLOR;

typedef enum LED_STATE { ON = 0, OFF = 1, TOGGLE = 2 } LED_STATE;

// Sets GPIO pins as output for connected leds.
void led_conf();

// Turns led off.
void led_off(LED_COLOR color);

// Turns led on.
void led_on(LED_COLOR color);

// Toggles led.
void led_toggle(LED_COLOR color);

#endif
