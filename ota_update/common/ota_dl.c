/*!
    \file    ota_dl.c
    \brief   下载串口实现 (USART1, PA2 TX / PA3 RX)
*/

#include "ota_dl.h"
#include "gd32f4xx.h"
#include "delay.h"

#define DL_USART      USART1
#define DL_GPIO       GPIOA
#define DL_GPIO_CLK   RCU_GPIOA
#define DL_USART_CLK  RCU_USART1
#define DL_TX_PIN     GPIO_PIN_2
#define DL_RX_PIN     GPIO_PIN_3
#define DL_BAUD       9600U

void ota_dl_init(void)
{
    /* 使能时钟 */
    rcu_periph_clock_enable(DL_GPIO_CLK);
    rcu_periph_clock_enable(DL_USART_CLK);

    /* PA2(TX) / PA3(RX) 复用为 USART1 (AF7) */
    gpio_mode_set(DL_GPIO, GPIO_MODE_AF, GPIO_PUPD_PULLUP, DL_TX_PIN | DL_RX_PIN);
    gpio_output_options_set(DL_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, DL_TX_PIN | DL_RX_PIN);
    gpio_af_set(DL_GPIO, GPIO_AF_7, DL_TX_PIN | DL_RX_PIN);

    /* USART1: 115200-8-N-1 */
    usart_deinit(DL_USART);
    usart_baudrate_set(DL_USART, DL_BAUD);
    usart_word_length_set(DL_USART, USART_WL_8BIT);
    usart_stop_bit_set(DL_USART, USART_STB_1BIT);
    usart_parity_config(DL_USART, USART_PM_NONE);
    usart_hardware_flow_rts_config(DL_USART, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(DL_USART, USART_CTS_DISABLE);
    usart_receive_config(DL_USART, USART_RECEIVE_ENABLE);
    usart_transmit_config(DL_USART, USART_TRANSMIT_ENABLE);
    usart_enable(DL_USART);
}

void ota_dl_send_byte(uint8_t c)
{
    while(usart_flag_get(DL_USART, USART_FLAG_TBE) == RESET) {
    }
    usart_data_transmit(DL_USART, c);
}

int ota_dl_recv_byte(uint32_t timeout_ms)
{
    uint32_t ticks = 0;
    uint32_t limit = timeout_ms * 100u;      /* 每 tick = 10us, limit = ms*100 */
    while(usart_flag_get(DL_USART, USART_FLAG_RBNE) == RESET) {
        delay_us(10);                         /* 10us 轮询粒度, 115200 下不丢字节 */
        ticks++;
        if(ticks >= limit) {
            return -1;
        }
    }
    return (int)usart_data_receive(DL_USART);
}
