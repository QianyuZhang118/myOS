#include "tinyOS.h"
#include "gd32f4xx.h"
#include "time.h"

void LedInit()
{
	// ʹ��IOʱ��
	rcu_periph_clock_enable(RCU_GPIOF);
	
	// IOģʽѡ��
	gpio_mode_set(GPIOF, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, GPIO_PIN_ALL);
	
	// IO���ģʽѡ��
	gpio_output_options_set(GPIOF, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO_PIN_ALL);
	
}

void UartInit(void) // TX PC12  RX PD2
{
	// ʹ��GPIOʱ��
	rcu_periph_clock_enable(RCU_GPIOC);
	rcu_periph_clock_enable(RCU_GPIOD);

	//IO����
	gpio_mode_set(GPIOC, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_12);  // IOģʽѡ��
	gpio_mode_set(GPIOD, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_2);
	
	gpio_af_set(GPIOC, GPIO_AF_8, GPIO_PIN_12);  // ���ù�������
	gpio_af_set(GPIOD, GPIO_AF_8, GPIO_PIN_2);
	
	gpio_output_options_set(GPIOC, GPIO_OTYPE_PP, GPIO_OSPEED_25MHZ, GPIO_PIN_12);  // ���IOѡ��

	// ʹ�ܴ���ʱ��
	rcu_periph_clock_enable(RCU_UART4);
	
	// ��λ����
	usart_deinit(UART4);
	
	//����������
	usart_baudrate_set(UART4, 115200);
	
	// У��λ
	usart_parity_config(UART4, USART_PM_NONE);
	
	// ֹͣλ
	usart_stop_bit_set(UART4, USART_STB_1BIT);
	
	usart_hardware_flow_rts_config(UART4, USART_RTS_DISABLE); /* ����rts */

	usart_hardware_flow_cts_config(UART4, USART_CTS_DISABLE); /* ��Ӳ������������ */

	// �ֳ�
	usart_word_length_set(UART4, USART_WL_8BIT);
	
	// ʹ�ܴ��ڷ���
	usart_transmit_config(UART4, USART_TRANSMIT_ENABLE);
	
	// ʹ�ܴ��ڽ���
	usart_receive_config(UART4, USART_RECEIVE_ENABLE);
	
	// ���ջ������ǿ��ж�ʹ��
	usart_interrupt_enable(UART4, USART_INT_RBNE);
	
	// �ж����ȼ�����
	// nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);
	
	// ʹ���ж�
	nvic_irq_enable(UART4_IRQn, 3, 3);
	
	// ʹ�ܴ���
	usart_enable(UART4);
}


/* 1B,4B����������,1B=8bit */
//void uart_data_transmit(uint8_t arr[], uint32_t len) 
//{
//      uint32_t i;
//        for(i=0; i<len; i++) 
//     {
//        usart_data_transmit(USART0, arr[i]); /* transmit */
//        while (usart_flag_get(USART0, USART_FLAG_TBE)== RESET); /* ��ȡ�������Ƿ�Ϊ�� */
//        }
//}


void delay(uint32_t ms)
{
	while(--ms)
	{
	}
}

/* ����4���жϴ����� */
void UART4_IRQHandler(void)
{
		uint8_t recData;
		if(RESET != usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE)) /* receive */
		{ 
				// ��ȡ��־λ�������־λ�������һֱ�����ж�
				usart_interrupt_flag_clear(UART4,USART_INT_FLAG_RBNE);
			
        /* receive data */
				recData = usart_data_receive(UART4); /* ���� */   
			
				gpio_bit_toggle(GPIOF, GPIO_PIN_13);
    }

}


// ���ڴ�ӡ���
int myPrintf(uint32_t usart_periph, uint8_t data)
{
	usart_data_transmit(usart_periph, (uint8_t)data);
	// �������ݼĴ����ձ�ʾ�����Ѿ����ͳ�ȥ
	while (RESET == usart_flag_get(usart_periph, USART_FLAG_TBE));
	return data;
}

