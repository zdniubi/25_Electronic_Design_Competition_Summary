#include "SPI.h"

void SPI_Write_Byte(uint8_t data) // 发??8位数??
{
    uint8_t i = 0;
    for (i = 0; i < 8; i++)
    {
        if (data & 0x80)
        {
            SPI_DOUT_H();
        }
        else
        {
            SPI_DOUT_L();
        }
        SPI_CLK_H();
        SPI_CLK_L();
        data <<= 1;
    }
}

uint8_t SPI_Read_Data(void) // 读取8位数??
{
    uint8_t data = 0;
    uint8_t i = 0;
    for (i = 0; i < 8; i++)
    {
        SPI_CLK_L();
        if (MySPI_R_MISO() == 1)
        {
            data |= (0x80 >> i);
        }
        SPI_CLK_H();
    }
    return data;
}

void SPI_Write_Data_len(uint16_t data, uint8_t len) // 发??16位长的数??
{
    for (uint8_t i = 0; i < len; i++)
    {
        if (data & 0x8000)
        {
            SPI_DOUT_H();
        }
        else
        {
            SPI_DOUT_L();
        }
        SPI_CLK_H();
        SPI_CLK_L();
        data <<= 1;
    }
}

void SPI_DOUT_H(void) // DOUT高电平
{
    HAL_GPIO_WritePin(SPI_DOUT_GPIO_Port, SPI_DOUT_Pin, GPIO_PIN_SET);
}

void SPI_DOUT_L(void) // DOUT低电平
{
    HAL_GPIO_WritePin(SPI_DOUT_GPIO_Port, SPI_DOUT_Pin, GPIO_PIN_RESET);
}

void SPI_CLK_H(void) // CLK高电平
{
    HAL_GPIO_WritePin(SPI_CLK_GPIO_Port, SPI_CLK_Pin, GPIO_PIN_SET);
}

void SPI_CLK_L(void) // CLK低电平
{
    HAL_GPIO_WritePin(SPI_CLK_GPIO_Port, SPI_CLK_Pin, GPIO_PIN_RESET);
}

void SPI_CS_H(void) // CS高电平
{
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

void SPI_CS_L(void) // CS低电平
{
    HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
}

uint8_t MySPI_R_MISO(void) // 读取MISO
{
    return HAL_GPIO_ReadPin(SPI_MISO_GPIO_Port, SPI_MISO_Pin);
}



