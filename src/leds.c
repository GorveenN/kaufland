#include <leds.h>

#include <string.h>

#include <gpio.h>
#include <stm32.h>

#define RED_LED_GPIO GPIOA
#define GREEN_LED_GPIO GPIOA
#define BLUE_LED_GPIO GPIOB
#define GREEN2_LED_GPIO GPIOA

#define RED_LED_PIN 6
#define GREEN_LED_PIN 7
#define BLUE_LED_PIN 0
#define GREEN2_LED_PIN 5

#define LED_NUM 4

typedef struct LED_DESCRIPTOR {
    GPIO_TypeDef *gpio;
    uint32_t pin;
    LED_STATE state;
} LED_DESCRIPTOR;

LED_DESCRIPTOR leds_desc[] = {{RED_LED_GPIO, RED_LED_PIN, ON},
                              {BLUE_LED_GPIO, BLUE_LED_PIN, ON},
                              {GREEN_LED_GPIO, GREEN_LED_PIN, ON},
                              {GREEN2_LED_GPIO, GREEN2_LED_PIN, ON}};

void led_conf() {
    for (int i = 0; i < LED_NUM; ++i) {
        led_off(i);
        GPIOoutConfigure(leds_desc[i].gpio, leds_desc[i].pin, GPIO_OType_PP,
                         GPIO_Low_Speed, GPIO_PuPd_NOPULL);
    }
}

void led_off(LED_COLOR color) {
    LED_DESCRIPTOR *led_desc = &(leds_desc[color]);
    GPIO_TypeDef *led_gpio = led_desc->gpio;
    uint32_t led_pin = led_desc->pin;
    led_desc->state = OFF;

    if (led_pin == RED_LED_PIN || led_pin == GREEN_LED_PIN ||
        led_pin == BLUE_LED_PIN) {
        led_gpio->BSRR = 1 << led_desc->pin;
    } else {
        led_gpio->BSRR = 1 << (led_desc->pin + 16);
    }
}

void led_on(LED_COLOR color) {
    LED_DESCRIPTOR *led_desc = &leds_desc[color];
    GPIO_TypeDef *led_gpio = led_desc->gpio;
    uint32_t led_pin = led_desc->pin;
    led_desc->state = ON;

    if (led_pin == RED_LED_PIN || led_pin == BLUE_LED_PIN ||
        led_pin == GREEN_LED_PIN) {
        led_gpio->BSRR = 1 << (led_desc->pin + 16);
    } else {
        led_gpio->BSRR = 1 << led_desc->pin;
    }
}

void led_toggle(LED_COLOR color) {
    LED_STATE current_state = leds_desc[color].state;
    LED_STATE target_state = current_state == ON ? OFF : ON;

    // Preventing from turning off already turned off led.
    if (target_state == ON && current_state == OFF) {
        led_on(color);
    } else if (target_state == OFF && current_state == ON) {
        led_off(color);
    }
}
