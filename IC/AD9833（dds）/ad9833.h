#ifndef _AD_9833_H
#define _AD_9833_H
#include <math.h>
#include "main.h"
#include <stdbool.h>

// ------------------------- Defines -------------------------
#define FMCLK 25000000        // Master Clock On AD9833
#define AD9833PORT GPIOA      // PORT OF AD9833
#define AD9833DATA GPIO_PIN_7 // SPI DATA PIN
#define AD9833SCK GPIO_PIN_5  // SPI Clock PIN
#define AD9833SS1 GPIO_PIN_4   // SPI Chip Select
#define AD9833SS2 GPIO_PIN_3   // SPI Chip Select
#define ASM_NOP()
// Assembly NOPE (Little Delay)
enum WaveType
{
    SIN,
    SQR,
    TRI
}; // Wave Selection Enum

// ------------------ Functions  ---------------------
void AD9833_Init(bool select,uint8_t WaveType, float FRQ, float Phase); // Initializing AD9833
void AD9833_GPIO_Init(void);
void AD9833_SetRegister_1(uint8_t WaveType,float Frequency,float Phase); // Sets Frequency Register
void AD9833_SetRegister_2(uint8_t WaveType,float Frequency,float Phase); // Sets Frequency Register
void AD9833_Reset_1(void);
void AD9833_Reset_2(void);
#endif