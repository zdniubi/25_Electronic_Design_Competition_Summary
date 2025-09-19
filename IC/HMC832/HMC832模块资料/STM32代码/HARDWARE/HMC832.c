#include "HMC832.h"
#include "stdio.h"

double 	DIV_data=0;
uint32_t RF_DIV=0;
uint8_t CHA_EN=0;
uint8_t CHB_EN=0;
uint8_t LOCKED=0;
uint8_t OUT_POWER=0;
double REF_FREQ=50;
extern uint16_t debug_outen;

void Wirte_Data(uint8_t ADDR,uint32_t Data){
	PLL_CS_SET();
	PLL_SCK_RESET();
	PLL_CS_RESET();
	for(int j=0;j<24;j++){//16bit data
			PLL_SCK_RESET();
			HAL_GPIO_WritePin(GPIOA,HMC_SDO_Pin,(Data >> (23-j)) & 0x01);
			PLL_SCK_SET();
		}
	
	for(int j=0;j<8;j++){//8bit addr
			PLL_SCK_RESET();
			HAL_GPIO_WritePin(GPIOA,HMC_SDO_Pin,(ADDR >> (7-j)) & 0x01);
			PLL_SCK_SET();
		}

	PLL_SCK_SET();
	PLL_CS_SET();
}

void HMC832_init(){

	Wirte_Data(REG_1,REG_DATA_1);

	Wirte_Data(REG_2,REG_DATA_2);

	Wirte_Data(REG_5,VCO_REG_2);//

	Wirte_Data(REG_5,VCO_REG_3);//

	Wirte_Data(REG_5,VCO_REG_7);//

	Wirte_Data(REG_5,VCO_REG_0);//

	Wirte_Data(REG_6,REG_DATA_6);

	Wirte_Data(REG_7,REG_DATA_7);

	//Wirte_Data(REG_8,REG_DATA_8);

	Wirte_Data(REG_9,REG_DATA_9);

	Wirte_Data(REG_A,REG_DATA_A);

	Wirte_Data(REG_F,REG_DATA_F);

//	Wirte_Data(REG_3,REG_DATA_3);

//	Wirte_Data(REG_4,REG_DATA_4);

}
//50-100 n
void HMC832_Write_freq(double freq){
	Find_DIV(freq);
	uint32_t int_buf=0;
	uint32_t FRAC_buf=0;
	double N_data=0;
	double frac_data=0;
	N_data=(freq*DIV_data)/REF_FREQ;//PFD=50Mhz
	int_buf=(int32_t)N_data;
	frac_data=(N_data-int_buf)*(1<<24);//
	FRAC_buf=(int32_t)frac_data;
	if(debug_outen)printf("DIV_data=%f,INTN_data=%d,N_data=%f,frac_data=%f\n",DIV_data,int_buf,N_data,frac_data);
	Wirte_Data(REG_1,REG_DATA_1);

	Wirte_Data(REG_2,REG_DATA_2);

	Wirte_Data(REG_5,RF_DIV);//change div

	Wirte_Data(REG_5,((VCO_REG_3&0xF9FF)|((CHA_EN)|(CHB_EN<<1))<<9));//

	Wirte_Data(REG_5,((VCO_REG_7&0xF87F)|(OUT_POWER<<7)));//

	Wirte_Data(REG_5,VCO_REG_0);//

	Wirte_Data(REG_6,REG_DATA_6);

	Wirte_Data(REG_7,REG_DATA_7);

	Wirte_Data(REG_9,REG_DATA_9);

	Wirte_Data(REG_A,REG_DATA_A);

	Wirte_Data(REG_F,REG_DATA_F);

	Wirte_Data(REG_3,int_buf);

	Wirte_Data(REG_4,FRAC_buf);

}

void Find_DIV(double freq){

		if((25<=freq) & (freq<=31.25)){
			DIV_data=60;
			RF_DIV= (uint32_t)0x1E10;
		}
		else if((31.25<freq) & (freq<=62.5)){
			DIV_data=48;
			RF_DIV= (uint32_t)0x1810;
		}
		else if((62.5<freq) & (freq<=125)){
			DIV_data=24;
			RF_DIV= (uint32_t)0x0C10;
		}		
		else if((125<freq) & (freq<=200)){
			DIV_data=12;
			RF_DIV=(uint32_t)0x0610;
		}
		
		else if((200<freq) & (freq<=375)){
			DIV_data=8;
			RF_DIV= (uint32_t)0x0410;
		}
		else if((375<freq) & (freq<=750)){
			DIV_data=4;
			RF_DIV=(uint32_t)0x0210;
		}
		else if((750<freq) & (freq<=1500)){
			DIV_data=2;
			RF_DIV= (uint32_t)0x0110;
		}
		else if((1500<freq) & (freq<=3000)){
			DIV_data=1;
			RF_DIV= (uint32_t)0x0090;
		}
		else {
			printf("A1:FREQ OUT OF RANGE\n");
			DIV_data=0;
			RF_DIV= (uint32_t)0x1E10;
		}
}
