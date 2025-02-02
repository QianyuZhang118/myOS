/* 延时相关代码 */

#include "tinyOS.h"

/* 新的延时函数 */
void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();
	
	tTimeTaskWait(currentTask, delay);  // 插入延时队列
	tTaskSchedUnRdy(currentTask);  // 移出就绪列表
	
	tTaskExitCritical(status);
	
	tTaskSched();  // 调用延时函数后，切换到其他任务运行
}
