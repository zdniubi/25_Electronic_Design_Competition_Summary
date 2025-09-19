/*
PC4 enable
PC5 INTB
trigger(TDC7200���������ʾ���Խ���start��)
PE7 start
*/

/*
    �÷���
    TDC_Init(); // ��ʼ��TDC7200
    TDC7200_preInit_stopmask(ns); // ����stopmask�Ĵ�
    TDC7200_Init(); // ����TDC7200�Ĵ�������������
    �ȴ��ж�
    TDC_Write_Readtest(); // ��ȡ���ݲ�׼��������һ��start
    double time = complate(); // ����ʱ��

    TDC_Init_Check(); // ���TDC7200״̬
*/



#ifndef TDC7200_H
#define TDC7200_H

#include "main.h"
#include "SPI.h"

#define TDC_CLOCK 16000000 // 8000000
#define CALIBRATION_PERIODS 10
#define CALIBRATION_PERIODS_D1 CALIBRATION_PERIODS - 1
#define Clock_NS 62.5 // 62.5 ns
#define TIE_NS 1000000000
#define TIE_DIF 4000000

void TDC_Init(void);
void TDC7200_preInit_stopmask(float stoptime);
double complate(void);
double complate_mode1(void);
void TDC7200_Init(void);
void TDC7200_Init_mode1(void);
void TDC7200_Init_mode1_Down(void);
void TDC7200_Init_Down(void);
void TDC7200_Write(uint8_t addr, uint8_t data);
uint32_t TDC7200_Read(uint8_t addr);
uint8_t TDC7200_Read_ADDR(uint8_t addr);
void TDC7200_Write_Data_16(uint16_t data, uint8_t len);
void TDC_Write_Readtest(void);
void TDC_INT_Check(void);




#endif