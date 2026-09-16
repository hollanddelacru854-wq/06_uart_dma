#ifndef __UART_PROCESS_H__
#define __UART_PROCESS_H__

#include <stdint.h>

void uart_rec_A_func(void *argument);

uint8_t buffer1[1] = {0};
uint8_t buffer2[2] = {0};

#endif // end __UART_PROCESS_H__
