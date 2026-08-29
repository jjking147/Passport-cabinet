/*!
    \file    ota_log.c
    \brief   日志串口实现 (USART0, PA9 TX / PA10 RX)
*/

#include "ota_log.h"
#include "gd32f4xx.h"
#include "delay.h"

#define LOG_USART      USART0
#define LOG_GPIO       GPIOA
#define LOG_GPIO_CLK   RCU_GPIOA
#define LOG_USART_CLK  RCU_USART0
#define LOG_TX_PIN     GPIO_PIN_9
#define LOG_RX_PIN     GPIO_PIN_10
#define LOG_BAUD       9600U

void ota_log_init(void)
{
    /* 使能时钟 */
    rcu_periph_clock_enable(LOG_GPIO_CLK);
    rcu_periph_clock_enable(LOG_USART_CLK);

    /* PA9(TX) / PA10(RX) 复用为 USART0 (AF7) */
    gpio_mode_set(LOG_GPIO, GPIO_MODE_AF, GPIO_PUPD_PULLUP, LOG_TX_PIN | LOG_RX_PIN);
    gpio_output_options_set(LOG_GPIO, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, LOG_TX_PIN | LOG_RX_PIN);
    gpio_af_set(LOG_GPIO, GPIO_AF_7, LOG_TX_PIN | LOG_RX_PIN);

    /* USART0: 115200-8-N-1 */
    usart_deinit(LOG_USART);
    usart_baudrate_set(LOG_USART, LOG_BAUD);
    usart_word_length_set(LOG_USART, USART_WL_8BIT);
    usart_stop_bit_set(LOG_USART, USART_STB_1BIT);
    usart_parity_config(LOG_USART, USART_PM_NONE);
    usart_hardware_flow_rts_config(LOG_USART, USART_RTS_DISABLE);
    usart_hardware_flow_cts_config(LOG_USART, USART_CTS_DISABLE);
    usart_receive_config(LOG_USART, USART_RECEIVE_ENABLE);
    usart_transmit_config(LOG_USART, USART_TRANSMIT_ENABLE);
    usart_enable(LOG_USART);
}

void ota_log_send_byte(uint8_t c)
{
    while(usart_flag_get(LOG_USART, USART_FLAG_TBE) == RESET) {
    }
    usart_data_transmit(LOG_USART, c);
}

int ota_log_recv_byte(uint32_t timeout_ms)
{
    uint32_t ticks = 0;
    uint32_t limit = timeout_ms * 100u;      /* 每 tick = 10us, limit = ms*100 */
    while(usart_flag_get(LOG_USART, USART_FLAG_RBNE) == RESET) {
        delay_us(10);                         /* 10us 轮询粒度 */
        ticks++;
        if(ticks >= limit) {
            return -1;
        }
    }
    return (int)usart_data_receive(LOG_USART);
}

void ota_log_print(const char *s)
{
    while(*s) {
        ota_log_send_byte((uint8_t)*s++);
    }
}

void ota_log_print_hex(uint32_t v)
{
    static const char hex[] = "0123456789ABCDEF";
    ota_log_send_byte('0');
    ota_log_send_byte('x');
    for(int i = 28; i >= 0; i -= 4) {
        ota_log_send_byte(hex[(v >> i) & 0xFu]);
    }
}
