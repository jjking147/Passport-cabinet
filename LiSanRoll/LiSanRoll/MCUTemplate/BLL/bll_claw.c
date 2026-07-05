#include "bll_claw.h"
#include "bll_adapter.h"
#include "bll_motor.h"
#include "modbus_master.h"
#include "config.h"
#include "delay.h"
#include "usart.h"
#include "gpio.h"
#include "timer.h"

static CommonStateFlag_Type tocase_flag = CSF_Idel;//����ԭ����״̬
vu8 swtich_flag;
vu8 swtich_count;
static vu8 zero_move_flag;
static vu8 fast_move_flag;
static vu8 slow_move_flag;

//extern u8 Master_Receive_Buff[MODBUS_MASTER_BUFF_LEN];

//��צ������Χ0~255����Ӧ��Ϊ2~50N
#define CLAW_POW_CLOSE	(80)	//�ر�ʱ��צ��������С0�����255 100 to 80
#define CLAW_POW_HALF	(80)	//�뿪ʱ��צ��������С0�����255 100 to 80
#define CLAW_POW_MICRO	(80)	//΢��ʱ��צ��������С0�����255 100 to 80
#define CLAW_POW_FULL	(80)	//ȫ��ʱ��צ��������С0�����255 100 to 80 


//���flag״̬
void BLL_Claw_ClearFlag(void)
{
	tocase_flag = CSF_Idel;
}

#define WAIT_MOTOR_STOP(span,n,label)	{ \
	u16 _motor_sate = 0,_retry = 0; \
	do \
	{ \
		if(_retry++ >= n) \
		{ \
			*err = Failure_Timeout; \
			goto label; \
		} \
		delay_ms(span); \
		_motor_sate = Check_Status(); \
	}while((_motor_sate & 0x02) == 0x02); \
}

#define WAIT_CLAW_STOP(span,n,label)	{ \
	u16 _CLAW_sate = 0,_retry = 0; \
	do \
	{ \
		if(_retry++ >= n) \
		{ \
			*err = Failure_Timeout; \
			goto label; \
		} \
		delay_ms(span); \
	_CLAW_sate = READ_JODELL_STATUS_NUMBER_2(); \
	}while((_CLAW_sate & 0xF1) == 0xF1); \
} 

static u8 Claw_First = 1;

CommonStateFlag_Type BLL_Claw_Execute(ParamShadow_Type params, u8 *err)
{
	if(tocase_flag == CSF_Idel)
	{		
    uint8_t high_byte = (params.Param1 >> 8) & 0xFF;  // ��ȡ�� 8 λ
    uint8_t low_byte = params.Param1 & 0xFF;          // ��ȡ�� 8 λ
		
		//Step0����ʼ�����ֱ�־λ��
		tocase_flag = CSF_Working;
		USART_ClearFlag(MOTOR_USART, USART_FLAG_TC);	
		if(Claw_First)
		{
			JODELL_STATUS(0x0000);
			delay_ms(10);
			JODELL_STATUS(0x0001);//ÿ��ʹ�ü���һ��
			delay_ms(10);
			Claw_First = 0;
		}
		
		if((high_byte & 0xF0)==0)
		{
			switch (high_byte)
			{
				case 0x01:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D1);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x02:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D2);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x03:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D3);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x04:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D4);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x05:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D5);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x06:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D6);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;
				case 0x07:
					fast_move_flag = 1;
					BLL_Motor_AD_RelativeMove(3000,1,1,D7);
					WAIT_MOTOR_STOP(100,200,die);	//100ms��һ�Σ���200�β��оͳ�ʱ
					fast_move_flag = 0;
				break;

				case 0:
				// �޶���
				break;
				default:
					printf("�� 8 λ����δ֪����ָ��: 0x%02X\n", low_byte);
					break;
			}
			switch (low_byte)
			{
				case 0x10:
					Claw_Action(0x10); // ���� Claw_Action �������м�צ�رղ���
				break;
				case 0x20:
					Claw_Action(0x20); // ���� Claw_Action �������м�צ�򿪲���
				break;
				case 0x30:
					Claw_Action(0x30); // ���� Claw_Action �������м�צ�򿪲���
				break;
				case 0:
				// �޶���
				break;
				default:
					//printf("�� 8 λ����δ֪����ָ��: 0x%02X\n", high_byte);
				break;
			}
	  }
		else
	  {
			switch (high_byte)
			{
				case 0x10:
					Claw_Action(0x10); // ���� Claw_Action �������м�צ�رղ���
				break;
				case 0x20:
					Claw_Action(0x20); // ���� Claw_Action �������м�צ�򿪲���
				break;
				case 0x30:
					Claw_Action(0x30); // ���� Claw_Action �������м�צ�򿪲���
				break;
					case 0x40:
					Claw_Action(0x40); // ���� Claw_Action �������м�צ�򿪲���
				break;
				case 0:
				// �޶���
				break;
				default:
					//printf("�� 8 λ����δ֪����ָ��: 0x%02X\n", high_byte);
				break;
			}
			WAIT_CLAW_STOP(100,200,die);
	   }
	}

//���ճ������
die:	
	tocase_flag = CSF_Finished;
	return tocase_flag;
}

void Claw_Action(u8 onoff)
{
	if(onoff == 0x10)
	{
		JODELL_OPEN_CLOSE_ACTIVETION(0x00,CLAW_POW_CLOSE,0xFF);//��צ��
	}
	else if(onoff == 0x20)
	{
		JODELL_OPEN_CLOSE_ACTIVETION(140,CLAW_POW_HALF,0xFF);//��צ�뿪 150
	}
	else if(onoff == 0x30)
	{
		JODELL_OPEN_CLOSE_ACTIVETION(0xFF,CLAW_POW_FULL,0xFF);//��צȫ��
	}
	else if(onoff == 0x40)
	{
		JODELL_OPEN_CLOSE_ACTIVETION(55,CLAW_POW_MICRO,0xFF);//��צ΢�� 55 to 75 to 65 to 60 to 55
	}
}

