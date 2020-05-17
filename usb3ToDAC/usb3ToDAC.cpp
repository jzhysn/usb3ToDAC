// usb3ToDAC.cpp : ���� DLL Ӧ�ó���ĵ���������
//
/*
��ȡusb�豸
��ȡ�鴫��˵㣬��Ҫ��out
�����������(��32��������У�ÿ����������PPX������ÿ������MaxPktSize�ֽ�)
��ȡ��������
������������
ͨ���˵�out������
*/
#pragma once
#include "stdafx.h"
#include "usb3ToDAC.h"
#include "CyAPI.h"
#include <wtypes.h> 
#include <dbt.h>

 //   using namespace System;
  //  using namespace System::ComponentModel;
  //  using namespace System::Collections;
   // using namespace System::Windows::Forms;
   // using namespace System::Data;
  //  using namespace System::Drawing;
//    using namespace System::Threading;
  //  using namespace System::Diagnostics;
  //  using namespace System::Reflection;

	//Thread *XferThread;
	CCyUSBDevice *USBDevice;
	static const int MAX_QUEUE_SIZE = 64;
	static const int VENDOR_ID = 0x04B4;
	static const int PRODUCT_ID = 0x00F1;
	
	static CCyUSBEndPoint *EndPt;
	static int PPX = PACKETS_PER_XFER;//ÿ�δ��ݵİ���
	static int QueueSize = QUEUE_SIZE;
	static int TimeOut = TIMEOUT;
	static UCHAR eptAddr = BULK_OUT_ADDR;
	//static bool bShowData;
	//static bool bStreaming;
	//static bool bDeviceRefreshNeeded;
	//static bool bAppQuiting;
	
	static bool bHighSpeedDevice;
	static bool bSuperSpeedDevice;
	
	
	int GetUSBDevice()//��ȡ�����豸�������豸��
		{
			
			int n = 0;
			USBDevice = new CCyUSBDevice(0,CYUSBDRV_GUID,true);
			if(USBDevice ==NULL) return 0;
		
			n = USBDevice->DeviceCount();
			//printf("deviceCount = %d \n",);
			//��USB�豸
			for(int i = 0; i < n; i++)
			{
				USBDevice->Open(i);
			
				//�����Ҫ��ȡ�豸�б����
				//USBDevice->VendorID.ToString("X4");USBDevice->ProductID.ToString("X4");
				//Ҳ��ƥ��Ψһ���豸
				//if ((USBDevice->VendorID == VENDOR_ID) && (USBDevice->ProductID == PRODUCT_ID)) 
                //    break;
				/////////////////////////////////////////////////////////////////
            // Walk through all devices looking for VENDOR_ID/PRODUCT_ID
            // We No longer got restricted with vendor ID and Product ID.
            // Check for vendor ID and product ID is discontinued.
            ///////////////////////////////////////////////////////////////////
              /*  String *strDeviceData = "";
                strDeviceData = String::Concat(strDeviceData, "(0x");
                strDeviceData = String::Concat(strDeviceData, USBDevice->VendorID.ToString("X4"));
                strDeviceData = String::Concat(strDeviceData, " - 0x");
                strDeviceData = String::Concat(strDeviceData, USBDevice->ProductID.ToString("X4"));
                strDeviceData = String::Concat(strDeviceData, ") ");
                strDeviceData = String::Concat(strDeviceData, USBDevice->FriendlyName);
				*/

			//	deviceId[i] = i;
               
        }

			//Ĭ�ϴ򿪵�һ���豸
			 if (n > 0 ) 
			 {               
                USBDevice->Open(0);
             }

			 int interfaces = USBDevice->AltIntfcCount()+1;//���ýӿ�+1
			 bHighSpeedDevice = USBDevice-> bHighSpeed;
			 bSuperSpeedDevice = USBDevice->bSuperSpeed;
			 
				//���ö˿�
			 EndPt = USBDevice->EndPointOf(eptAddr);
			// EndPt->TimeOut = 1000;


			 	//��ȡ�˿���Ϣ
			/*
			 for(int i = 0; i <interfaces;i++)
			 {
				if(USBDevice->SetAltIntfc(i) == true)
				{
					int eptCnt = USBDevice->EndPointCount();
					
				
					for( int e = 1; e < eptCnt; e++)
					{
						CCyUSBEndPoint *ept = USBDevice->EndPoints[e];
						//֧���жϡ����ͬ���˵�
						if((ept->Attributes >= 1)&&(ept->Attributes <=3))
						{
							string *s = "";
							s = String::Concat(s,((ept->Attributes == 1) ? 
								"ISOC " : ((ept->Attritubes == 2) ? "Bulk " : "INTR ")));
							s = String::Concat(s, ept->bIn ? "IN,       " : "OUT,       "����
							s = String::Concat(s, ept->MaxPktSize.toString(), "Bytes,");
							if(USBDevice->BcdUSB == USB30MAJORVER)
                                    s = String::Concat(s, ept->ssmaxburst.ToString(), " MaxBurst,");

                                s = String::Concat(s, "   (", i.ToString(), " - ");
                                s = String::Concat(s, "0x", ept->Address.ToString("X02"), ")");
						 }

					  }
					}
				}
			 */

			return n;		  

			 }
	CCyUSBEndPoint *getEndPt()
	{
		return USBDevice->BulkOutEndPt;
	}
	void CreatBuffer()
	{
		PUCHAR *buffers = new PUCHAR[QueueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[QueueSize];
		PUCHAR *contexts = new PUCHAR[QueueSize];
		OVERLAPPED inOvLap[QUEUE_SIZE];

		long len = EndPt->MaxPktSize*PPX;
		EndPt->SetXferSize(len);

		for(int i = 0; i < QueueSize; i++)
		{
			buffers[i] = new UCHAR[len];
			isoPktInfos[i] = new CCyIsoPktInfo[PPX];
		inOvLap[i].hEvent = CreateEvent(NULL,false,false,NULL);
		memset(buffers[i],0x00,len);

		}
	}


	static UINT *oddChannels;
	static UINT *evenChannels;
	static UINT *outData;
	

	int GetIn(unsigned int *inData, int len,UCHAR DAC_BITS)
	{//��ȡ���ݣ����ֳ���ż���飬����;ע������������2��

		if(len > MAX_CHANNLE_COUNT) return 0;

		UCHAR halfCount = (len+1)/2;
		oddChannels = new UINT[halfCount];
		evenChannels = new UINT[halfCount];
		
		

		for( int i = 0; i < halfCount ; i++)
		{
			oddChannels[i] = inData[2*i];
			evenChannels[i] = inData[2*i+1];

		}
		outData = new UINT[len];
		transpositionInt(oddChannels, outData, DAC_32BIT);
		transpositionInt(evenChannels, outData+halfCount, DAC_32BIT);
		//XferData(outData, len, DAC_32BIT);
		if(XferData(outData, len, DAC_BITS))//XferDataʧ�ܷ���1
		{
			return 0;
		}

		delete oddChannels,evenChannels,outData;
		

		return 1;
	}
	
	 void xFerToGPIFTest(unsigned int constData, unsigned char isSine)
	{
		PUCHAR *buffers = new PUCHAR[QueueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[QueueSize];
		PUCHAR *contexts = new PUCHAR[QueueSize];
		OVERLAPPED inOvLap[QUEUE_SIZE];
		unsigned long Successes = 1;

		UCHAR bitSize = 24;
           
		  
		
		
		EndPt = USBDevice->BulkOutEndPt;
		//EndPt = USBDevice->IsocOutEndPt;


		//���isSine����0 ����������Ǻ�����������ԣ�������й̶�ֵ�Ĳ���
		//ÿ48�鹲48*4=192���ֽ�Ϊһ�飬ÿ�δ���MaxPktSize*ppx%192 = 0
		if(isSine > 0)
		{
		}

		else
		{
			
			
			
			unsigned int groupData[32] = {0};
			
			for(int m = 0; m < 32; m++)
				groupData[m] = constData;

			unsigned int groupDataTrans[32] = {0};
			
			transpositionInt(groupData, groupDataTrans, DAC_32BIT);//��24λת��
			
			unsigned char groupDataTransChar[24*8] = {0};
		//  24λ��װ
		
		for(int i = 8; i < 32; i++)
			{
				
				//��˸�ʽ
				//groupDataTransChar[(i-8)*4] = groupDataTrans[i]>>24;
				//groupDataTransChar[(i-8)*4+1] = groupDataTrans[i]>>16;
				//groupDataTransChar[(i-8)*4+2] = groupDataTrans[i]>>8;
				//groupDataTransChar[(i-8)*4+3] =groupDataTrans[i];

				//groupDataTransChar[(i+16)*4] = groupDataTrans[i]>>24;
				//groupDataTransChar[(i+16)*4+1] = groupDataTrans[i]>>16;
				//groupDataTransChar[(i+16)*4+2] = groupDataTrans[i]>>8;
				//groupDataTransChar[(i+16)*4+3] =groupDataTrans[i];
				//С��
				memcpy(&(groupDataTransChar[(i-8)*4]),&(groupDataTrans[i]),4);
				memcpy(&(groupDataTransChar[(i+16)*4]),&(groupDataTrans[i]),4);			
			}
			
			//32λ����
			/*
			for(int i = 0; i < 32; i++)
			{
				
				//С��
				memcpy(&(groupDataTransChar[i*4]),&(groupDataTrans[i]),4);
				memcpy(&(groupDataTransChar[(i+32)*4]),&(groupDataTrans[i]),4);			
			}
			*/
		
		
			int ppx = 1;
			//while((EndPt->MaxPktSize*ppx)%192 !=0) ppx++;
			while((EndPt->MaxPktSize*ppx)%(bitSize*8) !=0) ppx++;
			ppx = 10*ppx;
			long bufferLen = ppx*EndPt->MaxPktSize;
		EndPt->SetXferSize(bufferLen);//ÿ��������bufferLen���ֽ�
		
		int i = 0;

		for(i = 0; i < QueueSize; i++)
		{
			buffers[i] = new UCHAR[bufferLen];
			isoPktInfos[i] = new CCyIsoPktInfo[ppx];
			inOvLap[i].hEvent = CreateEvent(NULL,false,false,NULL);
			//memset(buffers[i],0x00,byteLen);
			
			for(int h = 0; h < bufferLen;)
			{
			
				for(int j = 0; (j < bitSize*8) && (h < bufferLen); j++, h++)
					buffers[i][h] = groupDataTransChar[j];	
				

			}
		}
// Queue-up the first batch of transfer requests
           for (int i=0; i< QueueSize; i++)	
            {
                contexts[i] = EndPt->BeginDataXfer(buffers[i], bufferLen, &inOvLap[i]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                    
                    return;
                }
            }
             
		
		i=0;	

           
            while(Successes)		
            {
                long rLen = bufferLen;	// Reset this each time through because
                // FinishDataXfer may modify it
				if (!EndPt->WaitForXfer(&inOvLap[i], TimeOut))
                {
                    EndPt->Abort();
                    if (EndPt->LastError == ERROR_IO_PENDING)
                        WaitForSingleObject(inOvLap[i].hEvent,2000);
                }

				 if (EndPt->Attributes == 1) // ISOC Endpoint
                {	
                    if (EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i], isoPktInfos[i])) 
                    {			
                        CCyIsoPktInfo *pkts = isoPktInfos[i];
                        for (int j=0; j< ppx; j++) 
                        {
							if ((pkts[j].Status == 0) && (pkts[j].Length<=EndPt->MaxPktSize)) 
								Successes =1;
						//	else
						//		Successes = 0; 

                            pkts[j].Length = 0;	// Reset to zero for re-use.
							pkts[j].Status = 0;
                        }

                    } 
                 //   else
                  //      Successes = 0;

                } 

                else // BULK Endpoint

				{
					if (EndPt->FinishDataXfer(buffers[i], rLen, &inOvLap[i], contexts[i])) 
                    Successes =1;
				else
                       Successes = 0; 
				}

               
                


              

                // Re-submit this queue element to keep the queue full
                contexts[i] = EndPt->BeginDataXfer(buffers[i], bufferLen, &inOvLap[i]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                   
                    return;
                }



 


                i++;

                if (i == QueueSize) //Only update the display once each time through the Queue
                {
                    i=0;
                    					
                }

				

            }  

			
		}

		
		
	}
	int GetIn_no_transposition(unsigned int *inData, int len,UCHAR DAC_BITS)
	{//��ȡ���ݣ����ֳ���ż���飬����;ע������������2��

		if(len > MAX_CHANNLE_COUNT) return 0;

		
		if(XferData(inData, len, DAC_BITS))//XferDataʧ�ܷ���1
		{
			return 0;
		}

		delete oddChannels,evenChannels,outData;
		

		return 1;
	}
	//ת�÷���
	//�����ֱ�Ϊ�������顢������顢��Чλ����DACλ��,������Ϊ24bit��
	//����Ϊ�̶���32λת�ã����ڸĽ�
	int transpositionInt(unsigned int labView_in[32],  unsigned int test_int_data_out[32],char bitWide)
{
	unsigned int padBit = 0x01 << (bitWide -1);//��Чλ�����λȡ1
	for(int i = 0; i < 32; i++)//32λ�޷������͵�ת���������32*4�ֽڡ�
		{
			test_int_data_out[i] = 0;
			//��i���������������32��������iλ���綼�������λ����ĳλ����������������ƶ�0-31λ�����Ľ������out_1 = data_1_bit&data_2_biit&...data_32_bit.���У�bit��ĳλ��ָ��
			for(int j = 0; j < 32; j++)
			{
		//	printf("%08x\n",test_int_data[j]);

			test_int_data_out[i] |= (((labView_in[j] << i ) & padBit) >> j);

		     }
			//printf("**%d**",i);
			//printf("**%08x\n",test_int_data_out[i]);
		}
	return 0;

}


	static UCHAR *outBuffer;
	static UCHAR *outContext;
	//
	

	int XferData(UINT *outDatas, UCHAR len, UCHAR DAC_BITS)
	{
		 static OVERLAPPED outOvLap, inOvLap;  
		//OVERLAPPED outOvLap, inOvLap;
		outOvLap.hEvent = CreateEvent(NULL, false, false, L"CYUSB_OUT");
		//inOvLap.hEvent = CreateEvent(NULL, false, false, L"CYUSB_IN");

		//unsigned char inBuf[64*4];
		//ZeroMemory(inBuf, 64*4);
		

		//UCHAR *outBuffer;
		LONG byteLen = 0;
		

		//���������������ת��Ϊ�ֽ�����
		//��Ϊ24λDAC���ݣ���outData��0~7��32~39��int����ɾȥ
		switch(DAC_BITS)
		{
		case DAC_32BIT:

			byteLen = len*4;
			outBuffer = new UCHAR[byteLen];

			for(int i = 0; i < len; i++)
			{
				outBuffer[i*4] = *(outDatas+i)>>24;
				outBuffer[i*4+1] = *(outDatas+i)>>16;
				outBuffer[i*4+2] = *(outDatas+i)>>8;
				outBuffer[i*4+3] = *(outDatas+i);

			}
			
			break;
		case DAC_24BIT:

			byteLen = (len - 2*8)*4;
			outBuffer = new UCHAR[byteLen];

			for(int i = 8; i < 32; i++)
			{
				outBuffer[(i-8)*4] = *(outDatas+i)>>24;
				outBuffer[(i-8)*4+1] = *(outDatas+i)>>16;
				outBuffer[(i-8)*4+2] = *(outDatas+i)>>8;
				outBuffer[(i-8)*4+3] = *(outDatas+i);
			}
			for(int i = 40; i < 64; i++)
			{
				outBuffer[(i-8*2)*4] = *(outDatas+i)>>24;
				outBuffer[(i-8*2)*4+1] = *(outDatas+i)>>16;
				outBuffer[(i-8*2)*4+2] = *(outDatas+i)>>8;
				outBuffer[(i-8*2)*4+3] = *(outDatas+i);
			}
			

			break;
		default:
			byteLen = len*4;
			outBuffer = new UCHAR[byteLen];
			ZeroMemory(outBuffer, 64*4);
			
			break;
		}

		
		
		EndPt = USBDevice->BulkOutEndPt;
		//outContext= EndPt->BeginDataXfer(outBuffer, byteLen,&outOvLap);
		if(!(EndPt->XferData(outBuffer, byteLen)))
                {
                    delete outBuffer;
                    return 1;
                }

		delete outBuffer;
		return 0;

	
	}
	//����һ���̶����������в���
	//����DAC_BITS��ȡҪ��������ݣ��ر�˵��24λ���ݵĴ��䣬��ȡ�������ĺ�3���ֽڣ��������ִ�С��
	//����һ������Ԫ�غ��ͣ�Ȼ�󲻶��������������ݣ�
int XferTestData(UINT outDatas,  UCHAR DAC_BITS)
	{
		 UCHAR outDataWide = 0;

		 UCHAR Successes = 1;
            
		  int loop = 0;
		
		//���������������ת��Ϊ�ֽ�����
		//��Ϊ24λDAC���ݣ���outData��0~7��32~39��int����ɾȥ
		switch(DAC_BITS)
		{
		case DAC_32BIT:

			 outDataWide = 4;
			 outBuffer = new UCHAR[outDataWide];
		  memcpy(outBuffer,&outDatas,outDataWide);
			

			//	outBuffer[0] = outDatas>>24;
			//	outBuffer[1] = outDatas>>16;
			//	outBuffer[2] = outDatas>>8;
			//	outBuffer[3] = outDatas;	
				


			break;
		case DAC_24BIT:

			 outDataWide = 3;
			 outBuffer = new UCHAR[outDataWide];
		   memcpy(outBuffer,&outDatas,outDataWide);			

			
			//	outBuffer[0] = outDatas>>16;
			//	outBuffer[1] = outDatas>>8;
			//	outBuffer[2] = outDatas;
				
				break;
		default:
			outDataWide = 4;
			outBuffer = new UCHAR[outDataWide];
			ZeroMemory(outBuffer, 4);
			
			break;
		}


		

		PUCHAR *buffers = new PUCHAR[QueueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[QueueSize];
		PUCHAR *contexts = new PUCHAR[QueueSize];
		OVERLAPPED inOvLap[QUEUE_SIZE];

		

		//���������������ת��Ϊ�ֽ�����
	

		EndPt = USBDevice->BulkOutEndPt;//��������˵�
		long byteLen = EndPt->MaxPktSize*PPX;
		EndPt->SetXferSize(byteLen);//ÿ�δ���byteLen���ֽ�

		for(int i = 0; i < QueueSize; i++)
		{
			buffers[i] = new UCHAR[byteLen];
			isoPktInfos[i] = new CCyIsoPktInfo[PPX];
			inOvLap[i].hEvent = CreateEvent(NULL,false,false,NULL);
			//memset(buffers[i],0x00,byteLen);
			
			for(int h = 0; h < byteLen;)
			{
			
				for(int j = 0; (j < outDataWide) && (h < byteLen); j++, h++)
					buffers[i][h] = outBuffer[j];	
				

			}
		}

		// Queue-up the first batch of transfer requests
           for (int i=0; i< QueueSize; i++)	
            {
                contexts[i] = EndPt->BeginDataXfer(buffers[i], byteLen, &inOvLap[i]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                    
                    return 1;
                }
            }
             
	
            // The infinite xfer loop.
        while(Successes)		
            {
                long rLen = byteLen;	// Reset this each time through because
                // FinishDataXfer may modify it

               if (!EndPt->WaitForXfer(&inOvLap[loop], TimeOut))
                {
                    EndPt->Abort();
                    if (EndPt->LastError == ERROR_IO_PENDING)
                        WaitForSingleObject(inOvLap[loop].hEvent,2000);
                }

                if (EndPt->FinishDataXfer(buffers[loop], rLen, &inOvLap[loop], contexts[loop])) 
                    Successes = 1;
				else
                        Successes = 0;
             


              

                // Re-submit this queue element to keep the queue full
                contexts[loop] = EndPt->BeginDataXfer(buffers[loop], byteLen, &inOvLap[loop]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                   
                    return 1;
                }



				






                loop++;

                if (loop == QueueSize) //Only update the display once each time through the Queue
                {
                    loop=0;
                    					
                }

				

            }  
delete outBuffer;
		return 1;
	
	}
	


	///���л�����ʽ�������

/**
	void XferQueueData(UINT *outData, UCHAR len)
	{
		PUCHAR *buffers = new PUCHAR[QueueSize];
		CCyIsoPktInfo **isoPktInfos = new CCyIsoPktInfo*[QueueSize];
		PUCHAR *contexts = new PUCHAR[QueueSize];
		OVERLAPPED inOvLap[QUEUE_SIZE];

		

		//���������������ת��Ϊ�ֽ�����
		byteLen = len*4;

		EndPt = USBDevice->BulkOutEndPt//��������˵�
		//long len = EndPt->MaxPktSize*PPX;
		EndPt->SetXferSize(byteLen);//ÿ��������byteLen���ֽ�

		for(int i = 0; i < QueueSize; i++)
		{
			buffers[i] = new UCHAR[byteLen];
			isoPktInfos[i] = new CCyIsoPktInfo[PPX];
		inOvLap[i].hEvent = CreateEvent(NULL,false,false,NULL);
		//memset(buffers[i],0x00,len);
			for(int j = 0; j < len; j++)
			{
				buffers[i][j*4] = *(outData+j)>>24;
				buffers[i][j*4+1] = *(outData+j)>>16;
				buffers[i][j*4+2] = *(outData+j)>>8;
				buffers[i][j*4+3] = *(outData+j);

			}
		}
		             
		


		//���뻺��
		//unsigned char inBuf[64*4];
		//ZeroMemory(inBuf, 64*4);		

		
		
		contexts[i] = EndPt->BeginDataXfer(buffers[i], len, &inOvLap[i]);
                if (EndPt->NtStatus || EndPt->UsbdStatus) // BeginDataXfer failed
                {
                    Display(String::Concat("Xfer request rejected. NTSTATUS = ",EndPt->NtStatus.ToString("x")));
                    AbortXferQueueLoop(QueueSize,buffers,isoPktInfos,contexts,inOvLap);
                    return;
                }	
	}


	 static void AbortXferQueueLoop(int pending, PUCHAR *buffers, CCyIsoPktInfo **isoPktInfos, PUCHAR *contexts, OVERLAPPED inOvLap __nogc [])
        {
            //EndPt->Abort(); - This is disabled to make sure that while application is doing IO and user unplug the device, this function hang the app.
            long len = EndPt->MaxPktSize * PPX;
            EndPt->Abort();

            for (int j=0; j< QueueSize; j++) 
            { 
                if (j<pending)
                {
                    EndPt->WaitForXfer(&inOvLap[j], TimeOut);
                    EndPt->FinishDataXfer(buffers[j], len, &inOvLap[j], contexts[j]);
                }

                CloseHandle(inOvLap[j].hEvent);

                delete [] buffers[j];
                delete [] isoPktInfos[j];
            }

            delete [] buffers;
            delete [] isoPktInfos;
            delete [] contexts;

      
          
        }
		*/

	


