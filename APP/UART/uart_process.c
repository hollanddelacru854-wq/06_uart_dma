
#include "uart_process.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h" 
#include "task.h" 
#include "queue.h"
#include "elog.h"


QueueHandle_t queue_irq_rec_A = NULL;

void uart_rec_A_func(void *argument)
{
  /* USER CODE BEGIN uart_rec_A_func */
  /* Infinite loop */
	static uint32_t receive_data = 0;
	log_i("uart_rec_A_func Init Success");
	queue_irq_rec_A = NULL;
	queue_irq_rec_A = xQueueCreate( 1, 4 );
	if ( NULL == queue_irq_rec_A)
	{
		log_i("queue_irq_rec_A Init failed");
		
	} else {
		log_i("queue_irq_rec_A Init Success");
		log_i("queue_irq_rec_A = [%x]",queue_irq_rec_A);
	}
  for(;;)
  {
		
		xQueueReceive(queue_irq_rec_A,&receive_data,0xffff);
		log_i("receive_data = [%x]",receive_data);
    osDelay(1);
  }
  /* USER CODE END uart_rec_A_func */
}
