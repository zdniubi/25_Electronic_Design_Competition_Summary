
#ifndef __HMC832_H_
#define __HMC832_H_

#include "stdint.h"
#include "gpio.h"
#include "main.h"

#define REG_0 ((uint8_t)0x00)
#define REG_1 ((uint8_t)0x08)
#define REG_2 ((uint8_t)0x10)
#define REG_3 ((uint8_t)0x18)
#define REG_4 ((uint8_t)0x20)
#define REG_5 ((uint8_t)0x28)
#define REG_6 ((uint8_t)0x30)
#define REG_7 ((uint8_t)0x38)
#define REG_8 ((uint8_t)0x40)
#define REG_9 ((uint8_t)0x48)
#define REG_A ((uint8_t)0x50)
#define REG_F ((uint8_t)0x78)

#define VCO_REG_0 ((uint32_t)0x0000)
#define VCO_REG_2 ((uint32_t)0x0810)//div=16
#define VCO_REG_3 ((uint32_t)0x6998)
#define VCO_REG_7 ((uint32_t)0x4D38)//4D38



#define REG_DATA_1 ((uint32_t)0x2)
#define REG_DATA_2 ((uint32_t)0x1)
#define REG_DATA_3 ((uint32_t)0x21)
#define REG_DATA_4 ((uint32_t)0x99999A)
#define REG_DATA_6 ((uint32_t)0x200B4A)
#define REG_DATA_7 ((uint32_t)0x8CD)
#define REG_DATA_8 ((uint32_t)0xC1BEFF)
#define REG_DATA_9 ((uint32_t)0x3A7EFD)
#define REG_DATA_A ((uint32_t)0x2006)
#define REG_DATA_F ((uint32_t)0xC1)


#define PLL_SCK_SET() 	HAL_GPIO_WritePin(GPIOA,HMC_SCLK_Pin,GPIO_PIN_SET);
#define PLL_SCK_RESET() HAL_GPIO_WritePin(GPIOA,HMC_SCLK_Pin,GPIO_PIN_RESET);

#define PLL_CS_SET() 		HAL_GPIO_WritePin(GPIOA,HMC_CS_Pin,GPIO_PIN_SET);
#define PLL_CS_RESET() 	HAL_GPIO_WritePin(GPIOA,HMC_CS_Pin,GPIO_PIN_RESET);

#define PLL_SDI_SET() 	HAL_GPIO_WritePin(GPIOA,HMC_SDO_Pin,GPIO_PIN_SET);
#define PLL_SDI_RESET() HAL_GPIO_WritePin(GPIOA,HMC_SDO_Pin,GPIO_PIN_RESET);


void Wirte_Data(uint8_t ADDR,uint32_t Data);
void HMC832_Write_freq(double freq);
void delay_us (int length);
void HMC832_io_init();
void HMC832_init();
void Find_DIV(double freq);
#endif
