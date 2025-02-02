/* Ӧ����ش��� */

#include "tinyOS.h"
#include "gd32f4xx_gpio.h"

//void delay()
//{
//	int i;
//	for(i = 0; i < 0xFF; ++i)
//	{}
//}


/*��������*/
tTask tTask1;
tTask tTask2;
tTask tTask3;
tTask tTask4;

/* ����������Ϣ�ṹ�� */
tTaskInfo taskInfo1;
tTaskInfo taskInfo2;
tTaskInfo taskInfo3;
tTaskInfo taskInfo4;

/*�����ջ�ռ�*/
tTaskStack task1Env[1024];
tTaskStack task2Env[1024];
tTaskStack task3Env[1024];
tTaskStack task4Env[1024];


/*������*/
int task1Flag;
void task1Entry(void * param)
{
	
	// GPIO����


	for(;;)
	{

		FlagStatus stat = usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE);
		
		task1Flag ^= 1;
		
		// gpio_bit_write(GPIOF, GPIO_PIN_13, (task1Flag == 1 ? SET : RESET));
		
		// usart_data_transmit(USART5, 0xFF);
		
		myPrintf(UART4, 0xFF);
		
		stat = usart_interrupt_flag_get(UART4, USART_INT_FLAG_RBNE);
		
		tTaskDelay(TICKS_PER_SEC);  // ��ʱ1s
		
		// delay(100000);
		
	}
}

int task2Flag;
void task2Entry(void * param)
{
	for(;;)
	{	 
		task2Flag ^= 1;
		
		// gpio_bit_write(GPIOF, GPIO_PIN_14, (task2Flag == 1 ? SET : RESET));
		
		tTaskDelay(TICKS_PER_SEC / 2);  // ��ʱ0.5 s


	}
}

int task3Flag;
void task3Entry(void * param)
{
	for(;;)
	{
		task3Flag = 0;
		tTaskDelay(1);
		task3Flag = 1;
		tTaskDelay(1);

	}
}

int task4Flag;
void task4Entry(void * param)
{
	for(;;)
	{
		
		task4Flag = 0;
		tTaskDelay(1);
		task4Flag = 1;
		tTaskDelay(1);
	}
}



/* �����ʼ������ */
void tInitApp(void)
{
		/*�����ʼ��*/
	tTaskInit(&tTask1, task1Entry, (void *)0x11111111, 0, task1Env, sizeof(task1Env));
	tTaskInit(&tTask2, task2Entry, (void *)0x22222222, 1, task2Env, sizeof(task2Env));
	tTaskInit(&tTask3, task3Entry, (void *)0x22222222, 1, task3Env, sizeof(task3Env));
	tTaskInit(&tTask4, task4Entry, (void *)0x44444444, 1, task4Env, sizeof(task4Env));
}


