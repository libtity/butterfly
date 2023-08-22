#include "sys.h"
#include "usart.h"
#include "delay.h"

#define USART1_DR_ARR         0X40013804
#define LEN 27
unsigned char dr16_rbuff[LEN];    //ÿ��7ms����һ��25���ֽڵ����ݰ�
u16 SBUS_Ch[11];              //�����Ժ��ң��������

//����NVIC����
//NVIC_Group:NVIC���� 0~4 �ܹ�5��
void MY_NVIC_PriorityGroupConfig(u8 NVIC_Group) {
    u32 temp, temp1;
    temp1 = (~NVIC_Group) & 0x07;//ȡ����λ
    temp1 <<= 8;
    temp = SCB->AIRCR;  //��ȡ��ǰ������
    temp &= 0X0000F8FF; //�����ǰ����
    temp |= 0X05FA0000; //д��Կ��
    temp |= temp1;
    SCB->AIRCR = temp;  //���÷���
}
//����NVIC
//NVIC_PreemptionPriority:��ռ���ȼ�
//NVIC_SubPriority       :��Ӧ���ȼ�
//NVIC_Channel           :�жϱ��
//NVIC_Group             :�жϷ��� 0~4
//ע�����ȼ����ܳ����趨����ķ�Χ!����������벻���Ĵ���
//�黮��:
//��0:0λ��ռ���ȼ�,4λ��Ӧ���ȼ�
//��1:1λ��ռ���ȼ�,3λ��Ӧ���ȼ�
//��2:2λ��ռ���ȼ�,2λ��Ӧ���ȼ�
//��3:3λ��ռ���ȼ�,1λ��Ӧ���ȼ�
//��4:4λ��ռ���ȼ�,0λ��Ӧ���ȼ�
//NVIC_SubPriority��NVIC_PreemptionPriority��ԭ����,��ֵԽС,Խ����

void MY_NVIC_Init(u8 NVIC_PreemptionPriority, u8 NVIC_SubPriority, u8 NVIC_Channel, u8 NVIC_Group) {
    u32 temp;
    MY_NVIC_PriorityGroupConfig(NVIC_Group);//���÷���
    temp = NVIC_PreemptionPriority << (4 - NVIC_Group);
    temp |= NVIC_SubPriority & (0x0f >> NVIC_Group);
    temp &= 0xf;//ȡ����λ
    NVIC->ISER[NVIC_Channel / 32] |= (1 << NVIC_Channel % 32);//ʹ���ж�λ(Ҫ����Ļ�,�෴������OK)
    NVIC->IP[NVIC_Channel] |= temp << 4;//������Ӧ���ȼ����������ȼ�
}

//dma���ڽ�������
void DMA1_RX1Init(void) {
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);        /*����DMAʱ��*/
    //DMA_Cmd(DMA1_Channel5, DISABLE);                    //�ر�dma�ſ��Խ�������
    DMA_DeInit(DMA1_Channel5);                            /*����DMA1��5ͨ��*/    /*�ָ�ȱʡֵ*/
    DMA_InitStructure.DMA_BufferSize = LEN;                /*�����СDMA_BufferSize=SENDBUFF_SIZE*/
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;    //���䷽�����赽�ڴ�/*DMA_DIR_PeripheralSRC������Դ ��DMA_DIR_PeripheralDST������Ŀ�ĵ�*/
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;              /*��ֹ�ڴ浽�ڴ�Ĵ���	*/
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)(dr16_rbuff);/*�ڴ��ַ(Ҫ����ı�����ָ��)*/
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;/*�ڴ����ݵ�λ*/
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢������ģʽ
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;        //ѭ��ģʽ
    DMA_InitStructure.DMA_PeripheralBaseAddr = USART1_DR_ARR;//DMA�����ַ  (uint32_t)&(USART1->DR)
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//���������ģʽ/*�����ַ����*/
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;//set the piriority
    DMA_Init(DMA1_Channel5, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel5, ENABLE);

    DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
    USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);//����dma�ж������������ܽ���dma�жϷ�����
    //����dma�ж�
    MY_NVIC_Init(0, 2, DMA1_Channel5_IRQn, 2);//�ж����ȼ���Ϊ
}


void SBUS_USART1_Config(u32 Baudrate) {
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    //����Ҫ�����жϾͲ���Ҫ�����ж�����
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


    USART_InitStructure.USART_BaudRate = Baudrate;                                 //����������
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//Ӳ�����Ʋ�ʹ��
    USART_InitStructure.USART_Mode = USART_Mode_Rx;                                //ֻ����
    USART_InitStructure.USART_Parity = USART_Parity_Even;                          //��żУ��λΪżУ
    USART_InitStructure.USART_StopBits = USART_StopBits_2;                         //���һλΪ��ֹλ,����2λֹͣλ�д���ȶ����ǰ�õ�1λ
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                    //���ݳ���Ϊ8λ
    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE);                              //��������,֡��,����,У����ж�
    USART_Cmd(USART1, ENABLE);


    DMA1_RX1Init();


}


//********************************����ΪSBUS�Ĵ�����****************************//
//DMA�Ľ��ܷ�����
void DMA1_Channel5_IRQHandler(void) {
    if (DMA_GetFlagStatus(DMA1_IT_TC5) == SET)//DMA1 Channel5 transfer complete flag.
    {
//        SBUS_Ch[0] = (dr16_rbuff[0] | (dr16_rbuff[1] << 8)) & 0x07ff;                                // �Һ�
//        SBUS_Ch[1] = ((dr16_rbuff[1] >> 3) | (dr16_rbuff[2] << 5)) & 0x07ff;                         // ����
//        SBUS_Ch[2] = ((dr16_rbuff[2] >> 6) | (dr16_rbuff[3] << 2) | (dr16_rbuff[4] << 10)) & 0x07ff; // ���
//        SBUS_Ch[3] = ((dr16_rbuff[4] >> 1) | (dr16_rbuff[5] << 7)) & 0x07ff;                         // ����
//        SBUS_Ch[4] = ((dr16_rbuff[5] >> 4) & 0x000C) >> 2;                                           // ��S
//        SBUS_Ch[5] = ((dr16_rbuff[5] >> 4) & 0x0003);                                                // ��S
        if (dr16_rbuff[0] == 0x0f) {
            SBUS_Ch[1] = (dr16_rbuff[1] | (dr16_rbuff[2] << 8)) & 0x07ff;                       //ͨ��1���Һᣬ1400~1000~600
            SBUS_Ch[2] =
                ((dr16_rbuff[2] >> 3) | (dr16_rbuff[3] << 5)) & 0x07ff;                       //ͨ��2��������1800~1000~200
            SBUS_Ch[3] =
                ((dr16_rbuff[3] >> 6) | (dr16_rbuff[4] << 2) | (dr16_rbuff[5] << 10)) & 0x07ff; //ͨ��3��������1800~1000~200
            SBUS_Ch[4] =
                ((dr16_rbuff[5] >> 1) | (dr16_rbuff[6] << 7)) & 0x07ff;                       //ͨ��4����ᣬ1800~1000~200
            SBUS_Ch[5] =
                ((dr16_rbuff[6] >> 4) | (dr16_rbuff[7] << 4)) & 0x07ff;                       //ͨ��5����200����1000����1800
            SBUS_Ch[6] =
                ((dr16_rbuff[7] >> 7) | (dr16_rbuff[8] << 1) | (dr16_rbuff[9] << 9)) & 0x07ff;  //ͨ��6����200����1800
            SBUS_Ch[7] = ((dr16_rbuff[9] >> 2) | (dr16_rbuff[10] << 6)) & 0x07ff;
            SBUS_Ch[8] = ((dr16_rbuff[11] << 3) | (dr16_rbuff[10] >> 5)) & 0x07ff;
            SBUS_Ch[9] = ((dr16_rbuff[12]) | (dr16_rbuff[13] << 8)) & 0x07ff;
            SBUS_Ch[10] = ((dr16_rbuff[13] >> 13) | (dr16_rbuff[14] << 5)) & 0x07ff;
        }

        DMA_ClearFlag(DMA1_IT_TC5);
    }
}
