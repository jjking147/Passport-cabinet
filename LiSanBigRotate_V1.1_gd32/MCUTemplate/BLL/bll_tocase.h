#ifndef __BLL_TOCASE_H__ 
#define __BLL_TOCASE_H__ 

#include "sys.h"
#include "bll_adapter.h"
#include "config.h"
#include "bll_main.h"

void BLL_FlyOut_EXTIHandler(void);	// X2/PC3 护照脱出

void BLL_ToCase_ClearFlag(void);
void NVIC_Config(u8 num,u8);	//外部中断初始化	
CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err);

void SWTICH1_INT_HANDLER(void);
void SWTICH2_INT_HANDLER(void);
void SWTICH3_INT_HANDLER(void);
void SWTICH4_INT_HANDLER(void);
 
#endif 

 
