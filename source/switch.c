/* ��CPU��صĺ��� */

#include "gd32f4xx.h"
#include "tinyOS.h"

//���ڽ���PendSV�ж�
#define NVIC_INT_CTRL 		0xE000ED04    // �жϿ��ƺ�״̬�Ĵ���
#define NVIC_PENDSVSET   	0x10000000    // ISCR  28bit��1(PENDSVSET��1)  ����PendSV�쳣
//��������PendSVΪ������ȼ�
#define NVIC_SYSPRI2      0xE000ED22    // ����PendSV���ȼ�  ��λ�Ĵ���
#define NVIC_PENDSV_PRI   0x000000FF    // ȫ����1 

#define MEM32(addr)   *(volatile unsigned long *)(addr)
#define MEM8(addr)    *(volatile unsigned char *)(addr)


/* �ٽ�����������-���� */
uint32_t tTaskEnterCritical(void)
{
	// �õ�PRIMASK�жϿ��ƼĴ������Ա�������ٽ���ǰ�ļĴ���״̬
	uint32_t primask = __get_PRIMASK();
	// �ر��ж�
	__disable_irq();
	return primask;
}

/* �ٽ�����������-�˳� */
void tTaskExitCritical(uint32_t status)
{
	__set_PRIMASK(status);
}


/* PendSV�жϴ����� */
__asm void PendSV_Handler (void)
{
		IMPORT currentTask
		IMPORT nextTask
		MRS  R0,PSP   				//R0 = PSP
		CBZ  R0,SVC					// �ж�R0�Ƿ�Ϊ0��Ϊ0��˵����ǰҪ���г�ʼ������ת��SVC��ǩ
		STMDB R0!,{R4-R11}    //*R0 = R4~R11  R0++
		LDR  R1,=currentTask  //R1 = &currentTask
		LDR  R1,[R1]          //R1 = currentTask	
		STR  R0,[R1]          //*currentTask = PSP
SVC
	 LDR  R0,=currentTask   //R0 = &currentTask
	 LDR  R1,=nextTask      //R1 = &nextTask
	 LDR  R2,[R1]           //R2 = nextTask
	 STR  R2,[R0]           //currentTask = nextTask  
	 LDR  R0,[R2]   				//&currentTask = *nextTask
	 LDMIA R0!,{R4-R11}			//for(int i=4; i<=11;i++)
													//{Ri = *(*nextTask); *nextTask++;}     (Ri = *R0 R0++)
	 MSR  PSP,R0            //����PSPΪ��һ������
	 ORR  LR,LR,#0x04       //��LR��Ӧ��λ����֤����ʱʹ�õ���PSP
	 BX   LR                //�쳣�˳��Զ��ָ�R0~R3,R12,LR,PC,xPSR 
}


/* ��ʼ�������� */
void tTaskRunFirst()
{
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;  // ����PendSVΪ������ȼ�
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // ����PendSV�ж�
	
}

void tTaskSwitch()
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // ����PendSV�ж�
}

