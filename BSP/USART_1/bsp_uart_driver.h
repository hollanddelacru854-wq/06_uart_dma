#ifndef __BSP_UART_DRIVER_H__
#define __BSP_UART_DRIVER_H__

#include "usart.h"

#include "FreeRTOS.h"
#include "cmsis_os2.h" 
#include "task.h" 
#include "queue.h"

#include "mid_circular_buffer.h"
#include "elog.h"

void uart_driver_func(void *argument);


circular_buffer_t * get_circular_buffer(void);





void uart_driver_func(void *argument);

#endif // end __BSP_UART_DRIVER_H__
