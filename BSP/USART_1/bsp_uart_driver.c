#include "bsp_uart_driver.h"

#include "usart.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h" 
#include "task.h" 
#include "queue.h"

#include "mid_circular_buffer.h"
#include "SEGGER_RTT.h"



#define BUFFER_A 0
#define BUFFER_B 1

extern QueueHandle_t queue_irq_rec_A;

uint8_t flag_AB = BUFFER_A;

uint8_t g_data_buffer_A[1] = {0x00};
uint8_t g_data_buffer_B[1] = {0x00};



void uart_driver_func(void *argument)
{
  /* USER CODE BEGIN uart_rec_A_func */
	/* DEBUG USART */
	
    //0. alloc the ring buffer
    circular_buffer_t * p_circular_buffer = create_empty_circular_buffer();
    if ( NULL == p_circular_buffer )
    {
        SEGGER_RTT_printf(0,"circular_buffer create failed");
    }
    SEGGER_RTT_printf(0,"circular_buffer create Success.");
    
    if ( 0x00 == buffer_is_empty(p_circular_buffer))
    {
        SEGGER_RTT_printf(0,"buffer_is_empty");
    }
    
    if ( 0x00 == buffer_is_full(p_circular_buffer) )
    {
        SEGGER_RTT_printf(0,"buffer_is_full");
    }
    
    if ( 0x00 == insert_data(p_circular_buffer ,15))
    {
        SEGGER_RTT_printf(0 ,"buffer_inster_success");
        
    }
    uint8_t temp_data = 0;
    if ( 0x00 == get_data(p_circular_buffer,&temp_data))
    {
        SEGGER_RTT_printf(0,"buffer_get_success");
    }
    SEGGER_RTT_printf(0,"buffer_read_out = [%d]",temp_data );
    
    if ( 0x00 == buffer_is_empty(p_circular_buffer))
    {
        SEGGER_RTT_printf(0,"buffer_is_empty");
    }
    if ( 0x00 == buffer_is_full(p_circular_buffer) )
    {
        SEGGER_RTT_printf(0,"buffer_is_full");
    }
    
    flag_AB = BUFFER_A;
	
	HAL_StatusTypeDef ret = HAL_OK;
	
	ret = HAL_UART_Receive_IT(&huart1, g_data_buffer_A, 1);
	
	
	if(HAL_OK == ret)
	{
		SEGGER_RTT_printf(0,"HAL UART Init Success.");
	} 
	else
	{
		SEGGER_RTT_printf(0,"HAL UART Init Failed");
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
  /* Prevent unused argument(s) compilation warning */
  /* NOTE: This function should not be modified, when the callback is needed,
           the HAL_UART_RxCpltCallback could be implemented in the user file
   */
	SEGGER_RTT_printf(0,"HAL_UART_RxCpltCallback");
	
    
    
    HAL_StatusTypeDef ret = HAL_OK;
    
    if ( BUFFER_A == flag_AB )
    {
        SEGGER_RTT_printf(0,"g_data_buffer_A = [%c]",g_data_buffer_A[0]);
        ret = HAL_UART_Receive_IT(&huart1, g_data_buffer_B, 1);
        if ( HAL_OK != ret )
        {
            SEGGER_RTT_printf(0,"HAL_UART_Receive_IT error!");
        }
        flag_AB = BUFFER_B;
    } 
    else 
    {
        SEGGER_RTT_printf(0,"g_data_buffer_B = [%c]",g_data_buffer_B[0]);
        ret = HAL_UART_Receive_IT(&huart1, g_data_buffer_A, 1);
        if ( HAL_OK != ret )
        {
            SEGGER_RTT_printf(0,"HAL_UART_Receive_IT error!");
        }
        flag_AB = BUFFER_A;
    }
        
    
   
    
    //g_data_buffer_A
	//ret = xQueueSendFromISR( queue_irq_rec_A, &send_data_to_rec_A,NULL);
	//SEGGER_RTT_printf(0,"ret = [%d]",ret);
}
/* USER CODE END 1 */
