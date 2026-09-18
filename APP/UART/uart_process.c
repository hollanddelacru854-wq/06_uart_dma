
#include "uart_process.h"


QueueHandle_t queue_irq_rec_A = NULL;
static circular_buffer_t * g_circular_buffer_from_driver = NULL;



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
	
	//通过函数将缓冲区的指针传给另一指针（二级指针的思想）
	g_circular_buffer_from_driver = get_circular_buffer();
	
	
  for(;;)
  {
		
	xQueueReceive(queue_irq_rec_A,&receive_data,0xffff);
	log_i("receive_data = [%x]",receive_data);
	  
    //第1步：依次去将环形缓冲区的数据进行解析
	if ( NULL == g_circular_buffer_from_driver )
	{
		log_e("error pointer");
		return;
	}
   
	while( 0x00 != buffer_is_empty(g_circular_buffer_from_driver) )
	{
		uint8_t temp_data = 0;
		
		static uint8_t data_counter = 0;
		static uint8_t temp_data_array[20] = {0x00};
		
		
	   
		if ( 0x00 == get_data(g_circular_buffer_from_driver,&temp_data))
		{
//			log_i("buffer_get_success");
		}
		osDelay(2);
		
		

		// 第2步、寻找帧头，如果检测到帧头，则开始输出帧头后的数据
		static uint32_t status = FRAME_NOT_DETECTED;
		
		switch(status)
		{
			case FRAME_NOT_DETECTED:
				   //看一下帧头来了没，来了就切换状态
			   if( FRAME_HEAD_FLAG == temp_data )
			   {
				   status = FRAME_HEAD;
				   log_i("Data packet start");
			   }
			break;
			
			case FRAME_HEAD:
			// 数据包校验    
			//检测到帧尾之前，将数据全部存起来，检测到帧尾后，
			//开始计算校验和，若相等，则输出
			   if( FRAME_END_FLAG == temp_data )
			   {
					log_i("Data packet end");
					status = FRAME_NOT_DETECTED;
					
					// 校验和开始计算
					uint32_t data_sum = temp_data_array[data_counter-1];
					uint32_t data_sum_temp = 0;
					
					log_i("data_sum = [%d]",data_sum);
			   
					 for(int i = 0; i < (data_counter-1); i++)
					{
						log_i("receive data = [%d]",temp_data_array[i]);
					}

					// 计算校验和
					for(int i = 0; i < (data_counter-1); i++)
					{
						data_sum_temp += temp_data_array[i];
					}
					
					// 计算出来的校验和
					log_i("calculated data_sum = [%d]",data_sum_temp);
					
					// 打印真正想传入的数据
					if( data_sum == data_sum_temp )
					{
						for(int i = 0; i < (data_counter-1); i++)
						{
							log_i("calculated receive data = [%d]",temp_data_array[i]);
						}
					}
					
					for(int i = 0; i < (data_counter-1); i++)
					{
						temp_data_array[data_counter]=0x00;
					}
					data_counter = 0;
					
				   
			   }
			   else
			   {
					log_i("FRAME_HEAD data = [%d]",temp_data );
					
				   
				   //暂存数据等待帧尾到来
				   temp_data_array[data_counter] = temp_data;
				   
				   data_counter++;
				   
				   log_i("data_counter = [%d]",data_counter);
			   }
				break;
			
		
		}
		
	}
    osDelay(1);
  }
  /* USER CODE END uart_rec_A_func */
}
