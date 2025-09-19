#include "ad7606.h"

uint16_t ad7606_oncedata[adc_oncequantity];
uint16_t ad7606_data[adc_quantity][adc_length];



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

uint8_t MySPI_R_MISO_A(void) // 读取MISO
{
	return HAL_GPIO_ReadPin(SPI_MISO_A_GPIO_Port, SPI_MISO_A_Pin);
}

uint8_t MySPI_R_MISO_B(void) // 读取MISO
{
	return HAL_GPIO_ReadPin(SPI_MISO_B_GPIO_Port, SPI_MISO_B_Pin);
}

void SPI_Read_Data_16(uint16_t *dataA, uint16_t *dataB) // 讈取16位私
{
	*dataA = 0;
	*dataB = 0;
	for (uint8_t i = 0; i < 16; i++)
	{
		SPI_CLK_L();
		if (MySPI_R_MISO_A() == 1)
		{
			*dataA |= (0x8000 >> i);
		}
		if (MySPI_R_MISO_B() == 1)
		{
			*dataB |= (0x8000 >> i);
		}
		SPI_CLK_H();
	}
}

void AD7606_GPIO_Init(void) // AD7606 GPIO初始化
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, SPI_CLK_Pin | rst_Pin | SPI_CS_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, OS0_Pin | OS1_Pin | OS2_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(CV_GPIO_Port, CV_Pin, GPIO_PIN_SET);

	/*Configure GPIO pins : PEPin PEPin PEPin */
	GPIO_InitStruct.Pin = SPI_CLK_Pin | SPI_CS_Pin | CV_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = rst_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(rst_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PEPin PEPin */
	GPIO_InitStruct.Pin = SPI_MISO_A_Pin | SPI_MISO_B_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PtPin */
	GPIO_InitStruct.Pin = busy_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(busy_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PBPin PBPin PBPin */
	GPIO_InitStruct.Pin = OS0_Pin | OS1_Pin | OS2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void AD7606_SetSpeed(uint8_t mode) // AD7606过采样选择
{
	switch (mode)
	{
	case 0:
	OS0_0 OS1_0 OS2_0 case 2:
	OS0_0 OS1_0 OS2_1 case 4:
	OS0_0 OS1_1 OS2_0 case 8:
	OS0_0 OS1_1 OS2_1 case 16:
	OS0_1 OS1_0 OS2_0 case 32:
	OS0_1 OS1_0 OS2_1 case 64:
		OS0_1 OS1_1 OS2_0 default : OS0_0 OS1_0 OS2_0
	}
}

void AD7606_Reset() // AD7606复位
{
	RST_1
	Delay_Us(1);
	RST_0
}

void AD7606_Init() // AD7606初始化
{
	AD7606_SetSpeed(0);
	AD7606_Reset();
	CV_1 CS_1
}

void AD7606_Start() // ADC启动信号
{
	CV_0 CV_1
}

float process(uint16_t data)
{
	if (data & 0x8000) // 检查符号位
	{
		data = ~(data) + 1;						  // 负数转换为补码
		return -((float)data * 10.0f / 32768.0f); // 转换为电压值
	}
	else
	{
		return (float)data * 10.0f / 32768.0f; // 转换为电压值
	}
}

void AD7606_OnceConvert(float voltage[adc_oncequantity])
{
	AD7606_Init();
	AD7606_Start();
	while (HAL_GPIO_ReadPin(busy_GPIO_Port, busy_Pin))
		; // 等待busy信号低电平
	CS_0 for (uint8_t i = 0; i < adc_oncequantity / 2; i++)
	{
		SPI_Read_Data_16(&ad7606_oncedata[i], &ad7606_oncedata[i + adc_oncequantity / 2]);
	}
	CS_1

		for (uint8_t i = 0; i < adc_oncequantity; i++)
	{
		voltage[i] = process(ad7606_oncedata[i]);
	}
}

void AD7606_Convert(float voltage[adc_quantity][adc_length])
{
	AD7606_Init();
	for (uint8_t j = 0; j < adc_length; j++)
	{
		AD7606_Start();
		while (HAL_GPIO_ReadPin(busy_GPIO_Port, busy_Pin))
			; // 等待busy信号低电平
		CS_0 for (uint8_t i = 0; i < adc_quantity / 2; i++)
		{
			SPI_Read_Data_16(&ad7606_data[i][j], &ad7606_data[i + adc_quantity / 2][j]);
		}
		CS_1
	}

	for (uint8_t j = 0; j < adc_length; j++)
	{
		for (uint8_t i = 0; i < adc_quantity; i++)
		{
			voltage[i][j] = process(ad7606_data[i][j]);
		}
	}
}

float findmax_7606(float *data)
{
	float max = 0;
	for (uint8_t i = 0; i < adc_length; i++)
	{
		if (data[i] > max)
		{
			max = data[i];
		}
	}
	return max;
}

float average_7606(float *data)
{
	float sum = 0;
	for (uint8_t i = 0; i < adc_length; i++)
	{
		sum += data[i];
	}
	return sum / adc_length;
}