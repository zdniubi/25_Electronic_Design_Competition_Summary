#ifndef __TDC_H
#define __TDC_H
#include "stm32f10x.h"
#include "stdio.h"

	# define  ON  1
	# define  OFF 0 
	# define TDC_SSN(a)	if (a)	\
						GPIO_SetBits(GPIOB,GPIO_Pin_12);\
						else		\
						GPIO_ResetBits(GPIOB,GPIO_Pin_12)

	# define TDC_SCK(a)	if (a)	\
						GPIO_SetBits(GPIOB,GPIO_Pin_13);\
						else		\
						GPIO_ResetBits(GPIOB,GPIO_Pin_13)

	# define TDC_MISO_READ	GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)
	
	# define TDC_MOSI(a)	if (a)	\
						GPIO_SetBits(GPIOB,GPIO_Pin_15);\
						else		\
						GPIO_ResetBits(GPIOB,GPIO_Pin_15)
int Test_Communication(void);    //≤‚ ‘Õ®–≈ 

/* Device functions ----------------------------------------------------------*/
void      gp22_send_1byte(void *bus_type, uint8_t gp22_opcode_byte);
void      gp22_wr_config_reg(void *bus_type, uint8_t opcode_address,
                             uint32_t config_reg_data);
float     gp22_read_n_bytes(void *bus_type, uint8_t n, uint8_t read_opcode,
                            uint8_t read_addr, uint8_t fractional_bits);
uint16_t  gp22_read_status_bytes(void *bus_type);
uint8_t   gp22_status_count_error(void *bus_type);
void      gp22_analyse_error_bit(void *bus_type);
void Tdc_Init(void);
void Get_Resule(void);


/* Private functions ---------------------------------------------------------*/
void      Dly100us(void *arg);
void      Dly250ns(void *arg);
void      Dly1ms(void *arg);
void      Simple_delay_750ns(void *arg);


#endif


