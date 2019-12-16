#include "image.h"

/*��ʼ������*/
void InitQueue(Queue Q)
{
	if(Q == NULL)
	{
		printf("Q is NULL\n");
		return;
	}
	Q->Fornt = 1;
	Q->Rear = 0;
	Q->size = 0;
}

/*�ж϶����Ƿ�Ϊ��*/
uint8_t IsEmpty(Queue Q)
{
	return Q->size == 0;
}
/*�ж϶����Ƿ�Ϊ��*/
uint8_t IsFullQ(Queue Q)
{
	return Q->size >= PictureMaxSize;
}

static int Scc(int value)
{
	if (++value == PictureMaxSize)
		value = 0;
	return value;
}
/*���*/
uint8 EnQueue(Queue Q)
{
	if(IsFullQ(Q))
		return 0;
	Q->size++;
	Q->Rear = Scc(Q->Rear);
	return Q->Rear;
}
/*����*/
uint8 DeQueue(Queue Q)
{
	if(IsEmpty(Q))
		return 0;
	Q->size--;
	Q->Fornt = Scc(Q->Fornt);
	return Q->Fornt;
}

void SendImageToComputer(uint16_t width, uint16_t height)
{
	OS_ERR  err;
	CPU_SR_ALLOC();
	
	uint16_t i = 0,j = 0,k = 0; 
	uint16_t data_line = 0;
	uint8 temp_Q = 0;
	uint8_t Camera_Data;
	//uint8_t data[1280] = {0};
	
	for(i = 0; i < height; i++)
	{
		if(data_line%2 == 0)
		{	
			while(Q->size >= 3)
			{
				OSTimeDlyHMSM ( 0, 0, 0, 1, OS_OPT_TIME_DLY, & err );
				continue;
			}		
			
			temp_Q = EnQueue(Q);
			OS_CRITICAL_ENTER();                              //�����ٽ�Σ����⴮�ڴ�ӡ�����
			printf ( "\r\nд��һ֡����\r\n");        		
				
			OS_CRITICAL_EXIT();  //�˳��ٽ��
			k = 0;
		}
		for(j = 0; j < width; j++)
		{
			READ_FIFO_PIXEL(Camera_Data);		/* ��FIFO����һ��rgb565���صĸ�λ��Camera_Data���� */
			picture_data[temp_Q][k++] = Camera_Data;
			READ_FIFO_PIXEL(Camera_Data);		/* ��FIFO����һ��rgb565���صĵ�λ��Camera_Data���� */
			picture_data[temp_Q][k++] = Camera_Data;
		}
		data_line++;
		if(data_line >= 240)
			data_line = 0;
		//OSTimeDlyHMSM ( 0, 0, 0, 10, OS_OPT_TIME_DLY, & err );
	}
	
	//EnQueue(line_data, Q);
}

