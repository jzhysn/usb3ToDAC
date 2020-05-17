// XferData.cpp : ���� DLL Ӧ�ó���ĵ���������
/*
Ϊ��ʹUSB�����ܹ�����ϵ������Ӧһ�λ�ȡ����ͨ�����ݣ���Ϊһ�δ�������ݡ����������л���������Ӷ�ʹ����ϵͳ����ͬ����
1 ��ȡһ��ͨ�����ݣ����������ת�ú����ֽ���ʽ���ڻ���buffer��ַ0��48*4-1�У�buffer��СΪppx��ppx����USB�İ������ֵ�йأ�
maxPktSize*n/48*4 ==0ʱ��ȡppx = maxPktSize*n*10����������ֵΪ1024��ppx = 30��
2 �ظ���һ��ֱ��buffer����
3 �ظ�1��2����ֱ�����ж���Ԫ��������
4 �Ӷ�ͷ��ʼ�������ݣ���ͷ������󲹳����ݵ���ͷ�����ٴ�ѭ������ͷʱ  
*/
//

#include "stdafx.h"
#include "XferData.h"
#include "CyAPI.h"
#include <wtypes.h> 
#include <dbt.h>
#include <Windows.h>


static int TimeOut = TIMEOUT;

//Thread	^XferThread;
/*
HANDLECreateThread(
LPSECURITY_ATTRIBUTES lpThreadAttributes,//�̰߳�ȫ����
DWORD dwStackSize,//��ջ��С
LPTHREAD_START_ROUTINE lpStartAddress,//�̺߳���
LPVOID lpParameter,//�̲߳���
DWORD dwCreationFlags,//�̴߳�������
LPDWORD lpThreadId//�߳�ID
);
*/

//HANDLE XferThread;
UCHAR isXfer = 0;
unsigned char chCount = 0;
unsigned int *chDataSourceGroup;



	



/*
	ͨ������һ�����ݿ�ʼ����DAC�����
	CCyUSBEndPoint *outEndPt������˵�ָ�롣
	unsigned int *channelDataSourceGroup���������ݵ�ָ�룬����Ϊ�޷���32λ���ͣ�
	unsigned char channelsCount����Ҫ������ͨ������һ������������64��ͨ���������	
	unsigned char queueSize�� �����������ÿһ������Ԫ�أ��洢������64��ͨ��ppx���ֽڵ����ݣ�����ppx>=10��
	return status:����״̬��������
	0���ɹ��˳�
	1��ͨ����Ϊ0
	2��Xfer request rejected
	3��FinishDataXferʧ��
	4: ѭ����ʼ�󣬼����и���Ԫ�غ�BeginDataXferʧ��

	*/
//!!ע�⣬ÿ�δ���Ĵ�СӦС��4m bytes
	 unsigned char XferChData(CCyUSBEndPoint *EndPt, const unsigned char queueSize)
	{
		
		PUCHAR *buffers = new PUCHAR[queueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[queueSize];
		PUCHAR *contexts = new PUCHAR[queueSize];
		OVERLAPPED *inOvLap = new OVERLAPPED[queueSize];

		unsigned char status = 0;
		UCHAR bitSize = 24;
		//UCHAR queue = queueSize;
		USHORT xferSize = 0;//��ʱ��Ϊ��ֵ
		int ppx = 1;
		USHORT PPX = 30;//ÿ�δ���İ���С
		
		//ͨ������
		UINT *outGroup = new UINT[64];
		UINT *oddGroup = new UINT[32];
		UINT *evenGroup = new UINT[32];

		UCHAR q = 0;
			
		
		
		//while((EndPt->MaxPktSize*ppx)%192 !=0) ppx++;
			//ppx = 10*ppx;



		//unsigned char groupDataTransChar[24*8] = {0};

		if(chCount == 0) return 1;

		long len = EndPt->MaxPktSize*PPX;
		EndPt->SetXferSize(len);



		//��������
		for(q = 0; q < queueSize; q++)
		{
			buffers[q] = new UCHAR[len];
			isoPktInfos[q] = new CCyIsoPktInfo[PPX];
			inOvLap[q].hEvent = CreateEvent(NULL, false, false, NULL);
			xferSize = 0;

			while(xferSize < len)//����һ�η��͵�����
			{
				memset(oddGroup,0x00,32*4);//��Ϊ0
				memset(evenGroup,0x00,32*4);//��Ϊ0
				//�����ݴ��벢����ż����
				for( UCHAR i = 0; i < chCount; i++)
				{
					if(i%2 == 0)
						oddGroup[i/2] = chDataSourceGroup[i];
					else
						evenGroup[i/2] = chDataSourceGroup[i];
				}

				//ת��
				transpositionInt(oddGroup, outGroup, DAC_32BIT);
				transpositionInt(evenGroup, outGroup+32, DAC_32BIT);
				//  24λ���ֽ���װ��ȡ��24λ
				for(UCHAR i = 0; i < 64; i++)
				{
					//С��,ȡ��24λ
					
					if(i < 24)
					memcpy(&(buffers[q][i*4+xferSize]),&(outGroup[i]),4);
					if((i > 31)&&(i < 56))
					memcpy(&(buffers[q][(i-8)*4+xferSize]),&(outGroup[i]),4);
					/*
					//С��,ȡ��24λ
					if(��i > 7) && (i < 32))
					memcpy(&(buffers[q][(i-8)*4+xferSize]),&(outGroup[i]),4);
					if((i > 39) && (i < 64))
					memcpy(&(buffers[q][(i-16)*4+xferSize]),&(outGroup[i]),4);
					*/
				}
				xferSize += 192;
			}
		}
		//��һ�η���
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
				while(xferSize < len)//����һ�η��͵�����
				{
					memset(oddGroup,0x00,32*4);//��Ϊ0
					memset(evenGroup,0x00,32*4);//��Ϊ0
					//�����ݴ��벢����ż����
					for( UCHAR i = 0; i < chCount; i++)
					{
						if(i%2 == 0)
							oddGroup[i/2] = chDataSourceGroup[i];
						else
							evenGroup[i/2] = chDataSourceGroup[i];
					}

					//ת��
					transpositionInt(oddGroup, outGroup, DAC_32BIT);
					transpositionInt(evenGroup, outGroup+32, DAC_32BIT);
					//  24λ���ֽ���װ��ȡ��24λ
					for(UCHAR i = 0; i < 64; i++)
					{
						//С��,ȡ��24λ
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


	//ת�÷���
	//�����ֱ�Ϊ�������顢������顢��Чλ����DACλ��,������Ϊ24bit��
	//����Ϊ�̶���32λת�ã����ڸĽ�
	int transpositionInt(unsigned int in[32],  unsigned int out[32],char bitWide)
{
	unsigned int padBit = 0x01 << (bitWide -1);//��Чλ�����λȡ1
	for(int i = 0; i < 32; i++)//32λ�޷������͵�ת���������32*4�ֽڡ�
		{
			out[i] = 0;
			//��i���������������32��������iλ���綼�������λ����ĳλ����������������ƶ�0-31λ�����Ľ������out_1 = data_1_bit&data_2_biit&...data_32_bit.���У�bit��ĳλ��ָ��
			for(int j = 0; j < 32; j++)
			{
				out[i] |= (((in[j] << i ) & padBit) >> j);

		    }			
		}
	return 0;

}

	unsigned char getChannels(CCyUSBEndPoint *EndPt,unsigned int *channelDataSourceGroup, unsigned char channelsCount)
	{
		chCount = channelsCount;
		chDataSourceGroup = channelDataSourceGroup;
		
		isXfer =1;
	//	XferThread = CreateThread(NULL, 0, XferChData, NULL, 0, NULL);
	//	XferThread = gcnew Thread(gcnew ThreadStart(&XferChData));
		//������ö˵�
		EndPt->Abort();
		EndPt->Reset();
		XferChData(EndPt, QUEUE_SIZE);//
		return 0;

	}
		
			
			
		
		
	