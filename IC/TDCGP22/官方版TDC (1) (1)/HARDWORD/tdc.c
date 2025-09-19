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

float     CLKHS_freq = 1.000;           // Clock frequency in MHz  ʱ��Ƶ��
float     CLKHS_freq_cal = 1.000;       // Calibrated Clock frequency in MHz  У׼ʱ��Ƶ��
float     CLKHS_freq_corr_fact = 1.000; // Correction factor for Clock frequency  ����ʱ��Ƶ������ϵ��

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
//R0, A, B are parameters as specified in EN60 751   RO,A,B�ǲ���ΪEN 6O751�涨
float     R0_at_0C = 1000;            // R0 is the RTD resistance at 0 �C  R0����0��ʱ��RTD����
float     Coeff_A  = 3.9083/1000;     // A = 3,9083 x 10-3 �C-1
float     Coeff_B  = -5.775/10000000; // B = -5,775 x 10-7 �C-1
float     R1 = 0, R2 = 0;             // R1, R2 is resistance measured in Ohm   R1��R2�ǵ��������ŷ�ֵ�
float     corr_fact = 1.0000;   // Corr.-factor for temperature resistance ratio   �¶ȵ�������� Corr

/* Opcodes  ������-------------------------------------------------------------------*/
uint8_t   Init =                0x70;
uint8_t   Power_On_Reset =      0x50;
uint8_t   Start_TOF =           0x01;
uint8_t   Start_Temp =          0x02;
uint8_t   Start_Cal_Resonator = 0x03;   // Resonator г����
uint8_t   Start_Cal_TDC =       0x04;
uint8_t   Start_TOF_Restart =   0x05;
uint8_t   Start_Temp_Restart =  0x06;

uint32_t  *sram_memory     = ((uint32_t *)(SRAM_BASE + 0xB00));
uint32_t  sram_mem_offset  = 0x0;

uint32_t  Dummy_var = 0;

// For mathematical calculations   ��ѧ����
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
 * Description: Writes the Opcode to GP22 д�������
 *
 ******************************************************************************/
void gp22_send_1byte (void *bus_type, uint8_t gp22_opcode_byte)
{
     // Deactivating Reset SPIx ͣ�ø�λ��SPI
     GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_RESET);
     
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     SPI_I2S_SendData(bus_type, gp22_opcode_byte);     // OPCODE TO Device  �豸������
     while (SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE)==0) {} 
     Simple_delay_750ns((void*)10); // important delay (16) at SPI freq.=750kHz 
     // Reset to device SPIx ���õ��豸��SPI
     GPIO_WriteBit(GPIOB, GPIO_Pin_12, Bit_SET);
}
/*******************************************************************************
 * Function Name: gp22_wr_config_reg
 * Parameters: Address byte, 4 bytes of Configuration   ��������ַ�ֽڣ�4���ֽڵ�����
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
   
      // Deactivating Reset SPIx ͣ�ø�λSPI
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
 *             n_bytes = how many bytes should be read �����ֽ�Ӧ��
 *             read_opcode = read opcode of the device ��ȡ�豸�Ĳ�����
 *             read_addr = read address of the device  �豸�Ķ���ַ
 *             fractional_bits = number of fractional bits of read data ��ȡ���ݵ�С��λ��
 *
 * Return: n bytes from the specified read address  ���أ�n���ֽڴ�ָ���Ķ�ȡ��ַ
 *
 * Description: Reads n bytes from an address in GP22 ��GP22��ַ ��ȡn���ֽ�
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
       
       //Compulsory reads to DR and SR to clear OVR, ǿ�ƶ�ȡDR��SRȥ���OVR
			 //so that next incoming data is saved  �Ա��´��������ݱ�����
       SPI_I2S_ReceiveData(bus_type);                     // To clear OVR
       SPI_I2S_GetFlagStatus(bus_type, SPI_I2S_FLAG_TXE); // To clear OVR

         //Reading byte1 ��ȡ�ֽ�1
         SPI_I2S_SendData(bus_type, 0x00F1);  // DUMMY WRITE ��д
         // Wait until RX buffer is not empty, then read the received data �ȴ����ջ�������Ϊ�գ�Ȼ���ȡ���յ�������
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
 * Return: 2 bytes from the status register, address 0x04 ���أ���״̬�Ĵ�����2���ֽڣ���ַ0X04
 *
 * Description: Reads 2 bytes from an address in GP22  ��GP22��ַ��ȡ2���ֽ�
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
 * Return: only one Error Bit (9..15) from the status register, address 0x04  ��״̬�Ĵ�����һ������λ ��ַ��0X04
 *
 * Description: Reads 2 bytes from an address in GP22 ��GP22��ַ��ȡ2���ֽ�
 ******************************************************************************/              
uint8_t gp22_status_count_error(void *bus_type)
{
  uint16_t STAT_REG = 0x0000;
  uint8_t  count_error     = 0;

  STAT_REG = gp22_read_status_bytes(bus_type);
  
  if ((STAT_REG&0x0200)==0x0200) count_error++; //Bit9: Timeout_TDC ��ʱTDC
  if ((STAT_REG&0x0400)==0x0400) count_error++; //Bit10: Timeout_Precounter ��ʱPrecounter
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
 * Description: Analyse the bit from the status register  ������״̬�Ĵ�����λ
 ******************************************************************************/              
void gp22_analyse_error_bit(void *bus_type)
{
  uint16_t STAT_REG = 0x0000;

  STAT_REG = gp22_read_status_bytes(bus_type);
  
  //Bit9: Timeout_TDC
  if ((STAT_REG&0x0200)==0x0200) 
    printf("\n-Indicates an overflow of the TDC unit,��ʾTDC��Ԫ���");
  //Bit10: Timeout_Precounter
  if ((STAT_REG&0x0400)==0x0400) 
    printf("\n-Indicates an overflow of the 14 bit precounter in MR 2����ʾ14λPRECOUNTER��MR2���");
  //Bit11: Error_open
  if ((STAT_REG&0x0800)==0x0800) 
    printf("\n-Indicates an open sensor at temperature measurement������ʾ�����¶ȵĴ�������·");
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
 
      // Writing to the configuration registers (CR)  д���üĴ���
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
    // ........................Calibrate High Speed Clock....................... У׼����ʱ��
    // ...................Laser Rangefinder Measurement CYCLE................... �������ǲ�������
    // .........................Caluculate Result Values........................ ������ֵ
    
   // N_Measure_Cycles = 5000;
    
    diff_Cal2_Cal1_old = diff_Cal2_Cal1_new;

if((Dummy_var==0) | (Dummy_var==10))
{
    //--------------------------------------------------------------------------
    // Start Calibrate High Speed Clock Cycle ��ʼУ׼����ʱ������
    gp22_send_1byte(Bus_Type, Init);
    gp22_send_1byte(Bus_Type, Start_Cal_Resonator);

    // Wait for INT Slot_x  �ȴ��ж��ź�
	 //  Test1=gp22_read_n_bytes(Bus_Type,1,0xB0,0x05,0);
  //   if(Test1==0x19)     GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);   // Output LOW
    while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 

    //Calculate Correction factor  ��������ϵ��
    //The time interval to be measured is set by ANZ_PER_CALRES  Ҫ��������ʱ������ANZ_PER_CALRES����
    //which defines the number of periods of the 32.768 kHz clock: ��������32.768KHZʱ�ӵ�������
    //2 periods = 61.03515625 �s            2������=61.03515625΢��
    CLKHS_freq_corr_fact = 61.03515625/
      gp22_read_n_bytes(Bus_Type, 4, 0xB0, 0x00, 16) * CLKHS_freq;
   
    printf("\n Correction factor for clock = %1.4f\n", CLKHS_freq_corr_fact);   //ʱ��У��ϵ��=
       
    CLKHS_freq_cal = CLKHS_freq * CLKHS_freq_corr_fact; // Calibrated Clock frequency  У׼ʱ��Ƶ��
}

    //--------------------------------------------------------------------------
    // Start Seperate Calibration Measurement Cycle      ��������У׼��������
    gp22_send_1byte(Bus_Type, Init);
    
    gp22_send_1byte(Bus_Type, Start_Cal_TDC); // update calibration data ����У׼����

    // Note: ע��
    // The calibration data are not addressed directly after the calibration  У׼���ݲ���У׼����
    // measurement but after the next regular measurement, before the next INIT. ���������������ĳ������������һ INIT
    //����һ���������֮������һ��INIT֮ǰ����
    // Wait for INT Slot_x   �ȴ�
     while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
    gp22_wr_config_reg(Bus_Type, 0x81, 0x19490000);

    //--------------------------------------------------------------------------
    // 1st ToF Measurement plus calibratio data readout ��һ�η���ʱ�������У�����ݶ���
    gp22_send_1byte(Bus_Type, Init);

    //Trigger pulse laser �������弤��
    //    SetPortHigh;   
		//USART_SendData(USART2,'A');
		//while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET){}//??????
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_SET); // Output HIGH
    //    SetPortLow;
    GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_RESET);   // Output LOW
   delay_us(1);  //����1US��ʱ���
	 
   GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_SET); // Output HIGH
   GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_RESET);   // Output LOW
    //TIM3_Int_Init(79,0);
    // Wait for INT Slot_x �ȴ��ж�
    while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
//GPIO_WriteBit(GPIOD, GPIO_Pin_9, Bit_RESET);   // Output LOW


    // First regular measurement (to readout calibration data) ���ȶ�ʱ�������Զ���У׼���ݣ�
    RAW_Result = gp22_read_n_bytes(Bus_Type,4,0xB0,0x00,16);
//    printf("\n 1. Measured RAW Value = %.0f \n",RAW_Result); // RAW value  ����ԭʼֵ

    // Check Status Register, next free result register   ���״̬�Ĵ�������һ�����н���Ĵ���
//    printf("Stat_Reg = 0x%04X \n",gp22_read_status_bytes(Bus_Type)); 

    // readout the new calibration data from result register adr 0x01  �ӽ���Ĵ�����ַΪ0X01�����µ�У׼����
    gp22_wr_config_reg(Bus_Type, 0x81, 0x67490000);
    diff_Cal2_Cal1_new = gp22_read_n_bytes(Bus_Type,4,0xB0,0x01,16);
    
    //--------------------------------------------------------------------------
    // Caluculate the real time after the hole first cycle loop 
    while (diff_Cal2_Cal1_old != 0) 
    {
        avg_diff_Cal2_Cal1 = (diff_Cal2_Cal1_new+diff_Cal2_Cal1_old) / 2;

       // average_RAW_Result /= N_Measure_Cycles;*****
    
        // Used Formulas:  ʹ�ù�ʽ
        // ---------------------------------------------------
        //                 T_ref                                       RAW_Value
        // Time_Value = ----------- * measured_RAW_Value  MY �� TIME=------------ * T_ref  (T_refΪδ��Ƶ�ǵ�Ƶ��)
        //               Cal2-Cal1                                     Cal2-Cal1
        // ---------------------------------------------------
        //                   velocity_of_light
        // Distance_Value = ------------------- * Time_Value
        //                           2
        // ---------------------------------------------------
        
        // For this Source Code would be a Reference Clock used with 1 MHz  �������Դ���뽫��һ��Ƶ��Ϊ1�׺յĲο�ʱ��
        
       // Time_Result = (average_RAW_Result/avg_diff_Cal2_Cal1) * 1000;//time [ns]****
        Time_Result = (RAW_Result/avg_diff_Cal2_Cal1) * 1000;//time [ns]

        Distance_Result = Time_Result / 6.671281904; //distance [m]
        
        printf("\n Time Measure Result (ToF) = %.6f ns   RAW_Result=%.6f ns\n",Time_Result,RAW_Result);
        printf(" corresponds to %.3f m of Distance\n",Distance_Result);
        printf(" to reflected point after %u Measurements\n",N_Measure_Cycles);

        diff_Cal2_Cal1_old = 0;
    }

    //--------------------------------------------------------------------------
    //average_RAW_Result = RAW_Result; // set first value of average_RAW_Result ƽ��RAW������ĵ�һ��ֵ****
    
    //--------------------------------------------------------------------------
/*   // n'th ToF Measurement  ��N����������ʱ��
    for ( i=2; i<=N_Measure_Cycles;i++)
    {
        gp22_send_1byte(Bus_Type, Init);
        
        //Trigger pulse laser  ���崥������
        //    SetPortHigh;   
			USART_SendData(USART2,'A');
		  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)!=SET){}//??????

        GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_SET); // Output HIGH
        //    SetPortLow;
        GPIO_WriteBit(GPIOD, GPIO_Pin_8, Bit_RESET);   // Output LOW
    
        // Wait for INT Slot_x
        while (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4)==1); 
        
        RAW_Result = gp22_read_n_bytes(Bus_Type,4,0xB0,0x00,16);
//        printf(" %u. Measure RAW Value = %.0f \n",i,RAW_Result); // RAW value ԭʼֵ
        average_RAW_Result += RAW_Result;
    }
    */
    //--------------------------------------------------------------------------

    printf("\nNEW CYCLE...\n"); 

    Dummy_var++; // To Control the loop
    
  } // End while Dummy_var

 //End main

 }
 
 
int Test_Communication(void)    //����ͨ�� 
{ 
   u32 Test = 0x00000000;  u8 Test1=0x01;
	void* Bus_Type = SPI2;	
	//SPI2_SetSpeed(SPI_BaudRatePrescaler_4); //spi�ٶ�Ϊ9Mhz�����SPIʱ��Ϊ10Mhz��   	 

  	gp22_send_1byte(Bus_Type,Power_On_Reset); 
  //Delay(1); 
  	gp22_wr_config_reg(Bus_Type,0x81,0x22000000); 
  //Delay(1); 
    // Test = SPI_Read32(0xB5); 
	   Test1=gp22_read_n_bytes(Bus_Type,1,0xB0,0x05,0);
  	// Test1= Test1>>24; 
  //Delay(1); 
  if( Test1==0x22)  {		USART_SendData(USART1,'Y') ;GPIO_ResetBits(GPIOE,GPIO_Pin_5);	//GPIO_ResetBits(GPIOE,GPIO_Pin_5);
 // Ƭѡtdc
return 0;}
		 else	   			{USART_SendData(USART1,'N');	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);
		 return 1;}

} 


