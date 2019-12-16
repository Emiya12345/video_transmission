#ifndef __IMAGE_H
#define __IMAGE_H

#include "stm32f10x.h"
#include "./ov7725/bsp_ov7725.h"
#include "W5500_conf.h"
#include "socket.h"
#include "stdio.h"
#include "w5500.h"
#include  <os.h>

#define PictureMaxSize	3


struct PictureQueue;
typedef uint8_t *data;
typedef struct PictureQueue *Queue;


extern uint8  remote_ip[4];											/*Զ��IP��ַ*/
extern uint16 remote_port;
extern uint8_t picture_data[3][1280];
extern Queue Q;
extern OS_MEM picture_mem;

/*ͼƬ���ݶ���*/
struct PictureQueue
{
	uint8_t Fornt;
	uint8_t Rear;
	uint8_t size;
};

/*��ʼ������*/
void InitQueue(Queue Q);
/*�ж϶����Ƿ�Ϊ��*/
uint8_t IsFullQ(Queue Q);
/*�ж϶����Ƿ�Ϊ��*/
uint8_t IsEmpty(Queue Q);
/*���*/
uint8 EnQueue(Queue Q);
/*����*/
uint8 DeQueue(Queue Q);


void SendImageToComputer(uint16_t width, uint16_t height);
#endif


