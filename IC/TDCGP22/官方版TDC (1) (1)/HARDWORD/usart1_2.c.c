#include "stm32f10x.h"
#include "usart1_2.h"
#include <math.h>

void USTAR1_2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStruct;
	USART_DeInit(USART2);  //��λ����2
	USART_DeInit(USART1);  //��λ����1
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);//ʹ��GPIOA��USART1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//ʹ��USART2
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   // ���ģʽΪ�����������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;         // Pin9λ����1��TX��
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // ����ٶ�Ϊ50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   // ���ģʽΪ�������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;         // Pin10λ����1��RX��
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // ����ٶ�Ϊ50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   // ���ģʽΪ�����������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;         // Pin2λ����1��TX��
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // ����ٶ�Ϊ50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   // ���ģʽΪ�������
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3;         // Pin3λ����2��RX��
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // ����ٶ�Ϊ50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	
	//Usart2 NVIC �ж�����
	NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;  //����2ͨ����ͨ����stm32f10x.hͷ�ļ���ͷ�ĵط���
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;    //ͨ��ʹ��
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2; //������ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;        //������Ӧ���ȼ�
	NVIC_Init(&NVIC_InitStruct);
	
	//��ʼ������1
	USART_InitStruct.USART_BaudRate=9600;        //������Ϊ9600
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;   //�����롢���
	USART_InitStruct.USART_Parity=USART_Parity_No;              //û��У��λ
	USART_InitStruct.USART_StopBits=USART_StopBits_1;           //ֹͣλΪ1 
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;       //�����ֽ�8
	USART_Init(USART1,&USART_InitStruct);                         //��ʼ������1
	USART_Cmd(USART1,ENABLE);              //ʹ�ܴ���1
	//��ʼ������2
	USART_InitStruct.USART_BaudRate=9600;        //������Ϊ9600
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;   //�����롢���
	USART_InitStruct.USART_Parity=USART_Parity_No;              //û��У��λ
	USART_InitStruct.USART_StopBits=USART_StopBits_1;           //ֹͣλΪ1 
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;       //�����ֽ�8
	USART_Init(USART2,&USART_InitStruct);                       //��ʼ������2
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE); //����2 �ж�ģʽѡ����ж�ʹ��
	USART_Cmd(USART2,ENABLE);              //ʹ�ܴ���2
	
}

void USART2_IRQHandler(void)
{
  	if(USART_GetITStatus(USART2,USART_IT_RXNE))
		{
			 GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET); // Output HIGH
      //    SetPortLow;
      GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);   // Output LOW
    
		}
		USART_ClearITPendingBit(USART2,USART_IT_RXNE);

	
}

	
		

  
	



