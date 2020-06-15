#include <buttons.h>

#include <gpio.h>
#include <irq.h>
#include <logger.h>
#include <stm32.h>

// active state 0
#define JOYSTICK_GPIO GPIOB
#define LEFT_JOYSTICK_PIN 3
#define RIGHT_JOYSTICK_PIN 4
#define UP_JOYSTICK_PIN 5
#define DOWN_JOYSTICK_PIN 6
#define ACTION_JOYSTICK_PIN 10

// active state 0
#define USER_BUTTON_GPIO GPIOC
#define USER_BUTTON_PIN 13

// active state 1
#define ATMODE_BUTTON_GPIO GPIOA
#define ATMODE_BUTTON_PIN 0

#define IRQ_PRIORITY 2U

typedef struct {
    GPIO_TypeDef *gpio;
    uint32_t pin;
} BUTTON_DESCRIPTOR;

static BUTTON_DESCRIPTOR buttons_desc[] = {
    {JOYSTICK_GPIO, LEFT_JOYSTICK_PIN},     {JOYSTICK_GPIO, RIGHT_JOYSTICK_PIN},
    {JOYSTICK_GPIO, UP_JOYSTICK_PIN},       {JOYSTICK_GPIO, DOWN_JOYSTICK_PIN},
    {JOYSTICK_GPIO, ACTION_JOYSTICK_PIN},   {USER_BUTTON_GPIO, USER_BUTTON_PIN},
    {ATMODE_BUTTON_GPIO, ATMODE_BUTTON_PIN}};

void (*read_cd)(BUTTON_ID, bool);

static inline bool is_button_pressed_high(GPIO_TypeDef *gpio, uint32_t pin) {
    return gpio->IDR & (1 << pin);
    JOYSTICK_GPIO->BSRR;
}

static inline bool is_button_pressed_low(GPIO_TypeDef *gpio, uint32_t pin) {
    return (~gpio->IDR) & (1 << pin);
}

bool buttons_is_pressed(BUTTON_ID id) {
    GPIO_TypeDef *button_gpio = buttons_desc[id].gpio;
    uint32_t button_pin = buttons_desc[id].pin;

    return button_gpio == ATMODE_BUTTON_GPIO
               ? is_button_pressed_high(button_gpio, button_pin)
               : is_button_pressed_low(button_gpio, button_pin);
}

void buttons_conf(void (*f)(BUTTON_ID, bool)) {
    read_cd = f;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    RCC->AHB1ENR |=
        RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    __NOP();

    GPIOinConfigure(USER_BUTTON_GPIO, USER_BUTTON_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(ATMODE_BUTTON_GPIO, ATMODE_BUTTON_PIN, GPIO_PuPd_UP,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JOYSTICK_GPIO, UP_JOYSTICK_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JOYSTICK_GPIO, DOWN_JOYSTICK_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JOYSTICK_GPIO, LEFT_JOYSTICK_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JOYSTICK_GPIO, RIGHT_JOYSTICK_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);
    GPIOinConfigure(JOYSTICK_GPIO, ACTION_JOYSTICK_PIN, GPIO_PuPd_DOWN,
                    EXTI_Mode_Interrupt, EXTI_Trigger_Rising_Falling);

    // Clear insterrupt flags
    EXTI->PR = EXTI->PR;

    // Enable interrupts
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

// atmode
void EXTI0_IRQHandler(void) {
    EXTI->PR = EXTI_PR_PR0;
    read_cd(ATMODE, buttons_is_pressed(ATMODE));
}

// left
void EXTI3_IRQHandler(void) {
    EXTI->PR = EXTI_PR_PR3;
    read_cd(LEFT_JOYSTICK, buttons_is_pressed(LEFT_JOYSTICK));
}

// right
void EXTI4_IRQHandler(void) {
    EXTI->PR = EXTI_PR_PR4;
    read_cd(RIGHT_JOYSTICK, buttons_is_pressed(RIGHT_JOYSTICK));
}

// up 5 down 6
void EXTI9_5_IRQHandler(void) {
    uint32_t exti = EXTI->PR;
    if (exti & EXTI_PR_PR5) {
        EXTI->PR = EXTI_PR_PR5;
        read_cd(UP_JOYSTICK, buttons_is_pressed(UP_JOYSTICK));
    }
    if (exti & EXTI_PR_PR6) {
        EXTI->PR = EXTI_PR_PR6;
        read_cd(DOWN_JOYSTICK, buttons_is_pressed(DOWN_JOYSTICK));
        EXTI->PR;
    }
}

// user 13, joystick action 10
void EXTI15_10_IRQHandler(void) {
    uint32_t exti = EXTI->PR;
    if (exti & EXTI_PR_PR13) {
        EXTI->PR = EXTI_PR_PR13;
        read_cd(USER, buttons_is_pressed(USER));
    }
    if (exti & EXTI_PR_PR10) {
        EXTI->PR = EXTI_PR_PR10;
        read_cd(ACTION_JOYSTICK, buttons_is_pressed(ACTION_JOYSTICK));
    }
}
