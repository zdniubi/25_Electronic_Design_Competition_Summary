/*************************************************************************************
  Title    :   Analog Devices AD9833 DDS Wave Generator Library for STM32 Using HAL Libraries
  Author:    Bardia Alikhan Afshar <bardia.a.afshar@gmail.com>
  Software:  IAR Embedded Workbench for ARM
  Hardware:  Any STM32 device
 *************************************************************************************/
#include "ad9833.h"

/*
  VCC   ---     3.3V
  SDATA ---     PA7
  SCLK  ---     PA5
  FSYNC1---     PA4
  FSYNC2---     PA3

*/
// ------------------- Variables ----------------
uint16_t FRQLW = 0;    // MSB of Frequency Tuning Word
uint16_t FRQHW = 0;    // LSB of Frequency Tuning Word
uint32_t phaseVal = 0; // Phase Tuning Value
uint8_t WKNOWN1 = 0;    // Flag Variable
uint8_t WKNOWN2 = 0;    // Flag Variable
// -------------------------------- Functions --------------------------------

// ----------------- Software SPI Function
void writeSPI(uint16_t word)
{
  for (uint8_t i = 0; i < 16; i++)
  {
    if (word & 0x8000)
      HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET); // bit=1, Set High
    else
      HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_RESET); // bit=0, Set Low
    ASM_NOP();
    HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_RESET); // Data is valid on falling edge
    ASM_NOP();
    HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
    word = word << 1; // Shift left by 1 bit
  }
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_RESET); // Idle low
  ASM_NOP();
}

// ------------------------------------------------ Sets Output Wave Type
void AD9833_SetWave_1(uint8_t Wave)
{
  switch (Wave)
  {
  case 0:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET);
    writeSPI(0x2000); // Value for Sinusoidal Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
    WKNOWN1 = 0;
    break;
  case 1:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET);
    writeSPI(0x2028); // Value for Square Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
    WKNOWN1 = 1;
    break;
  case 2:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET);
    writeSPI(0x2002); // Value for Triangle Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
    WKNOWN1 = 2;
    break;
  default:
    break;
  }
}

void AD9833_SetWave_2(uint8_t Wave)
{
  switch (Wave)
  {
  case 0:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET);
    writeSPI(0x2000); // Value for Sinusoidal Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
    WKNOWN2 = 0;
    break;
  case 1:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET);
    writeSPI(0x2028); // Value for Square Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
    WKNOWN2 = 1;
    break;
  case 2:
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET);
    writeSPI(0x2002); // Value for Triangle Wave
    HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
    WKNOWN2 = 2;
    break;
  default:
    break;
  }
}

// ------------------------------------------------ Sets Wave Frequency & Phase (In Degree) In PHASE0 & FREQ0 Registers
void AD9833_SetWaveData_1(float Frequency, float Phase)
{
  ASM_NOP();
  // ---------- Tuning Word for Phase ( 0 - 360 Degree )
  if (Phase < 0)
    Phase = 0; // Changing Phase Value to Positive
  if (Phase > 360)
    Phase = 360;                                     // Maximum value For Phase (In Degree)
  phaseVal = ((int)(Phase * (4096 / 360))) | 0XC000; // 4096/360 = 11.37 change per Degree for Register And using 0xC000 which is Phase 0 Register Address

  // ---------- Tuning word for Frequency
  long freq = 0;
  freq = (int)(((Frequency * pow(2, 28)) / FMCLK) + 1); // Tuning Word
  FRQHW = (int)((freq & 0xFFFC000) >> 14);              // FREQ MSB
  FRQLW = (int)(freq & 0x3FFF);                         // FREQ LSB
  FRQLW |= 0x4000;
  FRQHW |= 0x4000;
  // ------------------------------------------------ Writing DATA
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  writeSPI(0x2100); // enable 16bit words and set reset bit
  writeSPI(FRQLW);
  writeSPI(FRQHW);
  writeSPI(phaseVal);
  writeSPI(0x2000); // clear reset bit
  ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET); // high = deselected
  AD9833_SetWave_1(WKNOWN1);
  ASM_NOP();
  return;
}

void AD9833_SetWaveData_2(float Frequency, float Phase)
{
  ASM_NOP();
  // ---------- Tuning Word for Phase ( 0 - 360 Degree )
  if (Phase < 0)
    Phase = 0; // Changing Phase Value to Positive
  if (Phase > 360)
    Phase = 360;                                     // Maximum value For Phase (In Degree)
  phaseVal = ((int)(Phase * (4096 / 360))) | 0XC000; // 4096/360 = 11.37 change per Degree for Register And using 0xC000 which is Phase 0 Register Address

  // ---------- Tuning word for Frequency
  long freq = 0;
  freq = (int)(((Frequency * pow(2, 28)) / FMCLK) + 1); // Tuning Word
  FRQHW = (int)((freq & 0xFFFC000) >> 14);              // FREQ MSB
  FRQLW = (int)(freq & 0x3FFF);                         // FREQ LSB
  FRQLW |= 0x4000;
  FRQHW |= 0x4000;
  // ------------------------------------------------ Writing DATA
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  writeSPI(0x2100); // enable 16bit words and set reset bit
  writeSPI(FRQLW);
  writeSPI(FRQHW);
  writeSPI(phaseVal);
  writeSPI(0x2000); // clear reset bit
  ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET); // high = deselected
  AD9833_SetWave_2(WKNOWN2);
  ASM_NOP();
  return;
}

// ------------------------------------------------ Initializing AD9833
void AD9833_Init(bool select,uint8_t WaveType, float FRQ, float Phase)
{
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET); // Set All SPI pings to High
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);  // Set All SPI pings to High
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);   // Set All SPI pings to High
  if(!select)
  {
    AD9833_SetWave_1(WaveType);                                // Type Of Wave
    AD9833_SetWaveData_1(FRQ, Phase);                          // Frequency & Phase Set
  }
  else
  {
    AD9833_SetWave_2(WaveType);                                // Type Of Wave
    AD9833_SetWaveData_2(FRQ, Phase);                          // Frequency & Phase Set
  }
  return;
}

void AD9833_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_SET);

  /*Configure GPIO pins : PA7 PA3 PA4 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void AD9833_SetRegister_1(uint8_t WaveType,float Frequency,float Phase)
{
  ASM_NOP();
  // ---------- Tuning Word for Phase ( 0 - 360 Degree )
  if (Phase < 0)
    Phase = 0; // Changing Phase Value to Positive
  if (Phase > 360)
    Phase = 360;                                     // Maximum value For Phase (In Degree)
  phaseVal = ((int)(Phase * (4096 / 360))) | 0XC000;

  long freq = 0;
  freq = (int)(((Frequency * pow(2, 28)) / FMCLK) + 1); // Tuning Word
  FRQHW = (int)((freq & 0xFFFC000) >> 14);              // FREQ MSB
  FRQLW = (int)(freq & 0x3FFF);                         // FREQ LSB
  FRQLW |= 0x4000;
  FRQHW |= 0x4000;
  // ------------------------------------------------ Writing DATA
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  switch (WaveType)
  {
    case 0:
    writeSPI(0x2000); // enable 16bit words and set reset bit
    break;
    case 1:
    writeSPI(0x2028); // enable 16bit words and set reset bit
    break;
    case 2:
    writeSPI(0x2002); // enable 16bit words and set reset bit
    break;
  }
  writeSPI(FRQLW);
  writeSPI(FRQHW);
  writeSPI(phaseVal);
  ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET); // high = deselected
  ASM_NOP();
  return;
}

void AD9833_SetRegister_2(uint8_t WaveType,float Frequency,float Phase)
{
  ASM_NOP();
  // ---------- Tuning Word for Phase ( 0 - 360 Degree )
  if (Phase < 0)
    Phase = 0; // Changing Phase Value to Positive
  if (Phase > 360)
    Phase = 360;                                     // Maximum value For Phase (In Degree)
  phaseVal = ((int)(Phase * (4096 / 360))) | 0XC000;

  long freq = 0;
  freq = (int)(((Frequency * pow(2, 28)) / FMCLK) + 1); // Tuning Word
  FRQHW = (int)((freq & 0xFFFC000) >> 14);              // FREQ MSB
  FRQLW = (int)(freq & 0x3FFF);                         // FREQ LSB
  FRQLW |= 0x4000;
  FRQHW |= 0x4000;
  // ------------------------------------------------ Writing DATA
  HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  switch (WaveType)
  {
    case 0:
    writeSPI(0x2000); // enable 16bit words and set reset bit
    break;
    case 1:
    writeSPI(0x2028); // enable 16bit words and set reset bit
    break;
    case 2:
    writeSPI(0x2002); // enable 16bit words and set reset bit
    break;
  }
  writeSPI(FRQLW);
  writeSPI(FRQHW);
  writeSPI(phaseVal);
  ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET); // high = deselected
  ASM_NOP();
  return;
}

void AD9833_Reset_1(void)
{
	HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  writeSPI(0x2100);
	ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS1, GPIO_PIN_SET); // high = deselected
	return;
}

void AD9833_Reset_2(void)
{
	HAL_GPIO_WritePin(AD9833PORT, AD9833DATA, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SCK, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET);
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_RESET); // low = selected
  ASM_NOP();
  writeSPI(0x2100);
	ASM_NOP();
  HAL_GPIO_WritePin(AD9833PORT, AD9833SS2, GPIO_PIN_SET); // high = deselected
	return;
}
