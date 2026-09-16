#include "bsp_uart_driver.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h" 
#include "task.h" 
#include "queue.h"

#include "mid_circular_buffer.h"
#include "elog.h"



#define BUFFER_A 0
#define BUFFER_B 1

extern QueueHandle_t queue_irq_rec_A;

//uint8_t flag_AB = BUFFER_A;

//uint8_t g_data_buffer_A[1] = {0x00};
//uint8_t g_data_buffer_B[1] = {0x00};


uint8_t g_data_buffer = 0;

//指向环形缓冲区的指针
static circular_buffer_t * g_circular_buffer_irq_thread = NULL;


void uart_driver_func(void *argument)
{
  /* USER CODE BEGIN uart_rec_A_func */
	/* DEBUG USART */
	
	uint8_t temp_data = 0;
	
    //创建一个环形缓冲区
    circular_buffer_t * p_circular_buffer = create_empty_circular_buffer();
    if ( NULL == p_circular_buffer )
    {
        log_e("circular_buffer create failed");
    }
	
	//指向环形缓冲区
	g_circular_buffer_irq_thread = p_circular_buffer;
	
    log_i("circular_buffer create Success.");
    
	//判断空
    if ( 0x00 == buffer_is_empty(p_circular_buffer))
    {
        log_i("buffer_is_empty");
    }
    
	//判断满
    if ( 0x00 == buffer_is_full(p_circular_buffer) )
    {
        log_i("buffer_is_full");
    }
    
	//插入数据
    if ( 0x00 == insert_data(p_circular_buffer ,15))
    {
        log_i("buffer_inster_success");
        
    }
    
	//取出数据
    if ( 0x00 == get_data(p_circular_buffer,&temp_data))
    {
        log_i("buffer_get_success");
    }
    log_i("buffer_read_out = [%d]",temp_data );
    
	
    if ( 0x00 == buffer_is_empty(p_circular_buffer))
    {
        log_i("buffer_is_empty");
    }
    if ( 0x00 == buffer_is_full(p_circular_buffer) )
    {
        log_i("buffer_is_full");
    }
    
//    flag_AB = BUFFER_A;
	
	HAL_StatusTypeDef ret = HAL_OK;
	
	//将数据写入g_data_buffer（进入中断后将数据搬运到环形缓冲区）
	ret = HAL_UART_Receive_IT(&huart1, &g_data_buffer, 1);
	
	
	if(HAL_OK == ret)
	{
		log_i("HAL UART Init Success.");
	} 
	else
	{
		log_i("HAL UART Init Failed");
	}
	/* DEBUG USART */
    

  for(;;)
  {
        
  }
  /* USER CODE END uart_rec_A_func */
}


/* USER CODE BEGIN 1 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
   
   //将串口数据搬运到环形缓冲区
	if(NULL == g_circular_buffer_irq_thread)
	{
	  log_e("error with NULL pointer of g_circular_buffer");
	  return;
	}
      
	uint8_t ret = 0;

	ret = insert_data(g_circular_buffer_irq_thread,g_data_buffer);

	//判断是否将数据放入环形缓冲区，成功放入就读出来（在中断写入和读出？不合适吧）
	if( 0x00 == ret )
	{
		uint8_t temp_data = 0;
		if ( 0x00 == get_data(g_circular_buffer_irq_thread, &temp_data))
		{
			log_i("circular_buffer_get_success");
		}
		log_i("buffer_read_out = [%d]",temp_data );
	}
	
	
	//触发下一次搬运
	HAL_StatusTypeDef ret_1 = HAL_OK;

	ret_1 = HAL_UART_Receive_IT(&huart1, &g_data_buffer, 1);


	if(HAL_OK == ret_1)
	{
		log_i("HAL UART Init Success.");
	} 
	else
	{
		log_i("HAL UART Init Failed");
	}
   
   
    
	
	
	
    
}
/* USER CODE END 1 */
