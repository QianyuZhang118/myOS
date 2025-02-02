#include "tinyOS.h"
#include "gd32f4xx.h"
#include "time.h"

void LedInit()
{
	// 使能IO时钟
	rcu_periph_clock_enable(RCU_GPIOF);
	
	// IO模式选择
	gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
	
	// IO输出模式选择
	gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_ALL);
	
}

void UartInit(void) // TX PC12  RX PD2
{
	// 使能GPIO时钟
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);

	//IO配置
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);  // IO模式选择
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	
	gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);  // 复用功能配置
	gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);
	
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_12);  // 输出IO选项

	// 使能串口时钟
	rcu_periph_clock_enable(RCU_UART4);
	
	// 复位串口
	usart_deinit(UART4);
	
	//波特率设置
	usart_baudrate_set(UART4, 115200);
	
	// 校验位
	usart_parity_config(UART4, USART_PM_NONE);
	
	// 停止位
	usart_stop_bit_set(UART4, USART_STB_1BIT);
	
	usart_hardware_flow_rts_config(UART4, USART_RTS_DISABLE); /* 禁用rts */

	usart_hardware_flow_cts_config(UART4, USART_CTS_DISABLE); /* 无硬件数据流控制 */

	// 字长
	usart_word_length_set(UART4, USART_WL_8BIT);
	
	// 使能串口发送
	usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
	
	// 使能串口接收
	usart_receive_config(UART4, USART_RECEIVE_ENABLE);
	
	// 接收缓冲区非空中断使能
	usart_interrupt_enable(UART4, USART_INT_RBNE);
	
	// 中断优先级分组
	// nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	
	// 使能中断
	nvic_irq_enable(UART4_IRQn, 3, 3);
	
	// 使能串口
	usart_enable(UART4);
}


/* 1B,4B，数据输入,1B=8bit */
//void uart_data_transmit(uint8_t arr[], uint32_t len) 
//{
//      uint32_t i;
//        for(i=0; i<len; i++) 
//     {
//        usart_data_transmit(USART0, arr[i]); /* transmit */
//        while (usart_flag_get(USART0, USART_FLAG_TBE)== RESET); /* 获取缓冲区是否为空 */
//        }
//}


void delay(uint32_t ms)
{
	while(--ms)
	{
	}
}

/* 串口4的中断处理函数 */
void UART4_IRQHandler(void)
{
		uint8_t recData;
		if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)) /* receive */
		{ 
				// 获取标志位后清除标志位，否则会一直进入中断
				usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
			
        /* receive data */
				recData = usart_data_receive(UART4); /* 接收 */   
			
				gpio_bit_toggle(GPIOF, GPIO_PIN_13);
    }

}


// 串口打印输出
int myPrintf(uint32_t usart_periph, uint8_t data)
{
	usart_data_transmit(usart_periph, (uint8_t)data);
	// 发送数据寄存器空表示数据已经发送出去
	while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE));
	return data;
}

