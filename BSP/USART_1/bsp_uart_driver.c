#include "bsp_uart_driver.h"



#define IRQ_SEND_TO_THREAD 0xA1A2A3A4
#define FRONT_SEND_TO_END  0xB1B2B3B4

#define BUFFER_A 0
#define BUFFER_B 1

extern QueueHandle_t queue_irq_rec_A;


uint8_t g_data_buffer = 0;

//指向环形缓冲区的指针
static circular_buffer_t * g_circular_buffer_irq_thread = NULL;
static QueueHandle_t       queue_uart_irq_thread        = NULL;


void uart_driver_func(void *argument)
{
  /* USER CODE BEGIN uart_rec_A_func */
	/* DEBUG USART */
	

	uint32_t receive_data = 0;
	
    //创建一个环形缓冲区
    circular_buffer_t * p_circular_buffer = create_empty_circular_buffer();
    if ( NULL == p_circular_buffer )
    {
        log_e("circular_buffer create failed");
    }
	
	
	log_i( "p_circular_buffer = [0x%p]",p_circular_buffer);
	//指向环形缓冲区
	g_circular_buffer_irq_thread = p_circular_buffer;
	
    log_i("circular_buffer create Success.");
    

	
	//创建串口和任务A间的队列
	queue_uart_irq_thread = xQueueCreate( 1, 4 );
    if ( NULL == queue_uart_irq_thread )
    {
        log_e("circular_buffer create failed");
    }
    log_i("queue_uart_irq_thread create success");
	
	
	
	//第一次启动串口接收数据（放入g_data_buffer）
	HAL_StatusTypeDef ret = HAL_OK;
	
//	ret = HAL_UART_Receive_IT(&huart1, &g_data_buffer, 1);
	
	//开启串口空闲中断接收+DMA半满全满
	HAL_UARTEx_ReceiveToIdle_DMA(&huart1,p_circular_buffer->data,CIRCULAR_BUFFER_SIZE);
	
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
	    xQueueReceive(queue_uart_irq_thread,&receive_data,0xffff);
		log_i("receive_data = [%x]",receive_data);
		if( IRQ_SEND_TO_THREAD == receive_data )
		{
			//2、将当前数据就绪的事件发送给任务B
			uint32_t send_to_end = FRONT_SEND_TO_END;
			BaseType_t ret_queue = pdTRUE;
			//开启复写模式避免出现send error
			ret_queue = xQueueGenericSend(  queue_irq_rec_A, &send_to_end, 0, queueOVERWRITE);
			
			if( pdTRUE != ret_queue)
			{
				log_e("send error");
				return;
			}
			
			log_d("xQueueSend to end send success"); 
		}
  /* USER CODE END uart_rec_A_func */
  }

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

//	//判断是否将数据放入环形缓冲区，成功放入就读出来（在中断写入和读出？不合适吧）
//	if( 0x00 == ret )
//	{
//		uint8_t temp_data = 0;
//		if ( 0x00 == get_data(g_circular_buffer_irq_thread, &temp_data))
//		{
//			log_i("circular_buffer_get_success");
//		}
//		log_i("buffer_read_out = [%d]",temp_data );
//	}
	
	
	//将数据放入缓冲区后通知任务A
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
	//开启复写模式
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, &send_to_thread, NULL, queueOVERWRITE);
	if( pdTRUE != ret_queue)
    {
        log_e("send error");
        return;
    }
	
	
	
	//触发下一次搬运
	HAL_StatusTypeDef ret_1 = HAL_OK;

	ret_1 = HAL_UART_Receive_IT(&huart1, &g_data_buffer, 1);
    
    
}



//将指向缓冲区的指针传出的函数
circular_buffer_t * get_circular_buffer(void)
{
        
    if ( NULL == g_circular_buffer_irq_thread )
    {
        return NULL;
    }
    
    return g_circular_buffer_irq_thread;
}



void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    log_d("HAL_UARTEx_RxEventCallback size = [%d]",Size ); 
}



//半满中断的处理函数
void dma_half_irq_callback(uint32_t number_of_data)
{
	//实现半满中断的逻辑

    uint32_t head_pos = 0;
    uint8_t ret = 0;
	
	
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
        log_d("get head pos error" ); 
    }

	
    //2.获取进入半满中断时，数据已经到达的位置：(CIRCULAR_BUFFER_SIZE/2)-1
    uint32_t current_data_pos = (CIRCULAR_BUFFER_SIZE/2);
    
	
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE/2);
    
	
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
    }
    
	
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
    
	
    //6.通知处理串口数据的任务B
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, &send_to_thread,NULL,queueOVERWRITE);
    log_d("dma_half_irq_callback ret_queue = [%d]",ret_queue); 
    
	
	
}


//全满中断的处理函数
void dma_comp_irq_callback(uint32_t number_of_data)
{
    //实现全满中断的逻辑
	
    uint32_t head_pos = 0;
    uint8_t ret = 0;
    
	
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
        log_d("get head pos error" ); 
    }


    //2.获取进入全满中断时，数据已经到达的位置：(CIRCULAR_BUFFER_SIZE)-1
    uint32_t current_data_pos = (CIRCULAR_BUFFER_SIZE);
    
	
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
    
	
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
    }
    
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
    
    //6.通知处理串口数据的任务B
    uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, &send_to_thread,NULL,queueOVERWRITE);
    log_d("dma_comp_irq_callback ret_queue = [%d]",ret_queue); 

	
	
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
        log_d("get head pos error" ); 
    }
    
    pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
    
    
    
}



//空闲中断的处理函数
void uart_idle_irq_callback(uint32_t number_of_data)
{
	
	//实现空闲中断的逻辑：取余，移动头指针
	
    uint32_t head_pos = 0;
    uint8_t ret = 0;
    
    //1.获取当前位置
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
        log_d("get head pos error" ); 
    }


    //2.获取进入空闲中断时，数据已经到达的位置：number_of_data-1
    uint32_t current_data_pos = number_of_data;

    
    //3.对heap进行取余数
    uint32_t pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);

    
    //4.算出当前应该偏移的数量
    uint32_t move_pos = 0x00;
    if(current_data_pos < pos_in_buffer)
    {
        move_pos = (current_data_pos+CIRCULAR_BUFFER_SIZE) - pos_in_buffer;
    } 
    else
    {
        move_pos = current_data_pos - pos_in_buffer;
    }
	
	
    //5.对head位置进行累加
    head_pos_increment(g_circular_buffer_irq_thread,move_pos);
	
	
    //6.通知处理串口数据的任务B
	uint32_t send_to_thread = IRQ_SEND_TO_THREAD;
    BaseType_t ret_queue = pdTRUE;
    ret_queue = xQueueGenericSendFromISR( queue_uart_irq_thread, &send_to_thread,NULL,queueOVERWRITE);
    log_d("uart_idle_irq_callback ret_queue = [%d]",ret_queue);
	
    ret=get_head_pos(g_circular_buffer_irq_thread,&head_pos);
    if( 0x00 != ret)
    {
        log_d("get head pos error" ); 
    }
    
    pos_in_buffer = head_pos%(CIRCULAR_BUFFER_SIZE);
    
    
}








/* USER CODE END 1 */
