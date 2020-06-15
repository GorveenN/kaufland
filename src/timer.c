#include <timer.h>

#include <buttons.h>
#include <gpio.h>
#include <irq.h>
#include <logger.h>
#include <stm32.h>

static void (*callback)(BUTTON_ID);

// We need to check those buttons after interrupt arrives
static bool activated_buttons[7] = {false, false, false, false,
                                    false, false, false};

void timer_delay(BUTTON_ID id) {
    activated_buttons[id] = true;
    // Zero interrupt
    TIM3->CNT = 0;
    // Enable timer
    TIM3->CR1 |= TIM_CR1_CEN;
}

void timer_conf(void (*f)(BUTTON_ID)) {
    callback = f;
    // Enable timer clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    __NOP();

    // Counting up
    TIM3->CR1 = TIM_CR1_UDIS;
    TIM3->PSC = 15999;
    // Maximum value of timer
    TIM3->ARR = 50;
    // Zero timer
    TIM3->CNT = 0;

    TIM3->EGR = TIM_EGR_UG;

    TIM3->CR1 = 0;
    // Enable timer interrupts
    TIM3->SR = ~TIM_SR_UIF;
    TIM3->DIER = TIM_DIER_UIE;

    NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void) {
    uint32_t it_status = TIM3->SR & TIM3->DIER;
    if (it_status & TIM_SR_UIF) {
        // Disable timer
        TIM3->SR = ~TIM_SR_UIF;
        TIM3->CR1 &= ~TIM_CR1_CEN;

        for (int i = 0; i < 7; ++i) {
            if (activated_buttons[i]) {
                activated_buttons[i] = false;
                callback(i);
            }
        }
    }
}
