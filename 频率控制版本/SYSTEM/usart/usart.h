#ifndef __USART_H
#define __USART_H

#include "stdio.h"
#include "sys.h"

//ȫ�ֱ�������
extern u16 SBUS_Ch[11];
//********************************����Ϊ��������***************************//

void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group);

void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group);

void DMA1_RX1Init(void);

void SBUS_USART1_Config(u32 Baudrate);

#endif


