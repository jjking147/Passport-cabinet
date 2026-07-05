#include "sys.h"
#include "string.h"
#include "usart.h"
#include "modbus_master.h"
#include "modbus_slave.h"

#if 1
#pragma import(__use_no_semihosting)
struct __FILE
{
	int handle;
};

FILE __stdout;
void _sys_exit(int x)
{
	x = x;
}
int fputc(int ch, FILE *f)
{
	while(usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
	usart_data_transmit(USART0, (u8)ch);
	return ch;
}
#endif

void Master_USART_Init(u32 bound)
{
	/* Enable clocks */
	rcu_periph_clock_enable(RCU_USART1);
	rcu_periph_clock_enable(RCU_GPIOA);

	/* Configure GPIO for USART1 (PA2-TX, PA3-RX) */
	gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_2 | GPIO_PIN_3);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_2 | GPIO_PIN_3);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2 | GPIO_PIN_3);

	/* USART1 configuration */
	usart_deinit(USART1);
	usart_baudrate_set(USART1, bound);
	usart_word_length_set(USART1, USART_WL_8BIT);
	usart_stop_bit_set(USART1, USART_STB_1BIT);
	usart_parity_config(USART1, USART_PM_NONE);
	usart_hardware_flow_rts_config(USART1, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(USART1, USART_CTS_DISABLE);
	usart_receive_config(USART1, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART1, USART_TRANSMIT_ENABLE);
	usart_enable(USART1);

	/* Enable RX interrupt */
	usart_interrupt_enable(USART1, USART_INT_RBNE);

	/* NVIC configuration */
	nvic_irq_enable(USART1_IRQn, 3, 3);
}

/* GD32 USART1 IRQ handler (STM32 USART2 -> GD32 USART1) */
void USART1_IRQHandler(void)
{
	if(usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE) != RESET)
	{
		usart_interrupt_flag_clear(USART1, USART_INT_FLAG_RBNE);
		Modbus_Master_Receive(usart_data_receive(USART1));
	}
}

void Slave_USART_Init(u32 bound)
{
	/* Enable clocks */
	rcu_periph_clock_enable(RCU_USART0);
	rcu_periph_clock_enable(RCU_GPIOA);

	/* Configure GPIO for USART0 (PA9-TX, PA10-RX) */
	gpio_af_set(GPIOA, GPIO_AF_7, GPIO_PIN_9 | GPIO_PIN_10);
	gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_9 | GPIO_PIN_10);
	gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_MAX, GPIO_PIN_9 | GPIO_PIN_10);

	/* USART0 configuration */
	usart_deinit(USART0);
	usart_baudrate_set(USART0, bound);
	usart_word_length_set(USART0, USART_WL_8BIT);
	usart_stop_bit_set(USART0, USART_STB_1BIT);
	usart_parity_config(USART0, USART_PM_NONE);
	usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);
	usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE);
	usart_receive_config(USART0, USART_RECEIVE_ENABLE);
	usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
	usart_enable(USART0);

	/* Enable RX interrupt */
	usart_interrupt_enable(USART0, USART_INT_RBNE);

	/* NVIC configuration */
	nvic_irq_enable(USART0_IRQn, 0, 0);

	/* Initialize DMA for USART0 TX */
	USART0_Tx_InitDMA();
}

static u8 DMA_Buffer[256];
static u8 DMA_Status = 0;

void USART0_Tx_InitDMA(void)
{
	dma_single_data_parameter_struct dma_init_struct;

	rcu_periph_clock_enable(RCU_DMA1);

	dma_deinit(DMA1, DMA_CH7);

	dma_init_struct.periph_addr = (uint32_t)&USART_DATA(USART0);
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
	dma_init_struct.memory0_addr = (uint32_t)DMA_Buffer;
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
	dma_init_struct.periph_memory_width = DMA_PERIPH_WIDTH_8BIT;
	dma_init_struct.circular_mode = DMA_CIRCULAR_MODE_DISABLE;
	dma_init_struct.direction = DMA_MEMORY_TO_PERIPH;
	dma_init_struct.number = 0;
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;
	dma_single_data_mode_init(DMA1, DMA_CH7, &dma_init_struct);

	/* CRITICAL: Select sub-peripheral for USART0 TX (DMA_SUBPERI4) */
	dma_channel_subperipheral_select(DMA1, DMA_CH7, DMA_SUBPERI4);

	/* Enable USART0 DMA transmit */
	usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE);

	/* NVIC for DMA transfer complete interrupt */
	nvic_irq_enable(DMA1_Channel7_IRQn, 0, 0);

	/* Enable DMA transfer complete interrupt */
	dma_interrupt_enable(DMA1, DMA_CH7, DMA_CHXCTL_FTFIE);
}

void USART0_SendWithDMA(u8* data, u32 len)
{
	while (DMA_Status == 1);
	memcpy(DMA_Buffer, data, len);
	dma_channel_disable(DMA1, DMA_CH7);
	dma_transfer_number_config(DMA1, DMA_CH7, len);
	dma_channel_enable(DMA1, DMA_CH7);
	DMA_Status = 1;
}

void USART0_SendByte(u8 byte)
{
	usart_data_transmit(USART0, byte);
	while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET);
}

void USART0_SendBytes(u8 *bytes, u32 len)
{
	while (len--) {
		USART0_SendByte(*bytes++);
	}
}

void USART0_SendString(u8 *str)
{
	while (*str) {
		USART0_SendByte(*str++);
	}
}

/* GD32 USART0 IRQ handler (STM32 USART1 -> GD32 USART0) */
void USART0_IRQHandler(void)
{
	if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET) {
		usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RBNE);
		ModBusRecByte(usart_data_receive(USART0));
	}
}

/* GD32 DMA1 Channel7 IRQ handler (STM32 DMA2_Stream7 -> GD32 DMA1_CH7) */
void DMA1_Channel7_IRQHandler(void)
{
	if (dma_interrupt_flag_get(DMA1, DMA_CH7, DMA_INT_FLAG_FTF) != RESET) {
		dma_interrupt_flag_clear(DMA1, DMA_CH7, DMA_INT_FLAG_FTF);
		dma_flag_clear(DMA1, DMA_CH7, DMA_FLAG_FTF);
		DMA_Status = 0;
		Modbus_ResetToIdle();
	}
}

void WriteToFlash(u32* dataArray, u16 len, u8 sector, u8* flashAddr)
{
	fmc_unlock();
	fmc_sector_erase((uint32_t)flashAddr);
	for (u16 i = 0; i < len; i += 4) {
		fmc_word_program((uint32_t)(flashAddr + i), *(u32*)(dataArray + i));
	}
	fmc_lock();
}
