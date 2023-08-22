#include "sys.h"
#include "usart.h"
#include "delay.h"

#define USART1_DR_ARR         0X40013804
#define LEN 27
unsigned char dr16_rbuff[LEN];    //每隔7ms发送一个25个字节的数据包
u16 SBUS_Ch[11];              //解算以后的遥控器数据

//设置NVIC分组
//NVIC_Group:NVIC分组 0~4 总共5组
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group) {
    u32 temp, temp1;
    temp1 = (~NVIC_Group) & 0x07;//取后三位
    temp1 <<= 8;
    temp = SCB->AIRCR;  //读取先前的设置
    temp &= 0X0000F8FF; //清空先前分组
    temp |= 0X05FA0000; //写入钥匙
    temp |= temp1;
    SCB->AIRCR = temp;  //设置分组
}
//设置NVIC
//NVIC_PreemptionPriority:抢占优先级
//NVIC_SubPriority       :响应优先级
//NVIC_Channel           :中断编号
//NVIC_Group             :中断分组 0~4
//注意优先级不能超过设定的组的范围!否则会有意想不到的错误
//组划分:
//组0:0位抢占优先级,4位响应优先级
//组1:1位抢占优先级,3位响应优先级
//组2:2位抢占优先级,2位响应优先级
//组3:3位抢占优先级,1位响应优先级
//组4:4位抢占优先级,0位响应优先级
//NVIC_SubPriority和NVIC_PreemptionPriority的原则是,数值越小,越优先

void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group) {
    u32 temp;
    MY_NVIC_PriorityGroupConfig(NVIC_Group);//设置分组
    temp = NVIC_PreemptionPriority << (4 - NVIC_Group);
    temp |= NVIC_SubPriority & (0x0f >> NVIC_Group);
    temp &= 0xf;//取低四位
    NVIC->ISER[NVIC_Channel / 32] |= (1 << NVIC_Channel % 32);//使能中断位(要清除的话,相反操作就OK)
    NVIC->IP[NVIC_Channel] |= temp << 4;//设置响应优先级和抢断优先级
}

//dma串口接收配置
void DMA1_RX1Init(void) {
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        /*开启DMA时钟*/
    //DMA_Cmd(DMA1_Channel5, DISABLE);                    //关闭dma才可以进行配置
    DMA_DeInit(DMA1_Channel5);                            /*配置DMA1的5通道*/    /*恢复缺省值*/
    DMA_InitStructure.DMA_BufferSize = LEN;                /*传输大小DMA_BufferSize=SENDBUFF_SIZE*/
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;    //传输方向外设到内存/*DMA_DIR_PeripheralSRC外设是源 ，DMA_DIR_PeripheralDST外设是目的地*/
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;              /*禁止内存到内存的传输	*/
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(dr16_rbuff);/*内存地址(要传输的变量的指针)*/
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;/*内存数据单位*/
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//存储器增量模式
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;        //循环模式
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_ARR;//DMA外设地址  (uint32_t)&(USART1->DR)
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据长度:8位
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设非增量模式/*外设地址不增*/
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//set the piriority
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);//开启dma中断请求，这样才能进入dma中断服务函数
    //配置dma中断
    MY_NVIC_Init(0, 2, DMA1_Channel5_IRQn, 2);//中断优先级设为
}


void SBUS_USART1_Config(u32 Baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    //不需要串口中断就不必要配置中断向量
    //	NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);


//  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);



    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);


    USART_InitStructure.USART_BaudRate = Baudrate;                                 //波特率设置
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件控制不使能
    USART_InitStructure.USART_Mode = USART_Mode_Rx;                                //只接收
    USART_InitStructure.USART_Parity = USART_Parity_Even;                          //奇偶校验位为偶校
    USART_InitStructure.USART_StopBits = USART_StopBits_2;                         //最后一位为终止位,这里2位停止位有待商榷，以前用的1位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //数据长度为8位
    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);                              //开启空闲,帧错,噪声,校验错中断
    USART_Cmd(USART1, ENABLE);


    DMA1_RX1Init();


}


//********************************以下为SBUS的处理函数****************************//
//DMA的接受服务函数
void DMA1_Channel5_IRQHandler(void) {
    if (DMA_GetFlagStatus(DMA1_IT_TC5) == SET)//DMA1 Channel5 transfer complete flag.
    {
//        SBUS_Ch[0] = (dr16_rbuff[0] | (dr16_rbuff[1] << 8)) & 0x07ff;                                // 右横
//        SBUS_Ch[1] = ((dr16_rbuff[1] >> 3) | (dr16_rbuff[2] << 5)) & 0x07ff;                         // 右竖
//        SBUS_Ch[2] = ((dr16_rbuff[2] >> 6) | (dr16_rbuff[3] << 2) | (dr16_rbuff[4] << 10)) & 0x07ff; // 左横
//        SBUS_Ch[3] = ((dr16_rbuff[4] >> 1) | (dr16_rbuff[5] << 7)) & 0x07ff;                         // 左竖
//        SBUS_Ch[4] = ((dr16_rbuff[5] >> 4) & 0x000C) >> 2;                                           // 左S
//        SBUS_Ch[5] = ((dr16_rbuff[5] >> 4) & 0x0003);                                                // 右S
        if (dr16_rbuff[0] == 0x0f) {
            SBUS_Ch[1] = (dr16_rbuff[1] | (dr16_rbuff[2] << 8)) & 0x07ff;                       //通道1，右横，1400~1000~600
            SBUS_Ch[2] =
                ((dr16_rbuff[2] >> 3) | (dr16_rbuff[3] << 5)) & 0x07ff;                       //通道2，右竖，1800~1000~200
            SBUS_Ch[3] =
                ((dr16_rbuff[3] >> 6) | (dr16_rbuff[4] << 2) | (dr16_rbuff[5] << 10)) & 0x07ff; //通道3，左竖，1800~1000~200
            SBUS_Ch[4] =
                ((dr16_rbuff[5] >> 1) | (dr16_rbuff[6] << 7)) & 0x07ff;                       //通道4，左横，1800~1000~200
            SBUS_Ch[5] =
                ((dr16_rbuff[6] >> 4) | (dr16_rbuff[7] << 4)) & 0x07ff;                       //通道5，上200，中1000，下1800
            SBUS_Ch[6] =
                ((dr16_rbuff[7] >> 7) | (dr16_rbuff[8] << 1) | (dr16_rbuff[9] << 9)) & 0x07ff;  //通道6，上200，下1800
            SBUS_Ch[7] = ((dr16_rbuff[9] >> 2) | (dr16_rbuff[10] << 6)) & 0x07ff;
            SBUS_Ch[8] = ((dr16_rbuff[11] << 3) | (dr16_rbuff[10] >> 5)) & 0x07ff;
            SBUS_Ch[9] = ((dr16_rbuff[12]) | (dr16_rbuff[13] << 8)) & 0x07ff;
            SBUS_Ch[10] = ((dr16_rbuff[13] >> 13) | (dr16_rbuff[14] << 5)) & 0x07ff;
        }

        DMA_ClearFlag(DMA1_IT_TC5);
    }
}
