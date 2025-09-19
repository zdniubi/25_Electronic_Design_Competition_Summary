#include "tdc.h"
#include "sys.h"
#include "stm32f10x.h"
#include "math.h"
#include "SPI.h"
#include "delay.h"
#include "timer.h"
/* Private variables ---------------------------------------------------------*/
#define LOOP_DLY_100US    2000
#define LOOP_DLY_250ns    2
#define LOOP_DLY_1ms      3299

u32       CriticalSecCntr;
u8      configured_true=0;

float     CLKHS_freq = 1.000;           // Clock frequency in MHz  时钟频率
float     CLKHS_freq_cal = 1.000;       // Calibrated Clock frequency in MHz  校准时钟频率
float     CLKHS_freq_corr_fact = 1.000; // Correction factor for Clock frequency  对于时钟频率修正系数

float     avg_diff_Cal2_Cal1 = 0;
float     diff_Cal2_Cal1_old = 0;
float     diff_Cal2_Cal1_new = 0;

float     average_RAW_Result = 0;
float     RAW_Result = 0;
float     Time_Result = 0;
float     Distance_Result = 0;

int       N_Measure_Cycles;

uint8_t   Error_Bit = 0;

/* PT1000 --------------------------------------------------------------------*/
//R0, A, B are parameters as specified in EN60 751   RO,A,B是参数为EN 6O751规定
float     R0_at_0C = 1000;            // R0 is the RTD resistance at 0 癈  R0是在0度时的RTD电阻
float     Coeff_A  = 3.9083/1000;     // A = 3,9083 x 10-3 癈-1
float     Coeff_B  = -5.775/10000000; // B = -5,775 x 10-7 癈-1
float     R1 = 0, R2 = 0;             // R1, R2 is resistance measured in Ohm   R1，R2是电阻测量的欧闹捣
float     corr_fact = 1.0000;   // Corr.-factor for temperature resistance ratio   温度电阻比因素 Corr

/* Opcodes  操作码-------------------------------------------------------------------*/
uint8_t   Init =                0x70;
uint8_t   Power_On_Reset =      0x50;
uint8_t   Start_TOF =           0x01;
uint8_t   Start_Temp =          0x02;
uint8_t   Start_Cal_Resonator = 0x03;   // Resonator 谐振器
uint8_t   Start_Cal_TDC =       0x04;
uint8_t   Start_TOF_Restart =   0x05;
uint8_t   Start_Temp_Restart =  0x06;

uint32_t  *sram_memory     = ((uint32_t *)(SRAM_BASE + 0xB00));
uint32_t  sram_mem_offset  = 0x0;

uint32_t  Dummy_var = 0;

// For mathematical calculations   数学计算
int       i;
int       j;

/*******************************************************************************
 * Function Name: Dly100us, Dly250ns, Dly1ms, Simple_delay_750ns
 * Parameters: delay multiplier
 *
 * Return: none
 *
 * Description: Delay Dly * (100us, 250ns, 1ms, 750ns)
 *
 ******************************************************************************/
void Dly100us(void *arg)                        // Gives 100us delay with arg 1
{ int       i;

u32 Dely = (u32)arg;
 while(Dely--)
 {
   for(i=LOOP_DLY_100US; i; i--);
 }
}

void Dly250ns(void *arg)                        // Gives 250ns delay with arg 1
{int       i;

u32 Dely = (u32)arg;
 while(Dely--)
 {
   for( i = LOOP_DLY_250ns; i; i--);
 }
}

void Dly1ms(void *arg)                            // Gives 1ms delay with arg 1
{int       i;

  u32 Dely = (u32)arg;
 while(Dely--)
 {
   for( i = LOOP_DLY_1ms; i; i--);
 }
}

void Simple_delay_750ns(void *arg)             // Gives 750ns delay, with arg 1 
{ int       i;

   u32 Dely = (u32)arg;
   for( i = Dely; i; i--);

}
/*******************************************************************************
 * Function Name: gp22_send_1byte
 * Parameters: Opcode byte   
 *
 * Return: none
 *
 * Description: Writes the Opcode to GP22 写入操作码
 *
 ******************************************************************************/
void gp22_send_1byte (void *bus_type, uint8_t gp22_opcode_byte)
{
     // Deactivating Reset SPIx 停用复位的SPI
     GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, gp22_opcode_byte);     // OPCODE TO Device  设备操作码
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     Simple_delay_750ns((void*)10); // important delay (16) at SPI freq.=750kHz 
     // Reset to device SPIx 重置到设备的SPI
     GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
}
/*******************************************************************************
 * Function Name: gp22_wr_config_reg
 * Parameters: Address byte, 4 bytes of Configuration   参数：地址字节，4个字节的配置
 *
 * Return: none
 *
 * Description: Writes the config.reg. specified in GP22 with the data 
 *
 ******************************************************************************/
void gp22_wr_config_reg (void *bus_type, uint8_t opcode_address,
                         uint32_t config_reg_data)
{
   uint8_t Data_Byte_Lo    = config_reg_data;
   uint8_t Data_Byte_Mid1  = config_reg_data>>8;
   uint8_t Data_Byte_Mid2  = config_reg_data>>16;
   uint8_t Data_Byte_Hi    = config_reg_data>>24;

   #define common_delay    10 // important delay (16) at SPI freq.=750kHz
   
      // Deactivating Reset SPIx 停用复位SPI
      GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
      
      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
      SPI_I2S_SendData(bus_type, opcode_address);  // RAM WR OPCODE+ADDRESS 
  Simple_delay_750ns((void*)common_delay);

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
      SPI_I2S_SendData(bus_type, Data_Byte_Hi);  // DATA BYTE HIGH 
  Simple_delay_750ns((void*)common_delay);

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
       SPI_I2S_SendData(bus_type, Data_Byte_Mid2);  // DATA MID - 2 
  Simple_delay_750ns((void*)common_delay);

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
       SPI_I2S_SendData(bus_type, Data_Byte_Mid1);  // DATA MID - 1
  Simple_delay_750ns((void*)common_delay);

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
       SPI_I2S_SendData(bus_type, Data_Byte_Lo);  // DATA LOW
  Simple_delay_750ns((void*)common_delay);

      while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {}
  Simple_delay_750ns((void*)common_delay);
      
     // Reset to device SPIx
     GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
}
/*******************************************************************************
 * Function Name: gp22_read_n_bytes
 * Parameters: bus_type = (SPI1, SPI2)
 *             n_bytes = how many bytes should be read 多少字节应读
 *             read_opcode = read opcode of the device 读取设备的操作码
 *             read_addr = read address of the device  设备的读地址
 *             fractional_bits = number of fractional bits of read data 读取数据的小数位数
 *
 * Return: n bytes from the specified read address  返回：n个字节从指定的读取地址
 *
 * Description: Reads n bytes from an address in GP22 从GP22地址 读取n个字节
 *
 ******************************************************************************/              
float gp22_read_n_bytes(void *bus_type, uint8_t n_bytes, uint8_t read_opcode,
                   uint8_t read_addr, uint8_t fractional_bits)
{
  uint32_t    Result_read = 0;
  float       Result = 0;
	int n;
  uint8_t     read_opcode_addr = read_opcode | read_addr;

  //.............. Result = n Byte = n x 8 bits......................
       // Deactivating Reset SPIx
       GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);      
       SPI_I2S_SendData(bus_type, read_opcode_addr);  // READ OPCODE + Address
       
       while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_750ns((void*)10); // important delay (16) at SPI freq.=750kHz
       
       //Compulsory reads to DR and SR to clear OVR, 强制读取DR和SR去清除OVR
			 //so that next incoming data is saved  以便下次输入数据被保存
       SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
       SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

         //Reading byte1 读取字节1
         SPI_I2S_SendData(bus_type, 0x00F1);  // DUMMY WRITE 虚写
         // Wait until RX buffer is not empty, then read the received data 等待接收缓冲区不为空，然后读取接收到的数据
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
         Result_read = SPI_I2S_ReceiveData(bus_type); //  Read

       for ( n = 1; n < n_bytes; n++)
       {       
         //Reading byte2 .. byte.n  
         SPI_I2S_SendData(bus_type, 0x00F1);  // DUMMY WRITE
         // Wait until RX buffer is not empty, then read the received data
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}

         Result_read = Result_read<<8;
         Result_read |= SPI_I2S_ReceiveData(bus_type); //  Read
       }

       // Reset to device SPIx
       GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
 
  Result = Result_read / pow(2, fractional_bits);

  return Result;
}        
/*******************************************************************************
 * Function Name: gp22_read_status_bytes
 * Parameters: bus_type = (SPI1, SPI2)
 *
 * Return: 2 bytes from the status register, address 0x04 返回：在状态寄存器的2个字节，地址0X04
 *
 * Description: Reads 2 bytes from an address in GP22  从GP22地址读取2个字节
 *
 ******************************************************************************/              
uint16_t gp22_read_status_bytes(void *bus_type)
{
  uint16_t    Result_read = 0;
  uint8_t     n_bytes = 2;
  int n;
  uint8_t     read_opcode_addr = 0xB0 | 0x04;

  //.............. Result = 2 Byte = 16 bits......................

       // Deactivating Reset SPIxGPIO_Pin_12
       GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);      
       SPI_I2S_SendData(bus_type, read_opcode_addr);  // READ OPCODE + Address
       
       while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==RESET) {};
  Simple_delay_750ns((void*)10); // important delay (16) at SPI freq.=750kHz
       
       //Compulsory reads to DR and SR to clear OVR,
       //so that next incoming data is saved
       SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
       SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

         //Reading byte1
         SPI_I2S_SendData(bus_type, 0x00FF);  // DUMMY WRITE
         // Wait until RX buffer is not empty, then read the received data
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}
         Result_read = SPI_I2S_ReceiveData(bus_type); //  Read

       for ( n = 1; n < n_bytes; n++)
       {       
         //Reading byte2 .. byte.n
         SPI_I2S_SendData(bus_type, 0x00FF);  // DUMMY WRITE
         // Wait until RX buffer is not empty, then read the received data
         while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_RXNE)==0) {}

         Result_read = Result_read<<8;
         Result_read |= SPI_I2S_ReceiveData(bus_type); //  Read
       }

       // Reset to device SPIx
       GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
  
  return Result_read;
}                 

/*******************************************************************************
 * Function Name: gp22_status_count_error
 * Parameters: bus_type = (SPI1, SPI2)
 *
 * Return: only one Error Bit (9..15) from the status register, address 0x04  在状态寄存器的一个错误位 地址，0X04
 *
 * Description: Reads 2 bytes from an address in GP22 从GP22地址读取2个字节
 ******************************************************************************/              
uint8_t gp22_status_count_error(void *bus_type)
{
  uint16_t STAT_REG = 0x0000;
  uint8_t  count_error     = 0;

  STAT_REG = gp22_read_status_bytes(bus_type);
  
  if ((STAT_REG&0x0200)==0x0200) count_error++; //Bit9: Timeout_TDC 超时TDC
  if ((STAT_REG&0x0400)==0x0400) count_error++; //Bit10: Timeout_Precounter 超时Precounter
  if ((STAT_REG&0x0800)==0x0800) count_error++; //Bit11: Error_open
  if ((STAT_REG&0x1000)==0x1000) count_error++; //Bit12: Error_short
  if ((STAT_REG&0x2000)==0x2000) count_error++; //Bit13: EEPROM_eq_CREG
  if ((STAT_REG&0x4000)==0x4000) count_error++; //Bit14: EEPROM_DED
  if ((STAT_REG&0x8000)==0x8000) count_error++; //Bit15: EEPROM_Error
         
  return count_error;
}

/*******************************************************************************
 * Function Name: gp22_analyse_error_bit
 * Parameters: none
 *
 * Return: none
 *
 * Description: Analyse the bit from the status register  分析在状态寄存器的位
 ******************************************************************************/              
void gp22_analyse_error_bit(void *bus_type)
{
  uint16_t STAT_REG = 0x0000;

  STAT_REG = gp22_read_status_bytes(bus_type);
  
  //Bit9: Timeout_TDC
  if ((STAT_REG&0x0200)==0x0200) 
    printf("\n-Indicates an overflow of the TDC unit,表示TDC单元溢出");
  //Bit10: Timeout_Precounter
  if ((STAT_REG&0x0400)==0x0400) 
    printf("\n-Indicates an overflow of the 14 bit precounter in MR 2，表示14位PRECOUNTER的MR2溢出");
  //Bit11: Error_open
  if ((STAT_REG&0x0800)==0x0800) 
    printf("\n-Indicates an open sensor at temperature measurement，正显示测量温度的传感器开路");
  //Bit12: Error_short
  if ((STAT_REG&0x1000)==0x1000) 
    printf("\n-Indicates a shorted sensor at temperature measurement");
  //Bit13: EEPROM_eq_CREG
  if ((STAT_REG&0x2000)==0x2000) 
    printf("\n-Indicates whether the content of the configuration registers equals the EEPROM");
  //Bit14: EEPROM_DED
  if ((STAT_REG&0x4000)==0x4000) 
    printf("\n-Double error detection. A multiple error has been detected whcich can not be corrected.");
  //Bit15: EEPROM_Error
  if ((STAT_REG&0x8000)==0x8000) 
    printf("\n-Single error in EEPROM which has been corrected");
}
void Tdc_Init(void)
{
	    void* Bus_Type = SPI2;
            
      gp22_send_1byte(Bus_Type, Power_On_Reset);
      Dly100us((void*)5);              // 500 us wait for GP22
 
      // Writing to the configuration registers (CR)  写配置寄存器
      // CR0: DIV_CLKHS = 2, START_CLKHS = 1, CALIBRATE = 0, NO_CAL_AUTO = 1, MESSB2 = 0, NEG_STOP = NEGSTART = 0, ...
      gp22_wr_config_reg(Bus_Type, 0x80, 0x00241000);  
      // CR1: EN_FAST_INIT = 1, HITIN2 = 1, HITIN1 = 1, ...
      gp22_wr_config_reg(Bus_Type, 0x81, 0x01C90000); 
      // CR2: EN_INT = b111, RFEDGE1 = RFEDGE2 = 0, ... 
      gp22_wr_config_reg(Bus_Type, 0x82, 0xE0000000); 
      // CR3: ... 
      gp22_wr_config_reg(Bus_Type, 0x83, 0x00000000);
      // CR4: ...
      gp22_wr_config_reg(Bus_Type, 0x84, 0x20000000); 
      // CR5: CON_FIRE = b000, EN_STARTNOISE = 1, ...
      gp22_wr_config_reg(Bus_Type, 0x85, 0x10000000); 
      // CR6: QUAD_RES = 0, ...
      gp22_wr_config_reg(Bus_Type, 0x86, 0x00001000); 
    
	
}
 void Get_Resule(void)
 {
	 // Choose your Slot (SPI1, SPI2)
  void* Bus_Type = SPI2;
  u8 Test1; 
  /* controlled loop */
  while (Dummy_var!=11) // To control the loop, e.g. (Dummy_var!=7) 
  {
   // if (Dummy_var==10) Dummy_var=0; // Infinite loop
    


    // .........................................................................
    // ........................Calibrate High Speed Clock....................... 校准高速时钟
    // ...................Laser Rangefinder Measurement CYCLE................... 激光测距仪测量周期
    // .........................Caluculate Result Values........................ 计算结果值
    
   // N_Measure_Cycles = 5000;
    
    diff_Cal2_Cal1_old = diff_Cal2_Cal1_new;

if((Dummy_var==0) | (Dummy_var==10))
{
    //--------------------------------------------------------------------------
    // Start Calibrate High Speed Clock Cycle 开始校准高速时钟周期
    gp22_send_1byte(Bus_Type, Init);
    gp22_send_1byte(Bus_Type, Start_Cal_Resonator);

    // Wait for INT Slot_x  等待中断信号
	 //  Test1=gp22_read_n_bytes(Bus_Type,1,0xB0,0x05,0);
  //   if(Test1==0x19)     GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);   // Output LOW
    while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 

    //Calculate Correction factor  计算修正系数
    //The time interval to be measured is set by ANZ_PER_CALRES  要被测量的时间间隔由ANZ_PER_CALRES设置
    //which defines the number of periods of the 32.768 kHz clock: 它定义了32.768KHZ时钟的周期数
    //2 periods = 61.03515625 祍            2个周期=61.03515625微妙
    CLKHS_freq_corr_fact = 61.03515625/
      gp22_read_n_bytes(Bus_Type, 4, 0xB0, 0x00, 16) * CLKHS_freq;
   
    printf("\n Correction factor for clock = %1.4f\n", CLKHS_freq_corr_fact);   //时钟校正系数=
       
    CLKHS_freq_cal = CLKHS_freq * CLKHS_freq_corr_fact; // Calibrated Clock frequency  校准时钟频率
}

    //--------------------------------------------------------------------------
    // Start Seperate Calibration Measurement Cycle      启动单独校准测量周期
    gp22_send_1byte(Bus_Type, Init);
    
    gp22_send_1byte(Bus_Type, Start_Cal_TDC); // update calibration data 更新校准数据

    // Note: 注意
    // The calibration data are not addressed directly after the calibration  校准数据不在校准后处理
    // measurement but after the next regular measurement, before the next INIT. 测量，但接下来的常规测量后，先下一 INIT
    //在下一个常规测量之后，在下一个INIT之前测量
    // Wait for INT Slot_x   等待
     while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
    gp22_wr_config_reg(Bus_Type, 0x81, 0x19490000);

    //--------------------------------------------------------------------------
    // 1st ToF Measurement plus calibratio data readout 第一次飞行时间测量加校验数据读出
    gp22_send_1byte(Bus_Type, Init);

    //Trigger pulse laser 触发脉冲激光
    //    SetPortHigh;   
		//USART_SendData(USART2,'A');
		//while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET){}//??????
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_SET); // Output HIGH
    //    SetPortLow;
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_RESET);   // Output LOW
   delay_us(1);  //计算1US的时间差
	 
   GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_SET); // Output HIGH
   GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_RESET);   // Output LOW
    //TIM3_Int_Init(79,0);
    // Wait for INT Slot_x 等待中断
    while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
//GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_RESET);   // Output LOW


    // First regular measurement (to readout calibration data) 首先定时测量（以读出校准数据）
    RAW_Result = gp22_read_n_bytes(Bus_Type,4,0xB0,0x00,16);
//    printf("\n 1. Measured RAW Value = %.0f \n",RAW_Result); // RAW value  测量原始值

    // Check Status Register, next free result register   检查状态寄存器，下一个空闲结果寄存器
//    printf("Stat_Reg = 0x%04X \n",gp22_read_status_bytes(Bus_Type)); 

    // readout the new calibration data from result register adr 0x01  从结果寄存器地址为0X01读出新的校准数据
    gp22_wr_config_reg(Bus_Type, 0x81, 0x67490000);
    diff_Cal2_Cal1_new = gp22_read_n_bytes(Bus_Type,4,0xB0,0x01,16);
    
    //--------------------------------------------------------------------------
    // Caluculate the real time after the hole first cycle loop 
    while (diff_Cal2_Cal1_old != 0) 
    {
        avg_diff_Cal2_Cal1 = (diff_Cal2_Cal1_new+diff_Cal2_Cal1_old) / 2;

       // average_RAW_Result /= N_Measure_Cycles;*****
    
        // Used Formulas:  使用公式
        // ---------------------------------------------------
        //                 T_ref                                       RAW_Value
        // Time_Value = ----------- * measured_RAW_Value  MY ： TIME=------------ * T_ref  (T_ref为未分频是的频率)
        //               Cal2-Cal1                                     Cal2-Cal1
        // ---------------------------------------------------
        //                   velocity_of_light
        // Distance_Value = ------------------- * Time_Value
        //                           2
        // ---------------------------------------------------
        
        // For this Source Code would be a Reference Clock used with 1 MHz  对于这个源代码将是一个频率为1兆赫的参考时钟
        
       // Time_Result = (average_RAW_Result/avg_diff_Cal2_Cal1) * 1000;//time [ns]****
        Time_Result = (RAW_Result/avg_diff_Cal2_Cal1) * 1000;//time [ns]

        Distance_Result = Time_Result / 6.671281904; //distance [m]
        
        printf("\n Time Measure Result (ToF) = %.6f ns   RAW_Result=%.6f ns\n",Time_Result,RAW_Result);
        printf(" corresponds to %.3f m of Distance\n",Distance_Result);
        printf(" to reflected point after %u Measurements\n",N_Measure_Cycles);

        diff_Cal2_Cal1_old = 0;
    }

    //--------------------------------------------------------------------------
    //average_RAW_Result = RAW_Result; // set first value of average_RAW_Result 平均RAW结果集的第一个值****
    
    //--------------------------------------------------------------------------
/*   // n'th ToF Measurement  第N个测量飞行时间
    for ( i=2; i<=N_Measure_Cycles;i++)
    {
        gp22_send_1byte(Bus_Type, Init);
        
        //Trigger pulse laser  脉冲触发激光
        //    SetPortHigh;   
			USART_SendData(USART2,'A');
		  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET){}//??????

        GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_SET); // Output HIGH
        //    SetPortLow;
        GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_RESET);   // Output LOW
    
        // Wait for INT Slot_x
        while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
        
        RAW_Result = gp22_read_n_bytes(Bus_Type,4,0xB0,0x00,16);
//        printf(" %u. Measure RAW Value = %.0f \n",i,RAW_Result); // RAW value 原始值
        average_RAW_Result += RAW_Result;
    }
    */
    //--------------------------------------------------------------------------

    printf("\nNEW CYCLE...\n"); 

    Dummy_var++; // To Control the loop
    
  } // End while Dummy_var

 //End main

 }
 
 
int Test_Communication(void)    //测试通信 
{ 
   u32 Test = 0x00000000;  u8 Test1=0x01;
	void* Bus_Type = SPI2;	
	//SPI2_SetSpeed(SPI_BaudRatePrescaler_4); //spi速度为9Mhz（最大SPI时钟为10Mhz）   	 

  	gp22_send_1byte(Bus_Type,Power_On_Reset); 
  //Delay(1); 
  	gp22_wr_config_reg(Bus_Type,0x81,0x22000000); 
  //Delay(1); 
    // Test = SPI_Read32(0xB5); 
	   Test1=gp22_read_n_bytes(Bus_Type,1,0xB0,0x05,0);
  	// Test1= Test1>>24; 
  //Delay(1); 
  if( Test1==0x22)  {		USART_SendData(USART1,'Y') ;GPIO_ResetBits(GPIOE,GPIO_Pin_5);	//GPIO_ResetBits(GPIOE,GPIO_Pin_5);
 // 片选tdc
return 0;}
		 else	   			{USART_SendData(USART1,'N');	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
		 return 1;}

} 


