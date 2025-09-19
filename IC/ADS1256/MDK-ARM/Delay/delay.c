#include  "delay.h"
 
uint8_t fac_us=0;
uint16_t fac_ms=0;
 
void Delay_Init()
{
	//ֻ����ѡ�񲻷�Ƶ����8��Ƶ������ѡ��ϵͳʱ��8��Ƶ�����Ƶ��Ϊ9MHZ
	SysTick->CTRL &= ~(1<<2);
	//SystemCoreClockΪ72000000������fac_usΪ9��Ҳ���Ǽ�¼��9�Ρ���ΪƵ��Ϊ9MHZ����Ϊ1us
	fac_us  = SystemCoreClock  / 8000000;  
	fac_ms  = fac_us*1000;  //1000us=1ms
}
 
/*
	CTRL     SysTick���Ƽ�״̬�Ĵ���
	LOAD     SysTick��װ����ֵ�Ĵ���
	VAL      SysTick��ǰ��ֵ�Ĵ���
*/
void Delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->LOAD  =nus*fac_us;   //���ü��ص�ֵ������1us��Ҫ����9�Ρ�nus����1��CALIB=1*9=9��������1us
	SysTick->VAL   =0x00;         //��ռ������е�ֵ��LOAD���ֵ����д���ͻ���أ�������systickʹ����VALֵΪ0ʱ�ż���
	SysTick->CTRL  |=SysTick_CTRL_ENABLE_Msk;  //ʹ��ʱ�ӣ���ʼ��ʱ
	do
	{
		temp=SysTick->CTRL;   //��ѯ�Ƿ�������
	}while((temp&0x01)&&!(temp&(1<<16)));   //���ж϶�ʱ���Ƿ������У����ж��Ƿ�������
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����	 
}
 
void Delay_ms(uint32_t nms)
{
	uint32_t temp;
	SysTick->LOAD  =nms*fac_ms;   //���ü��ص�ֵ������1us��Ҫ����9�Ρ�nus����1��CALIB=1*9=9��������1us
	SysTick->VAL   =0x00;         //��ռ������е�ֵ��LOAD���ֵ����д���ͻ���أ�������systickʹ����VALֵΪ0ʱ�ż���
	SysTick->CTRL  |=SysTick_CTRL_ENABLE_Msk;  //ʹ��ʱ�ӣ���ʼ��ʱ
	do
	{
		temp=SysTick->CTRL;   //��ѯ�Ƿ�������
	}while((temp&0x01)&&!(temp&(1<<16)));   //���ж϶�ʱ���Ƿ������У����ж��Ƿ�������
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL =0X00;      					 //��ռ�����	 
}
