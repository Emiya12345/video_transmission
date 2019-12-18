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


