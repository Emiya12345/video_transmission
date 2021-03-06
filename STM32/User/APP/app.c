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

OS_FLAG_GRP	watchDogFlag;    //声明一个事件标志组
#define OV7725_EVENT	(0x01 << 0)
#define SENDPICTURE_EVENT	(0x01 << 1)
#define RECIVEDATA_EVENT	(0x01 << 2)
#define CONTROLSEND_EVENT	(0x01 << 3)

uint8_t transfer_falg = 0;
extern uint8_t Ov7725_vsync;
extern OV7725_MODE_PARAM cam_mode;
/*图片缓存内存管理对象*/
//OS_MEM picture_mem;
uint8_t picture_data[PictureMaxSize][1280];
struct PictureQueue temp_Q = {
								1,
								0,
								0};
Queue Q = &temp_Q;

/*
*********************************************************************************************************
*                                                 TCB
*********************************************************************************************************
*/

static  OS_TCB   AppTaskStartTCB;    //任务控制块

static  OS_TCB	 AppTaskWatchDogTCB;
static  OS_TCB   AppTaskOV7725TCB;
static  OS_TCB   AppTaskSendPictureTCB;
static  OS_TCB   AppTaskRecvieDataTCB;
static  OS_TCB   APPTaskControlSendTCB;


/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];       //任务堆栈 

static  CPU_STK  AppTakWatchDogSTK[APP_TASK_WATCHDOG_STK_SIZE];
static  CPU_STK  AppTaskOV7725Stk [ APP_TASK_OV7725_STK_SIZE ];
static  CPU_STK  AppTaskSendPictureSTK[APP_TASK_SEND_PICTURE_STK_SIZE];
static  CPU_STK  AppTaskReceiveDataStk [ APP_TASK_RECVIE_DATA_STK_SIZE ];
static  CPU_STK  APPTaskControlSendStk [ APP_TASK_CONTROL_SEND_SIZE ];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskStart  (void *p_arg);               //任务函数声明

static  void  AppTaskWatchDog(void *p_arg);
static  void  AppTaskOV7725  ( void * p_arg );
static  void  AppTaskSendPicture(void *p_arg);
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

	
    OSInit(&err);                                                           //初始化 uC/OS-III

	  /* 创建起始任务 */
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                            //任务控制块地址
                 (CPU_CHAR   *)"App Task Start",                            //任务名称
                 (OS_TASK_PTR ) AppTaskStart,                               //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) APP_TASK_START_PRIO,                        //任务的优先级
                 (CPU_STK    *)&AppTaskStartStk[0],                         //任务堆栈的基地址
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,               //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,                    //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值OSCfg_TickRate_Hz/10）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&err);                                       //返回错误类型

    OSStart(&err);                                                          //启动多任务管理（交由uC/OS-III控制）

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

    BSP_Init();                                                 //板级初始化
    CPU_Init();                                                 //初始化 CPU 组件（时间戳、关中断时间测量和主机名）

    cpu_clk_freq = BSP_CPU_ClkFreq();                           //获取 CPU 内核时钟频率（SysTick 工作时钟）
    cnts = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        //根据用户设定的时钟节拍频率计算 SysTick 定时器的计数值
    OS_CPU_SysTickInit(cnts);                                   //调用 SysTick 初始化函数，设置定时器计数值和启动定时器

    Mem_Init();                                                 //初始化内存管理组件（堆内存池和内存池表）

#if OS_CFG_STAT_TASK_EN > 0u                                    //如果使能（默认使能）了统计任务
    OSStatTaskCPUUsageInit(&err);                               //计算没有应用任务（只有空闲任务）运行时 CPU 的（最大）
#endif                                                          //容量（决定 OS_Stat_IdleCtrMax 的值，为后面计算 CPU 
                                                                //使用率使用）。
    CPU_IntDisMeasMaxCurReset();                                //复位（清零）当前最大关中断时间

    
    /* 配置时间片轮转调度 */		
    OSSchedRoundRobinCfg((CPU_BOOLEAN   )DEF_ENABLED,          //使能时间片轮转调度
		                 (OS_TICK       )0,                    //把 OSCfg_TickRate_Hz / 10 设为默认时间片值
						 (OS_ERR       *)&err );               //返回错误类型


	//创建事件标志组
	OSFlagCreate((OS_FLAG_GRP  *)&watchDogFlag,  //事件标志组指针
                 (CPU_CHAR     *)"flag for watchdog", //命名事件标志组
                 (OS_FLAGS      )0,  //标志初始值
                 (OS_ERR       *)&err);

	/*创建图片缓存管理对象*/
	// OSMemCreate((OS_MEM       *)&picture_mem,    //内存分区控制块
    //             (CPU_CHAR     *)"picture_mem",   //命名内存分区
    //             (void         *)picture_data,   //内存分区首地址
    //             (OS_MEM_QTY    )3,   //内存块数目
    //             (OS_MEM_SIZE   )1280, //内存块大小（单位：字节）
    //             (OS_ERR       *)&err);
				
	/*创建队列*/
	InitQueue(Q);

	OSTaskCreate((OS_TCB     *)&AppTaskWatchDogTCB,                             //任务控制块地址
						(CPU_CHAR   *)"App Task WatchDog",                             //任务名称
						(OS_TASK_PTR ) AppTaskWatchDog,                                //任务函数
						(void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
						(OS_PRIO     ) APP_TASK_WATCHDOG_PRIO,                         //任务的优先级
						(CPU_STK    *)&AppTakWatchDogSTK[0],                          //任务堆栈的基地址
						(CPU_STK_SIZE) APP_TASK_WATCHDOG_STK_SIZE / 10,                //任务堆栈空间剩下1/10时限制其增长
						(CPU_STK_SIZE) APP_TASK_WATCHDOG_STK_SIZE,                     //任务堆栈空间（单位：sizeof(CPU_STK)）
						(OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
						(OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值）
						(void       *) 0,                                          //任务扩展（0表不扩展）
						(OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
						(OS_ERR     *)&err);

		/* 创建 OV7725摄像头 任务 */
    OSTaskCreate((OS_TCB     *)&AppTaskOV7725TCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task OV7725",                             //任务名称
                 (OS_TASK_PTR ) AppTaskOV7725,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) APP_TASK_OV7725_PRIO,                         //任务的优先级
                 (CPU_STK    *)&AppTaskOV7725Stk[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) APP_TASK_OV7725_STK_SIZE / 10,                //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) APP_TASK_OV7725_STK_SIZE,                     //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&err);                                       //返回错误类型

	OSTaskCreate((OS_TCB     *)&AppTaskRecvieDataTCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task Recvie Data",                             //任务名称
                 (OS_TASK_PTR ) AppTaskReciveData,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) APP_TASK_RECVIE_DATA_PRIO,                         //任务的优先级
                 (CPU_STK    *)&AppTaskReceiveDataStk[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) APP_TASK_RECVIE_DATA_STK_SIZE / 10,                //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) APP_TASK_RECVIE_DATA_STK_SIZE,                     //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&err);                                       //返回错误类型

	OSTaskCreate((OS_TCB     *)&APPTaskControlSendTCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task Control Send",                             //任务名称
                 (OS_TASK_PTR ) APPTaskControlSend,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) APP_TASK_CONTROL_SEND_PRIO,                         //任务的优先级
                 (CPU_STK    *)&APPTaskControlSendStk[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) APP_TASK_CONTROL_SEND_SIZE / 10,                //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) APP_TASK_CONTROL_SEND_SIZE,                     //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&err);                                       //返回错误类型

    OSTaskCreate((OS_TCB     *)&AppTaskSendPictureTCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task Send Picture",                             //任务名称
                 (OS_TASK_PTR ) AppTaskSendPicture,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) APP_TASK_SEND_PICTURE_PRIO,                         //任务的优先级
                 (CPU_STK    *)&AppTaskSendPictureSTK[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) APP_TASK_SEND_PICTURE_STK_SIZE / 10,                //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) APP_TASK_SEND_PICTURE_STK_SIZE,                     //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&err);

	

	IWDG_Config(IWDG_Prescaler_64, 3500);

	OSTaskDel ( 0, & err );                     //删除起始任务本身，该任务不再运行
		
		
}

/*
*********************************************************************************************************
*                                          WatchDog TASK
*********************************************************************************************************
*/
static  void  AppTaskWatchDog(void *p_arg)
{
	OS_ERR err;
	OS_FLAGS	flag_rdy;

	while(DEF_TRUE)
	{
		flag_rdy = OSFlagPend((OS_FLAG_GRP  *)&watchDogFlag,   //事件标志组指针
                      		  (OS_FLAGS      )(OV7725_EVENT | SENDPICTURE_EVENT | RECIVEDATA_EVENT | CONTROLSEND_EVENT),   //选定要操作的标志位
                      		  (OS_TICK       )0, //等待期限（单位：时钟节拍）
                              (OS_OPT        )(OS_OPT_PEND_FLAG_SET_ALL | OS_OPT_PEND_FLAG_CONSUME | OS_OPT_PEND_BLOCKING),     //选项
                              (CPU_TS       *)0,    //返回等到事件标志时的时间戳
                              (OS_ERR       *)&err);   //返回错误类型

		if(flag_rdy & (OV7725_EVENT | SENDPICTURE_EVENT | RECIVEDATA_EVENT | CONTROLSEND_EVENT) == (OV7725_EVENT | SENDPICTURE_EVENT | RECIVEDATA_EVENT | CONTROLSEND_EVENT))
		{
			//喂狗
			IWDG_Feed();
		}
		OSTimeDlyHMSM ( 0, 0, 2, 0, OS_OPT_TIME_DLY, & err );
	}
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
		//抛出事件
		OSFlagPost((OS_FLAG_GRP  *)&watchDogFlag, //事件标志组指针
                   (OS_FLAGS      )CONTROLSEND_EVENT, //选定要操作的标志位
                   (OS_OPT        )OS_OPT_POST_FLAG_SET,   //选项
                   (OS_ERR       *)&err); //返回错误类型;
		
		while(getSn_SR(SOCK_UDPS) == SOCK_CLOSED)
		{
			socket(SOCK_UDPS,Sn_MR_UDP,local_port,0);
		}

		if(getSn_IR(SOCK_UDPS) & Sn_IR_RECV)
		{
			setSn_IR(SOCK_UDPS, Sn_IR_RECV);                                     /*清接收中断*/
			if((getSn_RX_RSR(SOCK_UDPS))>0)                                    /*接收到数据*/
			{
				recvfrom(SOCK_UDPS,buff, 1, remote_ip,&remote_port);               /*W5500接收计算机发送来的数据*/
				if(buff[0] == 0x01)
					transfer_falg = 1;
				else if(buff[0] == 0x02)
					SystemReset();
				else if(buff[0] == 0x08)
					transfer_falg = 0;
			}
		}
		OSTimeDlyHMSM ( 0, 0, 0, 5, OS_OPT_TIME_DLY, & err );     //每隔500ms发送一次
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
	uint16_t i = 0,j = 0,k = 0; 
	uint16_t data_line = 0;
	uint8 temp_Q = 0;
	uint8_t Camera_Data;

	//CPU_SR_ALLOC();
	(void)p_arg;
	
	while (DEF_TRUE) {                                   //任务体，通常写成一个死循环
		//抛出事件
		OSFlagPost((OS_FLAG_GRP  *)&watchDogFlag, //事件标志组指针
                   (OS_FLAGS      )OV7725_EVENT, //选定要操作的标志位
                   (OS_OPT        )OS_OPT_POST_FLAG_SET,   //选项
                   (OS_ERR       *)&err); //返回错误类型;

		if(transfer_falg)
		{
			if( Ov7725_vsync == 2 )
			{
				FIFO_PREPARE;  			/*FIFO准备*/	
				for (data_line = 0; data_line < cam_mode.cam_height; ) //行数小于画面高度，一直等待
				{
					if(Q->size < PictureMaxSize)	//如果队列未满
					{
						if(data_line%2 == 0)
						{			
							temp_Q = EnQueue(Q);
							i = 0;
							k = 0;
						}
						while (i < 2)  //保证读2行
						{
							for(j = 0; j < cam_mode.cam_width; j++)
							{
								READ_FIFO_PIXEL(Camera_Data);		/* 从FIFO读出一个rgb565像素的高位到Camera_Data变量 */
								picture_data[temp_Q][k++] = Camera_Data;
								READ_FIFO_PIXEL(Camera_Data);		/* 从FIFO读出一个rgb565像素的低位到Camera_Data变量 */
								picture_data[temp_Q][k++] = Camera_Data;
							}
							i++;
						}
						data_line += 2;
						//OS_CRITICAL_ENTER(); //进入临界段，避免串口打印被打断
						//printf ( "\r\n1\r\n");        		
						//OS_CRITICAL_EXIT();  //退出临界段
					}
					OSTimeDlyHMSM ( 0, 0, 0, 1, OS_OPT_TIME_DLY, & err );
				}
				
				Ov7725_vsync = 0;			
				macLED1_TOGGLE();
			}
		}
		OSTimeDlyHMSM ( 0, 0, 0, 5, OS_OPT_TIME_DLY, & err );	
	}		
}


/*
*********************************************************************************************************
*                                          Send Picture Data TASK
*********************************************************************************************************
*/
static  void  AppTaskSendPicture(void *p_arg)
{
	OS_ERR err;
	uint8 DeQueue_Temp = 0;
	//CPU_SR_ALLOC();

	while(DEF_TRUE)
	{
		//抛出事件
		OSFlagPost((OS_FLAG_GRP  *)&watchDogFlag, //事件标志组指针
                   (OS_FLAGS      )SENDPICTURE_EVENT, //选定要操作的标志位
                   (OS_OPT        )OS_OPT_POST_FLAG_SET,   //选项
                   (OS_ERR       *)&err); //返回错误类型;
		//if (transfer_falg)
		// {
			if(Q->size > 0)
			{
				DeQueue_Temp = DeQueue(Q);
				switch(getSn_SR(SOCK_UDPS))                                                /*获取socket的状态*/
				{
					case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
					{
						socket(SOCK_UDPS2,Sn_MR_UDP,local_port,0);                              /*初始化socket*/
						break;
					}
					case SOCK_UDP:
					{
						sendto(SOCK_UDPS,picture_data[DeQueue_Temp], 1280, remote_ip, remote_port);
						break;
					}
				}
				// OS_CRITICAL_ENTER();                              //进入临界段，避免串口打印被打断
				// printf ( "\r\n2\r\n");        		
				// OS_CRITICAL_EXIT();  //退出临界
			}
		// }
		
		
		
		OSTimeDlyHMSM ( 0, 0, 0, 1, OS_OPT_TIME_DLY, & err );
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
	uint16 local_port2 = 7000;                              /*定义本地端口UDP2*/
	uint8_t buff[1] = {0};
	(void)p_arg;
	
	while (DEF_TRUE) {                                   //任务体，通常写成一个死循环
		//抛出事件
		OSFlagPost((OS_FLAG_GRP  *)&watchDogFlag, //事件标志组指针
                   (OS_FLAGS      )RECIVEDATA_EVENT, //选定要操作的标志位
                   (OS_OPT        )OS_OPT_POST_FLAG_SET,   //选项
                   (OS_ERR       *)&err); //返回错误类型;

		switch(getSn_SR(SOCK_UDPS2))                                                /*获取socket的状态*/
		{
			case SOCK_CLOSED:                                                        /*socket处于关闭状态*/
				socket(SOCK_UDPS2,Sn_MR_UDP,local_port2,0);                              /*初始化socket*/
				break;
			
			case SOCK_UDP:                                                           /*socket初始化完成*/
				if(getSn_IR(SOCK_UDPS2) & Sn_IR_RECV)
				{
					setSn_IR(SOCK_UDPS2, Sn_IR_RECV);                                     /*清接收中断*/
				}
				//sendto(SOCK_UDPS2,buff1,6, remote_ip, remote_port);
				if((len=getSn_RX_RSR(SOCK_UDPS2))>0)                                    /*接收到数据*/
				{
					recvfrom(SOCK_UDPS2,buff, len, remote_ip,&remote_port2);               /*W5500接收计算机发送来的数据*/
					if(buff[0] == 0x0A)                                                    
						printf("u");
					if(buff[0] == 0x0B)                                                    
						printf("d");
					if(buff[0] == 0x0C)                                                    
						printf("l");
					if(buff[0] == 0x0D)                                                    
						printf("r");
					if(buff[0] == 0x0E)                                                    
						printf("s");
					//sendto(SOCK_UDPS,AckData, 1, remote_ip, remote_port);                /*W5500把接收到的数据发送给Remote*/
				}
				break;
		}	
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );                   //每20ms扫描一次
	}
	
}





