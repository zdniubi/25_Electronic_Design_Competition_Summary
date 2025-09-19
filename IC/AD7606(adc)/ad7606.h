#ifndef AD7606_H
#define AD7606_H

/*

    OS0 -> PB11
    OS1 -> PB6
    OS2 -> PB8
    ST_A,ST_B -> PE0
    RST -> PE3
    CS -> PE4
    SCK -> PE2
    busy -> PC13
    DB7 -> PE5
    DB8 -> PE6
    DB15 -> GND

*/

#include "main.h"

#define adc_length 100 //采样长度
#define adc_quantity 8 //通道数,2的倍数
#define adc_oncequantity 8 //一次采样通道数，2的倍数


#define SPI_CLK_Pin GPIO_PIN_2
#define SPI_CLK_GPIO_Port GPIOE
#define rst_Pin GPIO_PIN_3
#define rst_GPIO_Port GPIOE
#define SPI_CS_Pin GPIO_PIN_4
#define SPI_CS_GPIO_Port GPIOE
#define SPI_MISO_A_Pin GPIO_PIN_5
#define SPI_MISO_A_GPIO_Port GPIOE
#define SPI_MISO_B_Pin GPIO_PIN_6
#define SPI_MISO_B_GPIO_Port GPIOE
#define busy_Pin GPIO_PIN_13
#define busy_GPIO_Port GPIOC
#define OS0_Pin GPIO_PIN_11
#define OS0_GPIO_Port GPIOB
#define OS1_Pin GPIO_PIN_6
#define OS1_GPIO_Port GPIOB
#define OS2_Pin GPIO_PIN_8
#define OS2_GPIO_Port GPIOB
#define CV_Pin GPIO_PIN_0
#define CV_GPIO_Port GPIOE





#define OS0_1 HAL_GPIO_WritePin(OS0_GPIO_Port,OS0_Pin,GPIO_PIN_SET);//AD速率控制引脚
#define OS0_0 HAL_GPIO_WritePin(OS0_GPIO_Port,OS0_Pin,GPIO_PIN_RESET);

#define OS1_1 HAL_GPIO_WritePin(OS1_GPIO_Port,OS1_Pin,GPIO_PIN_SET);
#define OS1_0 HAL_GPIO_WritePin(OS1_GPIO_Port,OS1_Pin,GPIO_PIN_RESET);

#define OS2_1 HAL_GPIO_WritePin(OS2_GPIO_Port,OS2_Pin,GPIO_PIN_SET);
#define OS2_0 HAL_GPIO_WritePin(OS2_GPIO_Port,OS2_Pin,GPIO_PIN_RESET);

#define CV_1 HAL_GPIO_WritePin(CV_GPIO_Port,CV_Pin,GPIO_PIN_SET);
#define CV_0 HAL_GPIO_WritePin(CV_GPIO_Port,CV_Pin,GPIO_PIN_RESET);


#define RST_1 HAL_GPIO_WritePin(rst_GPIO_Port,rst_Pin,GPIO_PIN_SET);//复位信号
#define RST_0 HAL_GPIO_WritePin(rst_GPIO_Port,rst_Pin,GPIO_PIN_RESET);

#define CS_1 HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin,GPIO_PIN_SET);//片选信号
#define CS_0 HAL_GPIO_WritePin(SPI_CS_GPIO_Port,SPI_CS_Pin,GPIO_PIN_RESET);


void AD7606_Init();
void AD7606_GPIO_Init(void);
void AD7606_OnceConvert(float voltage[adc_oncequantity]); // 一次采样转换
void AD7606_Convert(float voltage[adc_quantity][adc_length]); // 多次采样转换
float findmax_7606(float *data);
float average_7606(float *data);




#endif