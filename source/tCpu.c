/* CPU�ӿ���ش��� */

#include "tinyOS.h"
#include "gd32f4xx.h"

/* SysTick��ʼ������, ����ϵͳʱ�ӽ��� */
void SysTick_init(uint32_t ms)
{
    // �������ֵ��������ת��Ϊ��������
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; 
	
    // ���� SysTick �жϵ����ȼ�Ϊ���
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	
    // �����ǰ����ֵ��ȷ���� LOAD ��ʼ����
    SysTick->VAL = 0;
	
    // ���ò�ʹ�� SysTick ��ʱ��
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // ѡ��ʱ��Դ��ϵͳʱ�ӣ�
                    SysTick_CTRL_TICKINT_Msk   | // ʹ�� SysTick �ж�
                    SysTick_CTRL_ENABLE_Msk;    // ʹ�� SysTick ������
}

/*  SysTick�жϴ����� */
void  SysTick_Handler()
{
	tTaskSystemTickHandler();  // ϵͳʱ���ж��е�����ʱ������
}
