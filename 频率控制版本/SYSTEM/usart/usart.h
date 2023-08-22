#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "sys.h"

//全局变量部分
extern u16 SBUS_Ch[11];
//********************************以下为函数声明***************************//

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);

void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group);

void DMA1_RX1Init(void);

void SBUS_USART1_Config(u32 Baudrate);

#endif


