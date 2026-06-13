/**
  ******************************************************************************
  * @file    BSP_USART.c
  * @brief   串口通信与 VOFA+ 上位机测试接口实现
  *          使用中断发送 + 环形缓冲区，线程安全
  ******************************************************************************
  */

#include "BSP_USART.h"

/* ── 环形发送缓冲区 ─────────────────────────────── */
#define UART_TX_BUF_SIZE  256
#define UART_TX_BUF_MASK  (UART_TX_BUF_SIZE - 1)

static uint8_t  tx_buf[UART_TX_BUF_SIZE];
static volatile uint16_t tx_wr = 0;    /* fputc 写入位置 */
static volatile uint16_t tx_rd = 0;    /* ISR  读取位置 */

/* ── 内部：启动中断发送 ──────────────────────────── */
static void UART_TX_Start(void)
{
    if (tx_rd != tx_wr) {
        USART3->DR = tx_buf[tx_rd];
        tx_rd = (tx_rd + 1) & UART_TX_BUF_MASK;
        SET_BIT(USART3->CR1, USART_CR1_TXEIE);   /* 使能 TXE 中断 */
    }
}

/* ── 中断服务（供 USART3_IRQHandler 调用）────────── */
void BSP_USART3_IRQHandler(void)
{
    /* TXE: 数据寄存器空，可以发下一字节 */
    if ((USART3->SR & USART_SR_TXE) && (USART3->CR1 & USART_CR1_TXEIE)) {
        if (tx_rd != tx_wr) {
            USART3->DR = tx_buf[tx_rd];
            tx_rd = (tx_rd + 1) & UART_TX_BUF_MASK;
        } else {
            /* 缓冲区空 → 关闭 TXE 中断，停止发送 */
            CLEAR_BIT(USART3->CR1, USART_CR1_TXEIE);
        }
    }
}

/* ── printf 重定向（中断发送 + 按行互斥）─────────── */
int fputc(int ch, FILE *f)
{
    static uint8_t tx_locked = 0;

    /* 获取行级互斥锁 */
    if (!tx_locked) {
        if (xSemaphoreTake(USART_Mutex, pdMS_TO_TICKS(200)) != pdTRUE) {
            return EOF;
        }
        tx_locked = 1;
    }

    /* 写入环形缓冲区（满则短暂让出 CPU 等 ISR 消费） */
    uint16_t next = (tx_wr + 1) & UART_TX_BUF_MASK;
    while (next == tx_rd) {
        taskYIELD();
    }
    tx_buf[tx_wr] = (uint8_t)ch;
    tx_wr = next;

    /* 若 UART 空闲，立即启动发送 */
    if (!(USART3->CR1 & USART_CR1_TXEIE)) {
        UART_TX_Start();
    }

    /* 行结束 → 释放互斥锁 */
    if (ch == '\n') {
        xSemaphoreGive(USART_Mutex);
        tx_locked = 0;
    }

    return ch;
}
