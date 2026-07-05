#include "bll_tocase.h"
#include "delay.h"
#include "usart.h"
#include "modbus_master.h"
#include "motor.h"
#include "gpio.h"

static CommonStateFlag_Type flag = CSF_Idel;//����ԭ����״̬
extern u8 swtich_flag;
extern u8 swtich_count;
u8 zero_flag = 0;
u8 photoelectric_sign = 0;
extern u8 Master_Receive_Buff[MODBUS_BUFF_LEN];

//���flag״̬
void BLL_ToCase_ClearFlag(void)
{
	flag = CSF_Idel;
}

CommonStateFlag_Type BLL_ToCase_Execute(ParamShadow_Type params, u8 *err)
{

	if(flag == CSF_Idel)
	{
		flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);
		
		if(zero_flag == 0)
		{
        Master_Set_Idle();
        Init_Parameter();
        Master_Set_Idle();
        Motor_Enable();
        Master_Set_Idle();
        Set_Back_Zero_Mode();
        Master_Set_Idle();
        Set_Neg_Limit_Mode();
        Master_Set_Idle();
        Set_Back_Zero_Acce(params.Param4);
        Master_Set_Idle();
        Set_Back_Zero_Speed(params.Param5);
        Master_Set_Idle();
        Set_Abso_Position_Mode();
        Master_Set_Idle();
        Abso_Position_Start_Sample();
        Master_Set_Idle();
		}
		
		Master_Set_Idle();
		Driver_Power_On();
		Master_Set_Idle();
		Init_Parameter();
		Master_Set_Idle();
		Motor_Enable();//���ʹ��
		Master_Set_Idle();
		Set_Position_Mode();//����λ��ģʽ
		Master_Set_Idle();
		Position_Mode_Set_Speed(params.Param1);//����Ŀ���ٶȣ���ֵ��
		Master_Set_Idle();
		Set_Acce(1);//���ü��ٶ�
		Master_Set_Idle();
		Set_Dece(1);//���ü��ٶ�
		Master_Set_Idle();
		Set_Distence(1000);//��������λ��
		Master_Set_Idle();
		Set_Abso_Position_Mode();//���þ���λ��ģʽ
		Master_Set_Idle();
		Abso_Position_Start_Sample();//�����˶�
		Master_Set_Idle();
		Check_Status();//�����״̬
		Master_Set_Idle();
		if((Master_Receive_Buff[0] & 0X0001) == 0X0001)
		{
		    Fast_Dece_Stop_Motor_Enable();
		    Master_Set_Idle();
			*err = 1; //�����������
		    return CSF_Finished; 
		}
		
		do
		{
		Master_Set_Idle();
		Check_Status();//�����״̬
		Master_Set_Idle();
		}
		while((Master_Receive_Buff[0] & 0X0400) != 0X0400);
		
		
		   if(photoelectric_sign != 1)
		   {
		      if(swtich_count == 3)
		      {
		       Master_Set_Idle();
		       Driver_Power_On();
	           Master_Set_Idle();
		       Init_Parameter();
		       Master_Set_Idle();
		       Motor_Enable();//���ʹ�� 
			   Master_Set_Idle();
               Set_Speed_Mode();
               Master_Set_Idle();
	       	   Set_Acce(1);//���ü��ٶ�
		       Master_Set_Idle();
		       Set_Dece(1);//���ü��ٶ�
		       Master_Set_Idle();
               Position_Mode_Set_Speed(params.Param2);//����Ŀ���ٶȣ����ٶȣ���ֵ���Ƚ�С��	
               Master_Set_Idle();				  
		      }
		      else if(swtich_count == 2)			  
			  {
		       Master_Set_Idle();
		       Driver_Power_On();
	           Master_Set_Idle();
		       Init_Parameter();
		       Master_Set_Idle();
		       Motor_Enable();//���ʹ�� 
			   Master_Set_Idle();
               Set_Speed_Mode();
               Master_Set_Idle();
	       	   Set_Acce(1);//���ü��ٶ�
		       Master_Set_Idle();
		       Set_Dece(1);//���ü��ٶ�
		       Master_Set_Idle();
               Position_Mode_Set_Speed(params.Param3);//����Ŀ���ٶȣ����ٶȣ���ֵ���Ƚ�С�� 
               Master_Set_Idle();				  
			  }
		      else 
			  {
			   Master_Set_Idle();
			   Fast_Dece_Stop_Motor_Enable();
		       Master_Set_Idle();
			   *err = 2; //����������
		       return CSF_Finished; 
			  }
			  
			   if( photoelectric_sign == 1)
		      {
		       Master_Set_Idle();
               Fast_Dece_Stop_Motor_Enable();
		       Master_Set_Idle();
		       return CSF_Finished; 
		      }
		   }
		      if((swtich_flag & photoelectric_sign) == 1)
	          {
		       Master_Set_Idle();  
		       Stop();
		       swtich_flag = 0;
		       Master_Set_Idle();
		       flag = CSF_Finished;
	          }
      
	}
	return flag;
}