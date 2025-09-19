#ifndef _AD_9833_H
#define _AD_9833_H

#include <math.h>
//#include "main.h"

typedef struct _PID
{
	float kp,ki,kd;
	float error,lastError;//误差、上次误差
	float integral,maxIntegral;//积分、积分限幅
	float output,maxOutput;//输出、输出限幅
}PID;

#define LIMIT(x,min,max) (x)=(((x)<=(min))?(min):(((x)>=(max))?(max):(x)))

void PID_Init(PID *pid, float p, float i, float d, float maxI, float maxOut);
void PID_SingleCalc(PID *pid, float reference, float feedback);
void PID_Clear(PID *pid);






#endif