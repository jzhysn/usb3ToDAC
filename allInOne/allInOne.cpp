// allInOne.cpp : 定义 DLL 应用程序的导出函数。
//

/*
为了使USB数据能够不间断的输出，应一次获取多组通道数据，作为一次传输的数据。并建立队列缓存输出，从而使整个系统保持同步。
1 获取一组通道数据，经过分组和转置后以字节形式放于缓存buffer地址0到48*4-1中，buffer大小为ppx，ppx根据USB的包的最大值有关，
maxPktSize*n/48*4 ==0时，取ppx = maxPktSize*n*10，如包的最大值为1024，ppx = 30；
2 重复第一步直到buffer填满
3 重复1、2步，直到所有队列元素填满，
4 从队头开始传输数据，对头传输完后补充数据到队头，当再次循环到对头时  
*/
//

#include "stdafx.h"
#include "allInOne.h"
#include "CyAPI.h"
#include <wtypes.h> 
#include <dbt.h>
#include <windows.h>



static int TimeOut = TIMEOUT;
CCyUSBDevice *USBDevice;
	
static CCyUSBEndPoint *EndPt;
	
static UCHAR eptAddr = BULK_OUT_ADDR;

//Thread	^XferThread;
/*
HANDLECreateThread(
LPSECURITY_ATTRIBUTES lpThreadAttributes,//线程安全属性
DWORD dwStackSize,//堆栈大小
LPTHREAD_START_ROUTINE lpStartAddress,//线程函数
LPVOID lpParameter,//线程参数
DWORD dwCreationFlags,//线程创建属性
LPDWORD lpThreadId//线程ID
);
*/

//HANDLE XferThread;
UCHAR isXfer = 0;
unsigned char chCount = 0;
int *chDataSourceGroup;
static bool bHighSpeedDevice;
static bool bSuperSpeedDevice;
int *inArrayData;
unsigned char chC;

DWORD WINAPI ThreadProFunc(LPVOID lpParam);



int GetUSBDevice(int *inArray,unsigned char channelsCount )//获取并打开设备，返回设备数
		{
			
			HANDLE hThread;
			DWORD dwThreadId;
			
			inArrayData = inArray;
			chC = channelsCount;
			int n = 0;
			USBDevice = new CCyUSBDevice(0,CYUSBDRV_GUID,true);
			if(USBDevice ==NULL) return 0;
		
			n = USBDevice->DeviceCount();
			//printf("deviceCount = %d \n",);
			//打开USB设备
			for(int i = 0; i < n; i++)
			{
				USBDevice->Open(i);
			
				              
			}

			//默认打开第一个设备
			 if (n > 0 ) 
			 {               
                USBDevice->Open(0);
             }

			 int interfaces = USBDevice->AltIntfcCount()+1;//备用接口+1
			 bHighSpeedDevice = USBDevice-> bHighSpeed;
			 bSuperSpeedDevice = USBDevice->bSuperSpeed;
			 
				//设置端口
			 EndPt = USBDevice->EndPointOf(eptAddr);


			 EndPt = USBDevice->BulkOutEndPt;
			

			hThread = CreateThread(NULL,//默认安全属性
								  NULL,//默认堆栈大小
								  ThreadProFunc,//线程入口地址
								  NULL,//传递给线程函数的参数
								  0,//指定线程立即运行
								  &dwThreadId//线程ID
								);
			CloseHandle(hThread);//关闭线程句柄


			return n;		  

			 }
	
DWORD WINAPI ThreadProFunc(LPVOID lpParam)
{
	getChannels(EndPt,inArrayData,chC);
	return 0; 

}


/*
	通过输入一组数据开始进行DAC输出。
	CCyUSBEndPoint *outEndPt：输出端点指针。
	int *channelDataSourceGroup：输入数据的指针，数据为无符号32位整型；
	unsigned char channelsCount：需要启动的通道数，一次最多可以启动64个通道数输出；	
	unsigned char queueSize： 缓存队列数，每一个队列元素，存储了所有64个通道ppx个字节的数据，其中ppx>=10。
	return status:返回状态，包括：
	0：成功退出
	1：通道数为0
	2：Xfer request rejected
	3：FinishDataXfer失败
	4: 循环开始后，即队列更新元素后，BeginDataXfer失败

	*/
//!!注意，每次传输的大小应小于4m bytes
	 unsigned char XferChData(CCyUSBEndPoint *EndPt,  unsigned char queueSize)
	{
		
		PUCHAR *buffers = new PUCHAR[queueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[queueSize];
		PUCHAR *contexts = new PUCHAR[queueSize];
		OVERLAPPED *inOvLap = new OVERLAPPED[queueSize];

		unsigned char status = 0;
		UCHAR bitSize = 24;
		//UCHAR queue = queueSize;
		UINT xferSize = 0;//暂时设为定值
		int ppx = 1;
		USHORT PPX = 64*3;//每次传输的包大小
		
		//通道缓存
		int *outGroup = new int[64];
		int *oddGroup = new int[32];
		int *evenGroup = new int[32];

		UCHAR q = 0;
			
		
		
		//while((EndPt->MaxPktSize*ppx)%192 !=0) ppx++;
			//ppx = 10*ppx;



		//unsigned char groupDataTransChar[24*8] = {0};

		if(chCount == 0) return 1;

		long len = EndPt->MaxPktSize*PPX;
		EndPt->SetXferSize(len);



		//填满队列
		for(q = 0; q < queueSize; q++)
		{
			buffers[q] = new UCHAR[len];
			isoPktInfos[q] = new CCyIsoPktInfo[PPX];
			inOvLap[q].hEvent = CreateEvent(NULL, false, false, NULL);
			xferSize = 0;

			while(xferSize < len)//填满一次发送的数据
			{
				memset(oddGroup,0x00,32*4);//设为0
				memset(evenGroup,0x00,32*4);//设为0
				//将数据传入并分奇偶两组
				for( UCHAR i = 0; i < chCount; i++)
				{
					if(i%2 == 0)
						oddGroup[i/2] = chDataSourceGroup[i];
					else
						evenGroup[i/2] = chDataSourceGroup[i];
				}

				//转置
				transpositionInt(oddGroup, outGroup, DAC_32BIT);
				transpositionInt(evenGroup, outGroup+32, DAC_32BIT);
				//  24位按字节填装，取高24位
				for(UCHAR i = 0; i < 64; i++)
				{
					//小端,取高24位
					
					if(i < 24)
					memcpy(&(buffers[q][i*4+xferSize]),&(outGroup[i]),4);
					if((i > 31)&&(i < 56))
					memcpy(&(buffers[q][(i-8)*4+xferSize]),&(outGroup[i]),4);
					/*
					//小端,取低24位
					if(（i > 7) && (i < 32))
					memcpy(&(buffers[q][(i-8)*4+xferSize]),&(outGroup[i]),4);
					if((i > 39) && (i < 64))
					memcpy(&(buffers[q][(i-16)*4+xferSize]),&(outGroup[i]),4);
					*/
				}
				xferSize += 192;
			}
		}
		//第一次发送
		 for (q = 0; q < queueSize; q++)	
            {
                contexts[q] = EndPt->BeginDataXfer(buffers[q], len, &inOvLap[q]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
					return 2;
            }

		 q = 0;

		 while(isXfer)
		 {
			 long rLen = len;	// Reset this each time through because
                // FinishDataXfer may modify it

                if (!EndPt->WaitForXfer(&inOvLap[q], TimeOut))
                {
                    EndPt->Abort();
                    if (EndPt->LastError == ERROR_IO_PENDING)
                        WaitForSingleObject(inOvLap[q].hEvent,2000);
                }
				
			
                if (EndPt->Attributes == 1) // ISOC Endpoint
                {	
						/*
                    if (EndPt->FinishDataXfer(buffers[q], rLen, &inOvLap[q], contexts[q], isoPktInfos[q])) 
                    {			
                        CCyIsoPktInfo *pkts = isoPktInfos[q];
                        for (int j=0; j< PPX; j++) 
                        {
							if ((pkts[j].Status == 0) && (pkts[j].Length<=EndPt->MaxPktSize)) 
                            {
                                BytesXferred += pkts[j].Length;

                                if (bShowData)
                                    Display16Bytes(buffers[q]);

                               
                            }
                            else
                                Failures++;

                            pkts[j].Length = 0;	// Reset to zero for re-use.
							pkts[j].Status = 0;
                        }

                    } 
                    else
                        Failures++; 
						*/

                } 
				

                else // BULK Endpoint
                {
				
                    if (!(EndPt->FinishDataXfer(buffers[q], rLen, &inOvLap[q], contexts[q]))) 
                     return 3;
                }


                // Re-submit this queue element to keep the queue full
				xferSize = 0;
				while(xferSize < len)//填满一次发送的数据
				{
					memset(oddGroup,0x00,32*4);//设为0
					memset(evenGroup,0x00,32*4);//设为0
					//将数据传入并分奇偶两组
					for( UCHAR i = 0; i < chCount; i++)
					{
						if(i%2 == 0)
							oddGroup[i/2] = chDataSourceGroup[i];
						else
							evenGroup[i/2] = chDataSourceGroup[i];
					}

					//转置
					transpositionInt(oddGroup, outGroup, DAC_32BIT);
					transpositionInt(evenGroup, outGroup+32, DAC_32BIT);
					//  24位按字节填装，取高24位
					for(UCHAR i = 0; i < 64; i++)
					{
						//小端,取高24位
						if(i < 24)
						memcpy(&(buffers[q][i*4+xferSize]),&(outGroup[i]),4);
						if((i > 31)&&(i < 56))
						memcpy(&(buffers[q][(i-8)*4+xferSize]),&(outGroup[i]),4);			
					}
					xferSize += 192;
				}
                contexts[q] = EndPt->BeginDataXfer(buffers[q], len, &inOvLap[q]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                    //AbortXferLoop(QueueSize,buffers,isoPktInfos,contexts,inOvLap);
                    return 4;
                }

                q++;

                if (q == queueSize) //
                   q=0;
		 }
		// AbortXferLoop(QueueSize,buffers,isoPktInfos,contexts,inOvLap);
		 return 0;




	 }


	//转置方法
	//参数分别为输入数组、输出数组、有效位数（DAC位数,本工程为24bit）
	//函数为固定的32位转置，后期改进
	int transpositionInt(int in[32],  int out[32],char bitWide)
{
	unsigned int padBit = 0x01 << (bitWide -1);//有效位的最高位取1
	for(int i = 0; i < 32; i++)//32位无符号整型的转置输出，即32*4字节。
		{
			out[i] = 0;
			//第i个整型数的输出是32个数左移i位后（如都移至最高位）与某位相与后，再依次向右移动0-31位并相或的结果。即out_1 = data_1_bit&data_2_biit&...data_32_bit.其中，bit是某位代指。
			for(int j = 0; j < 32; j++)
			{
				out[i] |= (((in[j] << i ) & padBit) >> j);

		    }			
		}
	return 0;

}

	unsigned char getChannels(CCyUSBEndPoint *EndPt,int *channelDataSourceGroup, unsigned char channelsCount)
	{
		chCount = channelsCount;
		chDataSourceGroup = channelDataSourceGroup;
		
		isXfer =1;
	//	XferThread = CreateThread(NULL, 0, XferChData, NULL, 0, NULL);
	//	XferThread = gcnew Thread(gcnew ThreadStart(&XferChData));
		//清空重置端点
		EndPt->Abort();
		EndPt->Reset();
		XferChData(EndPt, QUEUE_SIZE);//
		return 0;

	}
		
			
			
		
		
	


