#include "stm32f10x.h"
#include "usart1_2.h"
#include <math.h>

void USTAR1_2_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	NVIC_InitTypeDef   NVIC_InitStruct;
	USART_DeInit(USART2);  //复位串口2
	USART_DeInit(USART1);  //复位串口1
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_USART1,ENABLE);//使能GPIOA和USART1
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);//使能USART2
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   // 输出模式为复用推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9;         // Pin9位串口1的TX端
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // 输出速度为50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   // 输出模式为浮空输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10;         // Pin10位串口1的RX端
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // 输出速度为50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF_PP;   // 输出模式为复用推挽输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;         // Pin2位串口1的TX端
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // 输出速度为50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN_FLOATING;   // 输出模式为浮空输出
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3;         // Pin3位串口2的RX端
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz; // 输出速度为50MHZ
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	
	//Usart2 NVIC 中断配置
	NVIC_InitStruct.NVIC_IRQChannel=USART2_IRQn;  //串口2通道，通道在stm32f10x.h头文件开头的地方找
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;    //通道使能
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2; //设置抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;        //设置响应优先级
	NVIC_Init(&NVIC_InitStruct);
	
	//初始化串口1
	USART_InitStruct.USART_BaudRate=9600;        //波特率为9600
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;   //可输入、输出
	USART_InitStruct.USART_Parity=USART_Parity_No;              //没有校验位
	USART_InitStruct.USART_StopBits=USART_StopBits_1;           //停止位为1 
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;       //传输字节8
	USART_Init(USART1,&USART_InitStruct);                         //初始化串口1
	USART_Cmd(USART1,ENABLE);              //使能串口1
	//初始化串口2
	USART_InitStruct.USART_BaudRate=9600;        //波特率为9600
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;   //可输入、输出
	USART_InitStruct.USART_Parity=USART_Parity_No;              //没有校验位
	USART_InitStruct.USART_StopBits=USART_StopBits_1;           //停止位为1 
	USART_InitStruct.USART_WordLength=USART_WordLength_8b;       //传输字节8
	USART_Init(USART2,&USART_InitStruct);                       //初始化串口2
	
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE); //串口2 中断模式选择和中断使能
	USART_Cmd(USART2,ENABLE);              //使能串口2
	
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

	
		

  
	



