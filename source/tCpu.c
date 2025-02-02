/* CPU接口相关代码 */

#include "tinyOS.h"
#include "gd32f4xx.h"

/* SysTick初始化函数, 设置系统时钟节拍 */
void SysTick_init(uint32_t ms)
{
    // 计算加载值：将毫秒转换为计数周期
    SysTick->LOAD = ms * SystemCoreClock / 1000 - 1; 
	
    // 设置 SysTick 中断的优先级为最低
    NVIC_SetPriority(SysTick_IRQn, (1 << __NVIC_PRIO_BITS) - 1);
	
    // 清除当前计数值，确保从 LOAD 开始计数
    SysTick->VAL = 0;
	
    // 配置并使能 SysTick 定时器
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | // 选择时钟源（系统时钟）
                    SysTick_CTRL_TICKINT_Msk   | // 使能 SysTick 中断
                    SysTick_CTRL_ENABLE_Msk;    // 使能 SysTick 计数器
}

/*  SysTick中断处理函数 */
void  SysTick_Handler()
{
	tTaskSystemTickHandler();  // 系统时钟中断中调用软定时器函数
}
