#include "stm32f10x.h"
#include "delay.h"
#include "led.h"
#include "SPI.h"
#include "tdc.h"
#include "usart1_2.h"
#include "key.h"

 void Delay(u32 count)
 {
   u32 i=0;

   for(;i<count;i++);
 }
 int main(void)
 {		
	  	vu8 key=0;	

	 int flag=1;
   void* Bus_Type = SPI2;

	 NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
   USTAR1_2_Init();
	 SPI2_Init();
	 SPI2_SetSpeed(SPI_BaudRatePrescaler_4); //spi时钟为9Mhz（最大为10Mhz）	 

	 delay_init();
	 LED_Init();
	 KEY_Init();          //³õÊ¼»¯Óë°´¼üÁ¬½ÓµÄÓ²¼þ½Ó¿Ú

	 flag=Test_Communication();
   if(flag==0) Tdc_Init();	 
	 //GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); 
	 //USART_SendData(USART1,'T') ;
	 //while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET){}//??????

	 if(flag==1) gp22_analyse_error_bit(Bus_Type);
	 while(flag==0)
	 { 		//GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET); // Ƭѡtdc

		 key=KEY_Scan(0);	//µÃµ½¼üÖµ
	   	if(key)
		{						   
			switch(key)
			{				 
				case WKUP_PRES:	//¿ØÖÆ·äÃùÆ÷
					break;
				case KEY2_PRES:	//¿ØÖÆLED0·­×ª
					break;
				case KEY1_PRES:	//¿ØÖÆLED1·­×ª	 
					break;
				case KEY0_PRES:	//Í¬Ê±¿ØÖÆLED0,LED1·­×ª 
						 Get_Resule();	
			  	break;
			}
		}else delay_ms(10); 
	}	 
		 
	 
 }
