/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2013; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                        Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : EHS
*                 DC
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>



/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

uint8_t transfer_falg = 0;
extern uint8_t Ov7725_vsync;
extern OV7725_MODE_PARAM cam_mode;


/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   AppTaskStartTCB;    //������ƿ�

static  OS_TCB   AppTaskOV7725TCB;
static  OS_TCB   AppTaskRecvieDataTCB;
static  OS_TCB   APPTaskControlSendTCB;


/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];       //�����ջ

static  CPU_STK  AppTaskOV7725Stk [ APP_TASK_OV7725_STK_SIZE ];
static  CPU_STK  AppTaskReceiveDataStk [ APP_TASK_RECVIE_DATA_STK_SIZE ];
static  CPU_STK  APPTaskControlSendStk [ APP_TASK_CONTROL_SEND_SIZE ];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart  (void *p_arg);               //����������

static  void  AppTaskOV7725  ( void * p_arg );
static  void  AppTaskReciveData ( void * p_arg );
static  void  APPTaskControlSend(void * p_arg);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

int  main (void)
{
    OS_ERR  err;

	
    OSInit(&err);                                                           //��ʼ�� uC/OS-III

	  /* ������ʼ���� */
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                            //������ƿ��ַ
                 (CPU_CHAR   *)"App Task Start",                            //��������
                 (OS_TASK_PTR ) AppTaskStart,                               //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) APP_TASK_START_PRIO,                        //��������ȼ�
                 (CPU_STK    *)&AppTaskStartStk[0],                         //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,               //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,                    //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵOSCfg_TickRate_Hz/10��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&err);                                       //���ش�������

    OSStart(&err);                                                          //�����������������uC/OS-III���ƣ�

}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;


    (void)p_arg;

    BSP_Init();                                                 //�弶��ʼ��
    CPU_Init();                                                 //��ʼ�� CPU �����ʱ��������ж�ʱ���������������

    cpu_clk_freq = BSP_CPU_ClkFreq();                           //��ȡ CPU �ں�ʱ��Ƶ�ʣ�SysTick ����ʱ�ӣ�
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        //�����û��趨��ʱ�ӽ���Ƶ�ʼ��� SysTick ��ʱ���ļ���ֵ
    OS_CPU_SysTickInit(cnts);                                   //���� SysTick ��ʼ�����������ö�ʱ������ֵ��������ʱ��

    Mem_Init();                                                 //��ʼ���ڴ������������ڴ�غ��ڴ�ر�

#if OS_CFG_STAT_TASK_EN > 0u                                    //���ʹ�ܣ�Ĭ��ʹ�ܣ���ͳ������
    OSStatTaskCPUUsageInit(&err);                               //����û��Ӧ������ֻ�п�����������ʱ CPU �ģ����
#endif                                                          //���������� OS_Stat_IdleCtrMax ��ֵ��Ϊ������� CPU 
                                                                //ʹ����ʹ�ã���
    CPU_IntDisMeasMaxCurReset();                                //��λ�����㣩��ǰ�����ж�ʱ��

    
    /* ����ʱ��Ƭ��ת���� */		
    OSSchedRoundRobinCfg((CPU_BOOLEAN   )DEF_ENABLED,          //ʹ��ʱ��Ƭ��ת����
		                     (OS_TICK       )0,                    //�� OSCfg_TickRate_Hz / 10 ��ΪĬ��ʱ��Ƭֵ
												 (OS_ERR       *)&err );               //���ش�������


		/* ���� OV7725����ͷ ���� */
    OSTaskCreate((OS_TCB     *)&AppTaskOV7725TCB,                             //������ƿ��ַ
                 (CPU_CHAR   *)"App Task OV7725",                             //��������
                 (OS_TASK_PTR ) AppTaskOV7725,                                //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) APP_TASK_OV7725_PRIO,                         //��������ȼ�
                 (CPU_STK    *)&AppTaskOV7725Stk[0],                          //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) APP_TASK_OV7725_STK_SIZE / 10,                //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) APP_TASK_OV7725_STK_SIZE,                     //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵ��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&err);                                       //���ش�������
#if 0								 								 
	  OSTaskCreate((OS_TCB     *)&AppTaskRecvieDataTCB,                             //������ƿ��ַ
                 (CPU_CHAR   *)"App Task Recvie Data",                             //��������
                 (OS_TASK_PTR ) AppTaskReciveData,                                //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) APP_TASK_RECVIE_DATA_PRIO,                         //��������ȼ�
                 (CPU_STK    *)&AppTaskReceiveDataStk[0],                          //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) APP_TASK_RECVIE_DATA_STK_SIZE / 10,                //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) APP_TASK_RECVIE_DATA_STK_SIZE,                     //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵ��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&err);                                       //���ش�������
#endif
	OSTaskCreate((OS_TCB     *)&APPTaskControlSendTCB,                             //������ƿ��ַ
                 (CPU_CHAR   *)"App Task Control Send",                             //��������
                 (OS_TASK_PTR ) APPTaskControlSend,                                //������
                 (void       *) 0,                                          //���ݸ����������β�p_arg����ʵ��
                 (OS_PRIO     ) APP_TASK_CONTROL_SEND_PRIO,                         //��������ȼ�
                 (CPU_STK    *)&APPTaskControlSendStk[0],                          //�����ջ�Ļ���ַ
                 (CPU_STK_SIZE) APP_TASK_CONTROL_SEND_SIZE / 10,                //�����ջ�ռ�ʣ��1/10ʱ����������
                 (CPU_STK_SIZE) APP_TASK_CONTROL_SEND_SIZE,                     //�����ջ�ռ䣨��λ��sizeof(CPU_STK)��
                 (OS_MSG_QTY  ) 5u,                                         //����ɽ��յ������Ϣ��
                 (OS_TICK     ) 0u,                                         //�����ʱ��Ƭ��������0��Ĭ��ֵ��
                 (void       *) 0,                                          //������չ��0����չ��
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //����ѡ��
                 (OS_ERR     *)&err);                                       //���ش�������

    
		OSTaskDel ( 0, & err );                     //ɾ����ʼ������������������
		
		
}

/*
*********************************************************************************************************
*                                          Control Send TASK
*********************************************************************************************************
*/
static  void  APPTaskControlSend(void * p_arg)
{
	OS_ERR      err;
	uint8_t buff[1] = {0};
	(void)p_arg;
	while(DEF_TRUE)
	{
		while(getSn_SR(SOCK_UDPS) == SOCK_CLOSED)
		{
			socket(SOCK_UDPS,Sn_MR_UDP,local_port,0);
		}

		if(getSn_IR(SOCK_UDPS) & Sn_IR_RECV)
		{
			setSn_IR(SOCK_UDPS, Sn_IR_RECV);                                     /*������ж�*/
			if((getSn_RX_RSR(SOCK_UDPS))>0)                                    /*���յ�����*/
			{
				recvfrom(SOCK_UDPS,buff, 1, remote_ip,&remote_port);               /*W5500���ռ����������������*/
				if(buff[0] == 0x01)
					transfer_falg = 1;
				else if(buff[0] == 0x08)
					transfer_falg = 0;
			}
		}
		OSTimeDlyHMSM ( 0, 0, 0, 5, OS_OPT_TIME_DLY, & err );     //ÿ��500ms����һ��
	}	
}


/*
*********************************************************************************************************
*                                          OV7725 TASK
*********************************************************************************************************
*/
static  void  AppTaskOV7725 ( void * p_arg )
{
	OS_ERR      err;
	uint8_t frame_count = 0;
	uint8_t AckData[1] = { 0x01};
	//CPU_SR_ALLOC();
	uint8_t buff[1] = {0};
	//CPU_SR_ALLOC();
	(void)p_arg;
	
	
	
	
	while (DEF_TRUE) {                                   //�����壬ͨ��д��һ����ѭ��
		if(transfer_falg)
		{
			if( Ov7725_vsync == 2 )
			{
				frame_count++;
				FIFO_PREPARE;  			/*FIFO׼��*/	
				//OS_CRITICAL_ENTER();                              //�����ٽ�Σ����⴮�ڴ�ӡ�����
				//printf ( "\r\n��ʼ����һ֡����\r\n");        		
				
				//OS_CRITICAL_EXIT();  //�˳��ٽ��
						
				
				SendImageToComputer(cam_mode.cam_width,
									cam_mode.cam_height);
				
				//OS_CRITICAL_ENTER();                              //�����ٽ�Σ����⴮�ڴ�ӡ�����

				//printf ( "\r\nһ֡���ݷ������\r\n");        		
				//printf("%d\n\r",frame_count);
				//OS_CRITICAL_EXIT(); 
				Ov7725_vsync = 0;			
				macLED1_TOGGLE();
			}
		}	
		OSTimeDlyHMSM ( 0, 0, 0, 5, OS_OPT_TIME_DLY, & err );     //ÿ��500ms����һ��
	}		
}

/*
*********************************************************************************************************
*                                          Recive Data TASK
*********************************************************************************************************
*/
static  void  AppTaskReciveData ( void * p_arg )
{
	OS_ERR      err;
	uint8_t len = 0;
	uint8_t buff[1] = {0};
	uint8_t AckData[1] = { 0x01};
	(void)p_arg;
	
	while (DEF_TRUE) {                                   //�����壬ͨ��д��һ����ѭ��
		switch(getSn_SR(SOCK_UDPS))                                                /*��ȡsocket��״̬*/
		{
			case SOCK_CLOSED:                                                        /*socket���ڹر�״̬*/
				socket(SOCK_UDPS,Sn_MR_UDP,local_port,0);                              /*��ʼ��socket*/
				break;
			
			case SOCK_UDP:                                                           /*socket��ʼ�����*/
				if(getSn_IR(SOCK_UDPS) & Sn_IR_RECV)
				{
					setSn_IR(SOCK_UDPS, Sn_IR_RECV);                                     /*������ж�*/
				}
				//sendto(SOCK_UDPS,buff1,6, remote_ip, remote_port);
				if((len=getSn_RX_RSR(SOCK_UDPS))>0)                                    /*���յ�����*/
				{
					recvfrom(SOCK_UDPS,buff, len, remote_ip,&remote_port);               /*W5500���ռ����������������*/
					if(buff[0] == 0x0A)                                                    
						printf("up\r\n");
					if(buff[0] == 0x0B)                                                    
						printf("down\r\n");
					if(buff[0] == 0x0C)                                                    
						printf("left\r\n");
					if(buff[0] == 0x0D)                                                    
						printf("right\r\n");
					sendto(SOCK_UDPS,AckData, 1, remote_ip, remote_port);                /*W5500�ѽ��յ������ݷ��͸�Remote*/
				}
				break;
		}	
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );                   //ÿ20msɨ��һ��
	}
	
}





