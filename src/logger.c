#include <logger.h>

#include <stdbool.h>
#include <string.h>

#include <gpio.h>
#include <irq.h>
#include <leds.h>
#include <stm32.h>
#include <str_queue.h>

#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE

#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M

#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)

#define USART_StopBits_1 0x0000
#define USART_StopBits_0_5 0x1000
#define USART_StopBits_2 0x2000
#define USART_StopBits_1_5 0x3000

#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE

#define HSI_HZ 16000000U
#define PCLK1_HZ HSI_HZ

#define BUFFER_SIZE 4096

static str_queue queue = {NULL, NULL};
static char *receive_buff = "abc";

void (*read_cb)(char);

static void init_read() {
    DMA1_Stream5->M0AR = (uint32_t)receive_buff;
    DMA1_Stream5->NDTR = 1;
    DMA1_Stream5->CR |= DMA_SxCR_EN;
}

static void init_send(const char *buff, int len) {
    DMA1_Stream6->M0AR = (uint32_t)buff;
    DMA1_Stream6->NDTR = len;
    DMA1_Stream6->CR |= DMA_SxCR_EN;
}

void uart_print(const char *buff) {
    if ((DMA1_Stream6->CR & DMA_SxCR_EN) == 0 &&
        (DMA1->HISR & DMA_HISR_TCIF6) == 0) {
        init_send(buff, strlen(buff));
    } else {
        irq_level_t level = IRQprotectAll();
        queue_put(&queue, buff);
        IRQunprotectAll(level);
    }
}

void DMA1_Stream6_IRQHandler() {
    /* Odczytaj zgłoszone przerwania DMA1. */
    uint32_t isr = DMA1->HISR;
    if (isr & DMA_HISR_TCIF6) {
        /* Obsłuż zakończenie transferu w strumieniu 6. */
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;
        /* Jeśli jest coś do wysłania, wystartuj kolejną transmisję. */
        if (!queue_is_empty(&queue)) {
            const char *str = queue_get(&queue);
            init_send(str, strlen(str));
        }
    }
}

void DMA1_Stream5_IRQHandler() {
    /* Odczytaj zgłoszone przerwania DMA1. */
    uint32_t isr = DMA1->HISR;
    if (isr & DMA_HISR_TCIF5) {
        /* Obsłuż zakończenie transferu
        w strumieniu 5. */
        DMA1->HIFCR = DMA_HIFCR_CTCIF5;
        /* Ponownie uaktywnij odbieranie. */
        init_read();
        read_cb(USART2->DR);
    }
}

void uart_conf(void (*f)(char)) {
    read_cb = f;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    __NOP();

    USART2->CR1 = USART_Mode_Rx_Tx | USART_WordLength_8b | USART_Parity_No |
                  USART_CR1_RE | USART_CR1_TE | USART_Enable;
    USART2->CR2 = USART_StopBits_1;
    /* USART2->CR3 = USART_FlowControl_None | USART_CR3_DMAT | USART_CR3_DMAR;
     */
    USART2->CR3 = USART_CR3_DMAT | USART_CR3_DMAR;

    uint32_t const baudrate = 9600U;
    USART2->BRR = (PCLK1_HZ + (baudrate / 2U)) / baudrate;

    GPIOafConfigure(GPIOA, 2, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_NOPULL,
                    GPIO_AF_USART2);

    GPIOafConfigure(GPIOA, 3, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_UP,
                    GPIO_AF_USART2);

    /* układ odbiorczy */
    DMA1_Stream6->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC |
                       DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;
    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;

    /* układ nadawczy */
    DMA1_Stream5->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_TCIE;
    DMA1_Stream5->PAR = (uint32_t)&USART2->DR;

    /* Wyczyść znaczniki przerwań i włącz przerwania */
    DMA1->HIFCR = DMA_HIFCR_CTCIF6 | DMA_HIFCR_CTCIF5;
    NVIC_EnableIRQ(DMA1_Stream5_IRQn);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    /* Uaktywnij układ peryferyjny */
    USART2->CR1 |= USART_CR1_UE;
    init_read();
}
