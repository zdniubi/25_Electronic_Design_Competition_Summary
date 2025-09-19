/*
PA4 SPI_CS
PA5 SPI_CLK
PA6 SPI_MISO
PA7 SPI_DOUT
*/


#ifndef SPI_H
#define SPI_H

#include "main.h"

void SPI_Write_Byte(uint8_t data);
uint8_t SPI_Read_Data(void);
void SPI_Write_Data_len(uint16_t data, uint8_t len);
void SPI_DOUT_H(void);
void SPI_DOUT_L(void);
void SPI_CLK_H(void);
void SPI_CLK_L(void);
void SPI_CS_H(void);
void SPI_CS_L(void);
uint8_t MySPI_R_MISO(void);






#endif