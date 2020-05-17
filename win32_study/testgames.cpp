#include "math.h"
#include <iostream>
#include <stdlib.h>
#include <wtypes.h> 
#include <dbt.h>
#include <windows.h>
#include<conio.h>
#include <time.h> 
#include "Cardgame.h"
#include "wave.h"
using namespace std;
//void clrscr(void);
int Cardgame::totalparticipants = 0;
/*
	��x��y����ϵ�£����������Σ����ķ�ֵ��Χ��-1��1��
	1.scale_y��scale_xΪ��������ϵ�Ĳ���
*/
int init_xy(int x, int y)
{
	int tempx=x;
	int tempy=y;
	//cout<<"1 |\n";
	
	while(tempy)
	{
		if(tempy ==y)
		{
			printf("+1|\n");
			tempy--;
		}
		printf("  |\n");
		tempy--;
		if(tempy ==1)
		{
			printf(" 0|\n");
		/*
		//����x��
		while(tempx)
			{
				printf("_");
				tempx--;
			}
		*/
			tempy = y;
			while(tempy)
			{
				printf("  |\n");
				tempy--;
				if(tempy == 1)
				{
					printf("-1|\n");
					tempy--;
				}
			}

		}
		
	}
	return 1;
}
int print_xy(int *data, int len)
{
	int tempx=SCALE_X;
	int tempy=SCALE_Y;
	
	while(tempy)
	{
		if(tempy ==SCALE_Y)
		{
			printf("+1|");
			for(tempx = 0; tempx < len;tempx++)
			{
				if(tempy == data[tempx])
					printf(POINT_1);
				else printf(" ");
			}
			printf("\n");
			tempy--;
		}
		printf("  |");
		for(tempx = 0; tempx < len;tempx++)
		{
			if(tempy == data[tempx])
				printf(POINT_1);
			else printf(" ");
		}
		printf("\n");
		tempy--;
		if(tempy ==1)
		{
			printf(" 0|");
		/*
		//����x��
		while(SCALE_X)
			{
				printf("_");
				tempx--;
			}
		*/
			
			for(tempx = 0; tempx < len;tempx++)
			{
				if(tempy == data[tempx])
					printf(POINT_1);
				else printf(" ");
				}
			printf("\n");
			tempy = SCALE_Y;
			while(tempy)//�����°���
			{
				printf("  |");
				for(tempx = 0; tempx < len;tempx++)
				{
					if((tempy-SCALE_Y-1) == data[tempx])
						printf(POINT_1);
					else printf(" ");
					}
				printf("\n");
				tempy--;
				if(tempy == 1)
				{
					printf("-1|");
					for(tempx = 0; tempx < len;tempx++)
					{
						if((0-SCALE_Y) == data[tempx])
							printf(POINT_1);
						else printf(" ");
						}
					printf("\n");
					tempy--;
				}
			}

		}
		
	}
	return 1;
}

int main()
{
	Cardgame *brige =0;
	Cardgame *blackjack=0;
	Cardgame *solitaire=0;
	Cardgame *poker =0;
	//if(init_xy(SCALE_X,SCALE_Y));

	//else printf("xy can't init!\n");
	
	double x = 0.0;
	int y =0;
	//int data[SCALE_X];
	int *data = new int[10];
	int tempx = 0;//
	for(tempx = 0;tempx < SCALE_X;tempx++)
			data[tempx]= 0;
	/*
	for(tempx = 0;tempx < SCALE_X;tempx++)
	{
			data[tempx]= (sin(x)*SCALE_Y);
			x+=0.1;
			print_xy(&data,SCALE_X);//��ʾ����
			sleep(500);
	}
	*/
	tempx=0;
	while(1)
	{
		

		if(tempx == SCALE_X)//��������ͳһ����
		{
			
			for(int i =0;i < tempx-1;i++)
				data[i] = data[i+1];
			tempx--;
			
		}
		data[tempx]= (sin(x)*SCALE_Y);
		x+=0.1;
		//clrscr();//����
		print_xy(data,SCALE_X);//��ʾ����
		tempx++;
		system("cls");//����
		//for(;tempx < SCALE_X;temp++)
		
		Sleep(500);
	}
		


	
	return 0;
}

