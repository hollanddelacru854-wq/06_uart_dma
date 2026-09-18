#ifndef __UART_PROCESS_H__
#define __UART_PROCESS_H__

#include <stdint.h>

#include "FreeRTOS.h"
#include "cmsis_os2.h" 
#include "task.h" 
#include "queue.h"
#include "elog.h"
#include "mid_circular_buffer.h"

#include "bsp_uart_driver.h"

#define FRAME_NOT_DETECTED (0x01) //等待数据状态
#define FRAME_HEAD         (0x02) //有帧头后的状态
#define FRAME_END          (0x03) //有帧尾后的状态


#define FRAME_HEAD_FLAG    (0xFE) //帧头
#define FRAME_END_FLAG     (0xFF) //帧尾

void uart_rec_A_func(void *argument);

uint8_t buffer1[1] = {0};
uint8_t buffer2[2] = {0};

#endif // end __UART_PROCESS_H__
