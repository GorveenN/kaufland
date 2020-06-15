#include <bluetooth.h>

#include <string.h>

#include <gpio.h>
#include <irq.h>
#include <stm32.h>
#include <str_queue.h>

#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable USART_CR1_UE
#define USART_WordLength_8b 0x0000
#define USART_Parity_No 0x0000
#define USART_StopBits_1 0x0000
#define USART_FlowControl_None 0x0000
#define HSI_HZ 16000000U
#define PCLK2_HZ HSI_HZ

static str_queue queue = {NULL, NULL};
void (*read_cb)(char);

static void init_send(const char *buff, int len) {
    DMA2_Stream7->M0AR = (uint32_t)buff;
    DMA2_Stream7->NDTR = len;
    DMA2_Stream7->CR |= DMA_SxCR_EN;
}

static void init_read() {
    static char *read_buff = "0";
    DMA2_Stream5->M0AR = (uint32_t)read_buff;
    DMA2_Stream5->NDTR = 1;
    DMA2_Stream5->CR |= DMA_SxCR_EN;
}

// Puts string buff to queue to send via DMA over bluetooth
void bt_print(const char *buff) {
    if ((DMA2_Stream7->CR & DMA_SxCR_EN) == 0 &&
        (DMA2->HISR & DMA_HISR_TCIF7) == 0) {
        init_send(buff, strlen(buff));
    } else {
        queue_put(&queue, buff);
    }
}

void DMA2_Stream7_IRQHandler() {
    /* Read interrupts from DMA2 */
    uint32_t isr = DMA2->HISR;
    if (isr & DMA_HISR_TCIF7) {
        /* Transfer ended on stream 6 */
        DMA2->HIFCR = DMA_HIFCR_CTCIF7;
        /* If there is something to send, inistialize transmission */
        if (!queue_is_empty(&queue)) {
            const char *str = queue_get(&queue);
            init_send(str, strlen(str));
        }
    }
}

void DMA2_Stream5_IRQHandler() {
    /* Read interrupts from DMA2 */
    uint32_t isr = DMA2->HISR;
    if (isr & DMA_HISR_TCIF5) {
        /* Transfer ended on stream 6 */
        DMA2->HIFCR = DMA_HIFCR_CTCIF5;
        /* Activate receiving again */
        init_read();
        read_cb(USART1->DR);
    }
}

void bt_conf(void (*f)(char)) {
    read_cb = f;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA2EN;
    RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
    __NOP();

    USART1->CR1 = USART_Mode_Rx_Tx | USART_WordLength_8b | USART_Parity_No |
                  USART_CR1_RE | USART_CR1_TE | USART_Enable;
    USART1->CR2 = USART_StopBits_1;
    USART1->CR3 = USART_FlowControl_None | USART_CR3_DMAT | USART_CR3_DMAR;
    uint32_t const baudrate = 9600U;
    USART1->BRR = (PCLK2_HZ + (baudrate / 2U)) / baudrate;

    GPIOafConfigure(GPIOA, 9, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_NOPULL,
                    GPIO_AF_USART1);

    GPIOafConfigure(GPIOA, 10, GPIO_OType_PP, GPIO_Fast_Speed, GPIO_PuPd_UP,
                    GPIO_AF_USART1);

    /* Read stream */
    DMA2_Stream7->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC |
                       DMA_SxCR_DIR_0 | DMA_SxCR_TCIE;
    DMA2_Stream7->PAR = (uint32_t)&USART1->DR;

    /* Send stream */
    DMA2_Stream5->CR = 4U << 25 | DMA_SxCR_PL_1 | DMA_SxCR_MINC | DMA_SxCR_TCIE;
    DMA2_Stream5->PAR = (uint32_t)&USART1->DR;

    /* Clear interrupts flags and enable interrupts */
    DMA2->HIFCR = DMA_HIFCR_CTCIF7 | DMA_HIFCR_CTCIF5;
    NVIC_EnableIRQ(DMA2_Stream7_IRQn);
    NVIC_EnableIRQ(DMA2_Stream5_IRQn);

    /* Enable usart  */
    USART1->CR1 |= USART_CR1_UE;
    init_read();
}
