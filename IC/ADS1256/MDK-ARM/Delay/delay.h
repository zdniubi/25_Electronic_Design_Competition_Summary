#ifndef     __delay_H
#define     __delay_H
 
#include "stm32f1xx.h"  // �൱��51��Ƭ���е�  #include <reg51.h>
 
void Delay_Init(void);
void Delay_us(uint32_t nus);
void Delay_ms(uint32_t nms);
#endif

