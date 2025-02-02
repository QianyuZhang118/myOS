/* 与CPU相关的函数 */

#include "gd32f4xx.h"
#include "tinyOS.h"

//用于进入PendSV中断
#define NVIC_INT_CTRL 		0xE000ED04    // 中断控制和状态寄存器
#define NVIC_PENDSVSET   	0x10000000    // ISCR  28bit置1(PENDSVSET置1)  进入PendSV异常
//用于设置PendSV为最低优先级
#define NVIC_SYSPRI2      0xE000ED22    // 设置PendSV优先级  八位寄存器
#define NVIC_PENDSV_PRI   0x000000FF    // 全部置1 

#define MEM32(addr)   *(volatile unsigned long *)(addr)
#define MEM8(addr)    *(volatile unsigned char *)(addr)


/* 临界区保护函数-进入 */
uint32_t tTaskEnterCritical(void)
{
	// 得到PRIMASK中断控制寄存器，以保存进入临界区前的寄存器状态
	uint32_t primask = __get_PRIMASK();
	// 关闭中断
	__disable_irq();
	return primask;
}

/* 临界区保护函数-退出 */
void tTaskExitCritical(uint32_t status)
{
	__set_PRIMASK(status);
}


/* PendSV中断处理函数 */
__asm void PendSV_Handler (void)
{
		IMPORT currentTask
		IMPORT nextTask
		MRS  R0,PSP   				//R0 = PSP
		CBZ  R0,SVC					// 判断R0是否为0，为0则说明当前要运行初始任务，跳转到SVC标签
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
	 MSR  PSP,R0            //设置PSP为下一个任务
	 ORR  LR,LR,#0x04       //将LR相应置位，保证返回时使用的是PSP
	 BX   LR                //异常退出自动恢复R0~R3,R12,LR,PC,xPSR 
}


/* 开始运行任务 */
void tTaskRunFirst()
{
	__set_PSP(0);
	
	MEM8(NVIC_SYSPRI2) = NVIC_PENDSV_PRI;  // 设置PendSV为最低优先级
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // 进入PendSV中断
	
}

void tTaskSwitch()
{
	MEM32(NVIC_INT_CTRL) = NVIC_PENDSVSET;  // 进入PendSV中断
}

