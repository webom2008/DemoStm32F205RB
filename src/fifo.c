/*
 * fifo.c
 *
 *  Created on: 2013-09-25
 *      Author: QiuWeibo
 */
#include <string.h>

#define FIFO_GLOBALS
#include "fifo.h"
#undef FIFO_GLOBALS

//#define FIFO_DEBUG_INFO
#ifdef FIFO_DEBUG_INFO
#include <stdio.h>
#define FIFO_DBG_INFO(fmt, arg...) printf("\r\n[FIFO]:"fmt, ##arg)
#else
#define FIFO_DBG_INFO(fmt, arg...)
#endif


#ifndef NULL
#define NULL    0
#endif

void gp_fifo_init(  FIFO_StructDef *fifo_ptr,
                    const void *buffer_ptr,
                    const u32 data_size,
                    const u32 fifo_size )
{
    if (NULL == fifo_ptr || NULL == buffer_ptr) return;
    fifo_ptr->buffer        = (u8 *)buffer_ptr;
    fifo_ptr->write_ptr     = fifo_ptr->buffer;
    fifo_ptr->read_ptr      = fifo_ptr->buffer;
    fifo_ptr->data_size     = data_size;
    fifo_ptr->fifo_size     = fifo_size;
    fifo_ptr->fifo_empty    = 1;
    fifo_ptr->fifo_full     = 0;
    memset(fifo_ptr->buffer, 0, (data_size * fifo_size));
    FIFO_DBG_INFO("gp_fifo_init data_size=%d, fifo_size=%d",data_size, fifo_size);
}

int gp_fifo_push(FIFO_StructDef *fifo_ptr, void *buffer_ptr)
{
    if (NULL == fifo_ptr || NULL == buffer_ptr) return -1;
    if(!fifo_ptr->fifo_full)
    {
        memcpy(fifo_ptr->write_ptr, buffer_ptr, fifo_ptr->data_size);
        
        fifo_ptr->write_ptr += fifo_ptr->data_size;
        if(fifo_ptr->write_ptr >= fifo_ptr->buffer + fifo_ptr->data_size * fifo_ptr->fifo_size)
            fifo_ptr->write_ptr = fifo_ptr->buffer;
        if(fifo_ptr->write_ptr == fifo_ptr->read_ptr)
        {
            fifo_ptr->fifo_full=1;
        }
        fifo_ptr->fifo_empty=0;
        return 0;
    }
    else //fifo is full
    {
        FIFO_DBG_INFO("gp_fifo_push Full Error");
        return -1;
    }
}

//@return : 0 -- OK,  -1 -- Error
int gp_fifo_pop(FIFO_StructDef* fifo_ptr, void* buffer_ptr)
{
    if (NULL == fifo_ptr || NULL == buffer_ptr) return -1;
    
    if(!fifo_ptr->fifo_empty)
    {
        memcpy(buffer_ptr, fifo_ptr->read_ptr, fifo_ptr->data_size);
        
        fifo_ptr->read_ptr += fifo_ptr->data_size;
        if(fifo_ptr->read_ptr >= fifo_ptr->buffer + fifo_ptr->data_size * fifo_ptr->fifo_size)
            fifo_ptr->read_ptr = fifo_ptr->buffer;
        if(fifo_ptr->read_ptr == fifo_ptr->write_ptr)
        {
            fifo_ptr->fifo_empty = 1;
        }
        fifo_ptr->fifo_full = 0;
        return 0;
    }
    else
    {
        return -1;
    }
}

u32 gp_fifo_count(FIFO_StructDef *fifo_ptr)
{
    if (NULL == fifo_ptr) return 0;
    
    if(fifo_ptr->fifo_full)
    {
        return fifo_ptr->fifo_size;
    }
    else
    {
        if(fifo_ptr->write_ptr >= fifo_ptr->read_ptr)
        {
            return(fifo_ptr->write_ptr - fifo_ptr->read_ptr)/fifo_ptr->data_size;
        }
        else
        {
            return fifo_ptr->fifo_size-(fifo_ptr->read_ptr - fifo_ptr->write_ptr) / fifo_ptr->data_size;
        }
    }
}

void gp_fifo_reset(FIFO_StructDef *fifo_ptr)
{
    if (NULL == fifo_ptr) return;
    
    fifo_ptr->read_ptr = fifo_ptr->write_ptr; 
}


