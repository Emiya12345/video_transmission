#include "image.h"

/*��ʼ������*/
void InitQueue(Queue Q)
{
	int i = 0;
	for(i = 0; i < PictureMaxSize; ++i)
	{
		Q->data[i] = NULL;
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
void EnQueue(data Node, Queue Q)
{
	if(IsFullQ(Q))
		return;
	Q->size++;
	Q->Rear = Scc(Q->Rear);
	Q->data[Q->Rear] = Node;
}
/*����*/
data DeQueue(Queue Q)
{
	data re_val;
	if(IsEmpty(Q))
		return NULL;
	re_val = Q->data[Q->Fornt];
	Q->size--;
	Q->Fornt = Scc(Q->Fornt);
	return re_val;
}


void SendImageToComputer(uint16_t width, uint16_t height)
{
	OS_ERR  err;
//	uint8_t AckData[1] = { 0x01};
//	uint8 buff[1] = {0};

	
	uint16_t i = 0,j = 0,k = 0; 
	uint16_t data_line = 0;
	uint8_t Camera_Data;
	uint8_t data[1280] = {0};
	
	for(i = 0; i < height; i++)
	{
		for(j = 0; j < width; j++)
		{
			READ_FIFO_PIXEL(Camera_Data);		/* ��FIFO����һ��rgb565���صĸ�λ��Camera_Data���� */
			data[k++] = Camera_Data;
			READ_FIFO_PIXEL(Camera_Data);		/* ��FIFO����һ��rgb565���صĵ�λ��Camera_Data���� */
			data[k++] = Camera_Data;
		}
		data_line++;
		if(data_line%2 == 0)
		{			
			sendto(SOCK_UDPS,data, width*4, remote_ip, remote_port);
			k = 0;
		}
		//OSTimeDlyHMSM ( 0, 0, 0, 10, OS_OPT_TIME_DLY, & err );
		if(data_line >= 240)
			data_line = 0;
	}
}

