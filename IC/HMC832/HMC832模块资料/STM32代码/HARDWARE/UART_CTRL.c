#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "UART_CTRL.h"
char CTRLBuffer[RXBUFSIZE];   
uint16_t chapower=20;
uint16_t chbpower=20;
uint16_t debug_outen=0;
uint8_t find_locked=0;
extern double REF_FREQ;
extern uint8_t CHA_EN;
extern uint8_t CHB_EN;
extern uint8_t LOCKED;
extern uint8_t OUT_POWER;
double NOW_FREQ=0;
/* SCPI command format 
channel: A1,A2
command: pll command; setfreq, output control ...
data or read: 
end:'\n' '0x0A'
	example:
		set freq(200Mhz) for A1 channel:"A1:FREQ SET,200 \n" 
		set A1 channel out power(1dBm):"A1:OUTPOW,1\n"
		set A1 channelA out enable/disable:"A1:OUTP CHA ON/OFF \n" 
		 
*/ 


void HMC832_CTRL(){
		char *temp_ret;
		double RX_freq=0;
		/****************DEBUG CTRL********************/
		if(strstr(CTRLBuffer, "A1:DEBUGON")){
			debug_outen=1;
		}
		if(strstr(CTRLBuffer, "A1:DEBUGOFF")){
			debug_outen=0;
		}		
		/****************SET PLL OUTPUT FREQ********************/
		temp_ret=strstr(CTRLBuffer, "A1:FREQ SET,");
		if(temp_ret){
			RX_freq=atof(temp_ret+12);
			NOW_FREQ=RX_freq;
			if(debug_outen) printf("freq=%f\n",RX_freq);
			HMC832_Write_freq(RX_freq);
			find_locked=1;
		}	
		/****************FIND PLL LOCKED/OUT********************/
		if(strstr(CTRLBuffer, "A1:LOCKED?")){
			LOCKED= HAL_GPIO_ReadPin(GPIOA,HMC_LD_Pin);
			HAL_GPIO_WritePin(GPIOA,LD_LED_Pin,LOCKED);
			if(LOCKED) printf("A1:LOCKED GOOD\n");
			else printf("A1:LOCKED LOSS\n");
		}	
		
		if(strstr(CTRLBuffer, "A1:OUTP CHA?")){
			if(CHA_EN) printf("A1:OUTP CHA ON\n");
			else printf("A1:OUTP CHA OFF\n");
		}				
	
		if(strstr(CTRLBuffer, "A1:OUTP CHB?")){
			if(CHB_EN) printf("A1:OUTP CHB ON\n");
			else printf("A1:OUTP CHB OFF\n");
		}	
		
		if(strstr(CTRLBuffer, "A1:FREQ NOW?")){
			printf("A1:FREQ NOW,%fMHz\n",NOW_FREQ);
		}	
		
		if(strstr(CTRLBuffer, "A1:FREQ REF?")){
			printf("A1:FREQ REF,%fMHz\n",REF_FREQ);
		}		
		
		if(strstr(CTRLBuffer, "A1:OUTPOW GAIN?")){
			printf("A1:OUTPOW GAIN,%ddB\n",OUT_POWER);
		}				

		/****************SET PLL OUTPUT POWER********************/

			if(strstr(CTRLBuffer, "A1:OUTPOW GAIN,")){
				temp_ret=strstr(CTRLBuffer, "A1:OUTPOW GAIN,");
				OUT_POWER=(uint8_t)atoi(temp_ret+15);
				Wirte_Data(REG_5,((VCO_REG_7&0xF87F)|(OUT_POWER<<7)));
			}

		/****************SET PLL OUTPUT CHANNEL********************/
			if(strstr(CTRLBuffer, "A1:OUTP CHA ON")){
				if(LOCKED)CHA_EN=1;
				else {
				CHA_EN=0;
				printf("A1:LOCKED LOSS\n");
				}
				Wirte_Data(REG_5,((VCO_REG_3&0xF9FF)|((CHA_EN)|(CHB_EN<<1))<<9));
				if(debug_outen) printf("A1:OUTP CHA ON\n");
			}
			
			if(strstr(CTRLBuffer, "A1:OUTP CHA OFF")){
				CHA_EN=0;
				Wirte_Data(REG_5,((VCO_REG_3&0xF9FF)|((CHA_EN)|(CHB_EN<<1))<<9));
				if(debug_outen) printf("A1:OUTP CHA OFF\n");
			}
			
			if(strstr(CTRLBuffer, "A1:OUTP CHB ON")){
				if(LOCKED)CHB_EN=1;
				else {
				CHB_EN=0;
				printf("A1:LOCKED LOSS\n");
				}
				Wirte_Data(REG_5,((VCO_REG_3&0xF9FF)|((CHA_EN)|(CHB_EN<<1))<<9));
				if(debug_outen) printf("A1:OUTP CHB ON\n");
			}			
			
				if(strstr(CTRLBuffer, "A1:OUTP CHB OFF")){
				CHB_EN=0;
				Wirte_Data(REG_5,((VCO_REG_3&0xF9FF)|((CHA_EN)|(CHB_EN<<1))<<9));
				if(debug_outen) printf("A1:OUTP CHB OFF\n");
			}		
				
			/****************SET PLL REF********************/		
			if(strstr(CTRLBuffer, "A1:FREQ REF,")){
				temp_ret=strstr(CTRLBuffer, "A1:FREQ REF,");
				REF_FREQ=atof(temp_ret+12);
				HMC832_Write_freq(NOW_FREQ);
			}
			

		
}
double String_float(char *strbuffer){
		uint16_t frac_count=strlen(strbuffer);
		uint16_t point_count=0;
		char temp_inter[10];
		char temp_frac[10];
		double dataout=0;
		double inter=0;
		double frac=0;
		for(int i=0;i<frac_count;i++){
			if(strbuffer[i] == '.')break;
			else point_count++;
		}
		for(int i=0;i<point_count;i++){
		temp_inter[i]=strbuffer[i];
		}
		temp_inter[point_count]=(char)0;

		for(int i=0;i<(frac_count-point_count);i++){
		temp_frac[i]=strbuffer[1+i+point_count];
		}
		temp_frac[frac_count]=(char)0;
			
		inter = atoi(temp_inter);
		frac = atoi(temp_frac);
		double temp_di=pow(10,(frac_count-point_count-1));
		frac = frac/temp_di;
		dataout = inter+frac;
		printf("inter=%f,frac=%f,dataout=%f\n",inter,frac,dataout);
		return dataout;
}