/* ��ʱ��ش��� */

#include "tinyOS.h"

/* �µ���ʱ���� */
void tTaskDelay(uint32_t delay)
{
	uint32_t status = tTaskEnterCritical();
	
	tTimeTaskWait(currentTask, delay);  // ������ʱ����
	tTaskSchedUnRdy(currentTask);  // �Ƴ������б�
	
	tTaskExitCritical(status);
	
	tTaskSched();  // ������ʱ�������л���������������
}
