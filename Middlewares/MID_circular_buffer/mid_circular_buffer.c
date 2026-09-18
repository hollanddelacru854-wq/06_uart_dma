#include "mid_circular_buffer.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "elog.h"


//创建环形缓冲区
circular_buffer_t * create_empty_circular_buffer (void)
{
    circular_buffer_t * p_buffer_temp = NULL;
    
    p_buffer_temp = (circular_buffer_t *) malloc(sizeof(circular_buffer_t));
    if ( NULL == p_buffer_temp ) 
    {
        log_e("error: create_empty_circular_buffer");
        return NULL;
    }
    
    
    memset ( p_buffer_temp, 0, sizeof(circular_buffer_t));
    
    return p_buffer_temp;
}


//判断缓冲区是否空
uint8_t buffer_is_empty(circular_buffer_t * p_buffer)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }        
    if ( p_buffer->head == p_buffer->tail )
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }        
}


//判断缓冲区是否满
uint8_t buffer_is_full(circular_buffer_t * p_buffer)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }  
    if (
        (( (p_buffer->head)%CIRCULAR_BUFFER_SIZE) + 1) == 
        ( (p_buffer->tail)%CIRCULAR_BUFFER_SIZE)
        )
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }      
}


//向缓冲区插入数据
uint8_t insert_data (circular_buffer_t * p_buffer,data_type_t data)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    if( 0x00 == buffer_is_full(p_buffer) )
    {
        return 0xFE;
    }
    
    p_buffer->data[(p_buffer->head)%CIRCULAR_BUFFER_SIZE] = data;
    
    p_buffer->head++;
    
    return 0x00;


}



//在缓冲区中获取数据
uint8_t get_data (circular_buffer_t * p_buffer, data_type_t * data)
{
    if ( NULL == p_buffer )
    {
        return 0xFF;
    }
    if( 0x00 == buffer_is_empty(p_buffer) )
    {
        return 0xFE;
    }
    *data = p_buffer->data[(p_buffer->tail)%CIRCULAR_BUFFER_SIZE];
    p_buffer->tail++;
    
    return 0x00;
}


