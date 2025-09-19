#include "tdc7200.h"

const uint8_t ADDR_CONFIG1 = 0x40;                  // 4 0    R/W     Write
const uint8_t ADDR_CONFIG2 = 0x41;                  // 4 1    R/W     Write
const uint8_t ADDR_INT_STATUS = 0x42;               // 4 2    R/W     Write
const uint8_t ADDR_INT_MASK = 0x43;                 // 4 3    R/W     Write
const uint8_t ADDR_COARSE_CNTR_OVF_H = 0x44;        // 4 4    R/W     Write
const uint8_t ADDR_COARSE_CNTR_OVF_L = 0x45;        // 4 5    R/W     Write
const uint8_t ADDR_CLOCK_CNTR_OVF_H = 0x46;         // 4 6    R/W     Write
const uint8_t ADDR_CLOCK_CNTR_OVF_L = 0x47;         // 4 7    R/W     Write
const uint8_t ADDR_CLOCK_CNTR_STOP_MASK_H = 0x48;   // 4 8    R/W     Write
const uint8_t ADDR_CLOCK_CNTR_STOP_MASK_L = 0x49;   // 4 9    R/W     Write
const uint8_t ADDR_TIME1_Measured = 0x10;           // 0000 0000       Read Only
const uint8_t ADDR_CLOCK_COUNT1 = 0x11;             // 0000 0000       Read Only
const uint8_t ADDR_TIME2_Measured = 0x12;           // 0000 0000       Read Only
const uint8_t ADDR_CLOCK_COUNT2 = 0x13;             // 0000 0000       Read Only
const uint8_t ADDR_TIME3_Measured = 0x14;           // 0000 0000       Read Only
const uint8_t ADDR_CLOCK_COUNT3 = 0x15;             // 0000 0000       Read Only
const uint8_t ADDR_TIME4_Measured = 0x16;           // 0000 0000       Read Only
const uint8_t ADDR_CLOCK_COUNT4 = 0x17;             // 0000 0000       Read Only
const uint8_t ADDR_TIME5_Measured = 0x18;           // 0000 0000       Read Only
const uint8_t ADDR_CLOCK_COUNT5 = 0x19;             // 0000 0000       Read Only
const uint8_t ADDR_TIME6_Measured = 0x1A;           // 0000 0000       Read Only
const uint8_t ADDR_CALIBRATION1 = 0x1B;             // 0000 0000       Read Only
const uint8_t ADDR_CALIBRATION2 = 0x1C;             // 0000 0000       Read Only
const uint8_t DATA_CONFIG1_Start = 0x83;            // 1000 0011
const uint8_t DATA_CONFIG1 = 0x82;                  // 1000 0010
const uint8_t DATA_CONFIG2 = 0x40;                  // 0100 0000
const uint8_t DATA_INT_STATUS = 0x1F;               // 0001 1111
const uint8_t DATA_INT_MASK = 0x01;                 // 0000 0001
const uint8_t DATA_COARSE_CNTR_OVF_H = 0xFF;        // 1111 1111
const uint8_t DATA_COARSE_CNTR_OVF_L = 0x00;        // 0000 0000
const uint8_t DATA_CLOCK_CNTR_OVF_H = 0xFF;         // 1111 1111
const uint8_t DATA_CLOCK_CNTR_OVF_L = 0x00;         // 0000 0000
const uint8_t DATA_CLOCK_CNTR_STOP_MASK_H = 0xFF;   // 1111 1111
const uint8_t DATA_CLOCK_CNTR_STOP_MASK_L = 0x00;   // 0000 0000
const uint8_t DATA_CONFIG1_Down = 0x9A;             // 1001 1010
const uint8_t DATA_CONFIG1_mode1 = 0x80;            // 1000 0000
const uint8_t DATA_CONFIG1_mode1_Down = 0x98;       // 1001 1000
const uint8_t DATA_CONFIG1_Start_Down = 0x9B;       // 1001 1011
const uint8_t DATA_CONFIG1_Start_mode1 = 0x81;      // 1000 0001
const uint8_t DATA_CONFIG1_Start_mode1_Down = 0x99; // 1001 1001

//uint8_t Time1_Measured_Data[3];
//uint8_t Time2_Measured_Data[3];
//uint8_t Clock_Count_1_Data[3];
//uint8_t Calibration_1_Data[3];
//uint8_t Calibration_2_Data[3];
uint32_t TDC_Time1;
uint32_t TDC_Time2;
uint32_t TDC_Clock_Conunt1;
uint32_t TDC_Calibration1;
uint32_t TDC_Calibration2;
uint8_t read_TDC_Data;
uint8_t int_data[6];

/*

  该函数用来对TDC7200进行初始化，配置寄存器

*/
void TDC_Init(void)
{
  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(SPI_CLK_GPIO_Port, SPI_CLK_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SPI_DOUT_GPIO_Port, SPI_DOUT_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(enable_GPIO_Port, enable_Pin, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(enable_GPIO_Port, enable_Pin, GPIO_PIN_SET); 
  HAL_Delay(5);
}


/*
  配置stopmask寄存器  
*/
void TDC7200_preInit_stopmask(float stoptime)
{
  int stopmask =(int) (stoptime / Clock_NS); // 计算stopmask值
  uint8_t stopmask_h = (stopmask >> 8) & 0xFF; // 高字节
  uint8_t stopmask_l = stopmask & 0xFF;        // 低字节
  TDC7200_Write(ADDR_CLOCK_CNTR_STOP_MASK_H, stopmask_h);
  TDC7200_Write(ADDR_CLOCK_CNTR_STOP_MASK_L, stopmask_l);
}

/*

  该函数用mode2计算时间

*/
double complate(void)
{
  float CALIBAR_Dif;
  double calCount;
  double normLSB;
  double Time1_nor;
  double Time2_nor;
  double Clockcount; 
  double TOF1;
  CALIBAR_Dif = (float)(TDC_Calibration2 - TDC_Calibration1);
  calCount = (double)CALIBAR_Dif / CALIBRATION_PERIODS_D1;
  normLSB = (double)(Clock_NS / calCount);
  Time1_nor = (double)(TDC_Time1 * normLSB);
  Time2_nor = (double)(TDC_Time2 * normLSB);
  Clockcount = (double)(TDC_Clock_Conunt1 * Clock_NS);
  TOF1 = (double)(Clockcount + Time1_nor - Time2_nor);
  // TDC_TIE_Data = TOF1 * TIE_NS - TIE_DIF;
  return TOF1;
}

/*

  该函数用mode1计算时间

*/
double complate_mode1(void)
{
  float CALIBAR_Dif;
  double calCount;
  double normLSB;
  double TOF1;
  CALIBAR_Dif = (float)(TDC_Calibration2 - TDC_Calibration1);
  calCount = (double)CALIBAR_Dif / CALIBRATION_PERIODS_D1;
  normLSB = (double)(Clock_NS / calCount);
  TOF1 = (double)(TDC_Time1 * normLSB);

  // Time1_nor = (double)(TDC_Time1 * normLSB);

  // Time2_nor = (double)(TDC_Time2 * normLSB);

  // Clockcount = (double)(TDC_Clock_Conunt1 * Clock_NS);

  // TOF1 = (double)(Clockcount + Time1_nor - Time2_nor);

  // TDC_TIE_Data = TOF1 * TIE_NS - TIE_DIF;
  return TOF1;
}

/*

  

*/
void TDC_INT_Check(void)
{
  int_data[0] = TDC7200_Read_ADDR(0x00);
  int_data[1] = TDC7200_Read_ADDR(0x01);
  int_data[2] = TDC7200_Read_ADDR(0x02);
  int_data[3] = TDC7200_Read_ADDR(0x03);
  int_data[4] = TDC7200_Read_ADDR(0x08);
  int_data[5] = TDC7200_Read_ADDR(0x09);
  printf("STATUS: %02X %02X %02X %02X %02X %02X\r\n", int_data[0], int_data[1], int_data[2], int_data[3], int_data[4], int_data[5]);
}


/*

  读取数据为计算做准备, 并做好下一次测量的准备

*/
void TDC_Write_Readtest(void)
{
  TDC_Time1 = TDC7200_Read(ADDR_TIME1_Measured);
  TDC_Clock_Conunt1 = TDC7200_Read(ADDR_CLOCK_COUNT1);
  TDC_Time2 = TDC7200_Read(ADDR_TIME2_Measured);
  TDC_Calibration1 = TDC7200_Read(ADDR_CALIBRATION1);
  TDC_Calibration2 = TDC7200_Read(ADDR_CALIBRATION2);
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Start);
  read_TDC_Data = TDC7200_Read_ADDR(0x00);
  HAL_Delay(1);
}


/*

以下函数设置了config1与2以及中断掩码寄存器

*/

void TDC7200_Init(void) // mode2,上升沿
{
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1); 
  int_data[0] = TDC7200_Read_ADDR(0x00);
  TDC7200_Write(ADDR_CONFIG2, DATA_CONFIG2);
  int_data[1] = TDC7200_Read_ADDR(0x01);
  TDC7200_Write(ADDR_INT_MASK, DATA_INT_MASK);
  int_data[3] = TDC7200_Read_ADDR(0x03);
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Start);
  int_data[0] = TDC7200_Read_ADDR(0x00);
}

void TDC7200_Init_Down(void) // mode2,下降沿
{
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Down); 
  int_data[0] = TDC7200_Read_ADDR(0x00);
  TDC7200_Write(ADDR_CONFIG2, DATA_CONFIG2);
  int_data[1] = TDC7200_Read_ADDR(0x01);
  TDC7200_Write(ADDR_INT_MASK, DATA_INT_MASK);
  int_data[3] = TDC7200_Read_ADDR(0x03);
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Start_Down);
  int_data[0] = TDC7200_Read_ADDR(0x00);
}

void TDC7200_Init_mode1(void) // mode1,上升沿
{
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_mode1);
  int_data[0] = TDC7200_Read_ADDR(0x00);
  TDC7200_Write(ADDR_CONFIG2, DATA_CONFIG2);
  int_data[1] = TDC7200_Read_ADDR(0x01);
  TDC7200_Write(ADDR_INT_MASK, DATA_INT_MASK);
  int_data[3] = TDC7200_Read_ADDR(0x03);
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Start_mode1);
  int_data[0] = TDC7200_Read_ADDR(0x00);
}

void TDC7200_Init_mode1_Down(void) // mode1,下降沿
{
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_mode1_Down);
  int_data[0] = TDC7200_Read_ADDR(0x00);
  TDC7200_Write(ADDR_CONFIG2, DATA_CONFIG2);
  int_data[1] = TDC7200_Read_ADDR(0x01);
  TDC7200_Write(ADDR_INT_MASK, DATA_INT_MASK);
  int_data[3] = TDC7200_Read_ADDR(0x03);
  TDC7200_Write(ADDR_CONFIG1, DATA_CONFIG1_Start_mode1_Down);
  int_data[0] = TDC7200_Read_ADDR(0x00);
}

void TDC7200_Write(uint8_t addr, uint8_t data) // 写入数据到TDC7200寄存器
{
  SPI_CS_L();
  SPI_Write_Byte(addr);
  SPI_Write_Byte(data);
  SPI_CS_H();
}

uint8_t TDC7200_Read_ADDR(uint8_t addr) // 读取TDC7200寄存器的单字节数据
{
  uint8_t readdata;
  SPI_CS_L();
  SPI_Write_Byte(addr);
  readdata = SPI_Read_Data();
  SPI_CS_H();
  return readdata;
}

uint32_t TDC7200_Read(uint8_t addr) // 读取TDC7200寄存器的三字节数据
{
  uint32_t readdata;
  uint8_t read[3];
  SPI_CS_L();
  SPI_Write_Byte(addr);
  read[0] = SPI_Read_Data();
  read[1] = SPI_Read_Data();
  read[2] = SPI_Read_Data();
  SPI_CS_H();
  readdata = read[0] << 16 | read[1] << 8 | read[2];
  return readdata;
}

// void READ40(uint8_t *XD) // 读取CONFIG2
//
//{
//
//   MySPI_Start();
//
//   SPI_Write_Byte(0x01);
//
//   *XD = SPI_Read_Data();
//
//   MySPI_Stop();
// }
//
// void READ07(uint8_t *My) // 读取INT_MASK
//
//{
//
//   MySPI_Start();
//
//   SPI_Write_Byte(0x03);
//
//   *My = SPI_Read_Data();
//
//   MySPI_Stop();
// }