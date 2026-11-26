#include "bsp_board.h"
#include "bsp_led.h"
#include "bsp_tick.h"
#include "main.h"
#include "boot_config.h"
#include "stm32f4xx.h"

void BSP_KeyInit(void)
{
    GPIO_InitTypeDef io;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    GPIO_StructInit(&io);
    io.GPIO_Pin = GPIO_Pin_3;
    io.GPIO_Mode = GPIO_Mode_IN;
    io.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &io);
}

bool BSP_UpdateKeyPressed(void)
{
    return GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_3) == Bit_RESET;
}

void BSP_StatusTimerInit(void)
{
    RCC_ClocksTypeDef clocks;
    TIM_TimeBaseInitTypeDef timer;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_GetClocksFreq(&clocks);
    /* On F407 the timer runs at 2 x PCLK1 when APB1 is prescaled. */
    uint32_t hz = clocks.PCLK1_Frequency;
    if (clocks.PCLK1_Frequency != clocks.HCLK_Frequency) {
        hz *= 2U;
    }
    TIM_TimeBaseStructInit(&timer);
    /* 10 kHz counter. Both PSC and ARR count from zero. */
    timer.TIM_Prescaler = (uint16_t)(hz / 10000U - 1U);
    timer.TIM_Period = BOOT_HEARTBEAT_MS * 10U - 1U;
    TIM_TimeBaseInit(TIM3, &timer);
    TIM_ClearFlag(TIM3, TIM_FLAG_Update);
    TIM_Cmd(TIM3, ENABLE);
}

void BSP_StatusPoll(void)
{
    if (TIM_GetFlagStatus(TIM3, TIM_FLAG_Update) != RESET) {
        TIM_ClearFlag(TIM3, TIM_FLAG_Update);
        BSP_LED_ToggleAll();
    }
}

/* USART1 DMA requests use DMA2 channel 4 on STM32F407. */
#define UART_RX_DMA DMA2_Stream2
#define UART_TX_DMA DMA2_Stream7
#define UART_RX_DMA_ERRORS (DMA_LISR_TEIF2 | DMA_LISR_DMEIF2)
#define UART_TX_DMA_ERRORS (DMA_HISR_TEIF7 | DMA_HISR_DMEIF7)
#define UART_RX_DMA_FLAGS (DMA_FLAG_TCIF2 | DMA_FLAG_HTIF2 | DMA_FLAG_TEIF2 | \
                           DMA_FLAG_DMEIF2 | DMA_FLAG_FEIF2)
#define UART_TX_DMA_FLAGS (DMA_FLAG_TCIF7 | DMA_FLAG_HTIF7 | DMA_FLAG_TEIF7 | \
                           DMA_FLAG_DMEIF7 | DMA_FLAG_FEIF7)
#define UART_RX_ERRORS (USART_SR_ORE | USART_SR_FE | USART_SR_NE | USART_SR_PE)

enum { UART_RX_CAPACITY = 2048U, UART_TX_TIMEOUT_MS = 20U };
static volatile uint8_t uart_rx_ring[UART_RX_CAPACITY];
static uint8_t uart_tx_byte;
static uint32_t uart_rx_wrap_base;
static uint32_t uart_rx_consumed;
static bool uart_dma_initialized;

static void uart_dma_stop(DMA_Stream_TypeDef *stream)
{
    DMA_Cmd(stream, DISABLE);
    while (DMA_GetCmdStatus(stream) != DISABLE) {}
}

static void uart_dma_start_rx(void)
{
    USART_DMACmd(USART1, USART_DMAReq_Rx, DISABLE);
    uart_dma_stop(UART_RX_DMA);
    /* Reading the USART status then data register clears line errors. */
    (void)USART1->SR;
    (void)USART1->DR;

    uart_rx_wrap_base = 0U;
    uart_rx_consumed = 0U;
    DMA_ClearFlag(UART_RX_DMA, UART_RX_DMA_FLAGS);
    DMA_SetCurrDataCounter(UART_RX_DMA, UART_RX_CAPACITY);
    DMA_Cmd(UART_RX_DMA, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
}

static void uart_dma_init(void)
{
    DMA_InitTypeDef dma;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
    (void)RCC->AHB1ENR;

    USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);
    uart_dma_stop(UART_RX_DMA);
    uart_dma_stop(UART_TX_DMA);
    DMA_DeInit(UART_RX_DMA);
    DMA_DeInit(UART_TX_DMA);

    DMA_StructInit(&dma);
    dma.DMA_Channel = DMA_Channel_4;
    dma.DMA_PeripheralBaseAddr = (uint32_t)(uintptr_t)&USART1->DR;
    dma.DMA_Memory0BaseAddr = (uint32_t)(uintptr_t)uart_rx_ring;
    dma.DMA_DIR = DMA_DIR_PeripheralToMemory;
    dma.DMA_BufferSize = UART_RX_CAPACITY;
    dma.DMA_MemoryInc = DMA_MemoryInc_Enable;
    dma.DMA_Mode = DMA_Mode_Circular;
    dma.DMA_Priority = DMA_Priority_High;
    DMA_Init(UART_RX_DMA, &dma);

    dma.DMA_Memory0BaseAddr = (uint32_t)(uintptr_t)&uart_tx_byte;
    dma.DMA_DIR = DMA_DIR_MemoryToPeripheral;
    dma.DMA_BufferSize = 1U;
    dma.DMA_MemoryInc = DMA_MemoryInc_Disable;
    dma.DMA_Mode = DMA_Mode_Normal;
    dma.DMA_Priority = DMA_Priority_Low;
    DMA_Init(UART_TX_DMA, &dma);

    uart_dma_start_rx();
    uart_dma_initialized = true;
}

/* NDTR is the DMA stream's remaining transfer counter. Sample it with
 * the DMA transfer-complete flag so a ring wrap cannot look like no data.
 * The sticky flag is serviced by the polling reader. Ymodem sends at
 * most one frame per ACK, and one full frame fits in this ring. */
static uint32_t uart_rx_produced(void)
{
    uint16_t remaining;
    for (;;) {
        remaining = DMA_GetCurrDataCounter(UART_RX_DMA);
        if ((DMA2->LISR & DMA_LISR_TCIF2) == 0U) {
            if (remaining != 0U) {
                return uart_rx_wrap_base + UART_RX_CAPACITY - remaining;
            }
        } else {
            DMA_ClearFlag(UART_RX_DMA, DMA_FLAG_TCIF2);
            uart_rx_wrap_base += UART_RX_CAPACITY;
        }
    }
}

void BSP_UartInit(void)
{
    GPIO_InitTypeDef io;
    USART_InitTypeDef uart;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    (void)RCC->AHB1ENR;
    (void)RCC->APB2ENR;

    /* AF7 routes PA9 to USART1_TX and PA10 to USART1_RX. */
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);
    GPIO_StructInit(&io);
    io.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
    io.GPIO_Mode = GPIO_Mode_AF;
    io.GPIO_PuPd = GPIO_PuPd_UP;
    io.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &io);
    USART_StructInit(&uart);
    uart.USART_BaudRate = 115200U;
    uart.USART_WordLength = USART_WordLength_8b;
    uart.USART_StopBits = USART_StopBits_1;
    uart.USART_Parity = USART_Parity_No;
    uart.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    uart.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART1, &uart);
    USART_Cmd(USART1, ENABLE);
    uart_dma_init();
}

bool BSP_UartRead(uint8_t *byte, uint32_t timeout_ms)
{
    const uint32_t start = BSP_GetTick();
    do {
        if ((USART1->SR & UART_RX_ERRORS) != 0U ||
            (DMA2->LISR & UART_RX_DMA_ERRORS) != 0U) {
            uart_dma_start_rx();
            return false;
        }

        const uint32_t produced = uart_rx_produced();
        const uint32_t pending = produced - uart_rx_consumed;
        if (pending >= UART_RX_CAPACITY) {
            uart_rx_consumed = produced;
            return false;
        }
        if (pending != 0U) {
            __DMB();
            *byte = uart_rx_ring[uart_rx_consumed % UART_RX_CAPACITY];
            ++uart_rx_consumed;
            return true;
        }
        BSP_StatusPoll();
    } while (BSP_GetTick() - start < timeout_ms);
    return false;
}

void BSP_UartWrite(uint8_t byte)
{
    const uint32_t start = BSP_GetTick();
    uart_tx_byte = byte;
    DMA_ClearFlag(UART_TX_DMA, UART_TX_DMA_FLAGS);
    DMA_SetCurrDataCounter(UART_TX_DMA, 1U);
    USART_ClearFlag(USART1, USART_FLAG_TC);
    __DMB();
    DMA_Cmd(UART_TX_DMA, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);

    /* DMA TC means the byte reached the USART data register. */
    uint32_t status;
    do {
        status = DMA2->HISR;
        if ((status & UART_TX_DMA_ERRORS) != 0U ||
            BSP_GetTick() - start >= UART_TX_TIMEOUT_MS) {
            USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
            uart_dma_stop(UART_TX_DMA);
            Error_Handler(BOOT_ERROR_UART_DMA);
        }
    } while ((status & DMA_HISR_TCIF7) == 0U);

    USART_DMACmd(USART1, USART_DMAReq_Tx, DISABLE);
    uart_dma_stop(UART_TX_DMA);
}

void BSP_UartDrain(void)
{
    const uint32_t start = BSP_GetTick();
    /* USART TC means the final stop bit has left PA9. */
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) {
        if (BSP_GetTick() - start >= UART_TX_TIMEOUT_MS) {
            Error_Handler(BOOT_ERROR_UART_DMA);
        }
    }
}

void BSP_BoardDeinit(void)
{
    if (uart_dma_initialized) {
        USART_DMACmd(USART1, USART_DMAReq_Rx | USART_DMAReq_Tx, DISABLE);
        uart_dma_stop(UART_RX_DMA);
        uart_dma_stop(UART_TX_DMA);
        DMA_DeInit(UART_RX_DMA);
        DMA_DeInit(UART_TX_DMA);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, DISABLE);
        uart_dma_initialized = false;
    }
    USART_DeInit(USART1);
    TIM_DeInit(TIM3);
    uint32_t ports = RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF;
    RCC_AHB1PeriphResetCmd(ports, ENABLE);
    RCC_AHB1PeriphResetCmd(ports, DISABLE);
    RCC_AHB1PeriphClockCmd(ports, DISABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
}
