#include "CyAPI.h"
#include <wtypes.h> 

#define QUEUE_SIZE 12 //队列长度
#define PACKETS_PER_XFER 30  //每次传输的包数
#define TIMEOUT 2000 //超时（ms）
#define BULK_OUT_ADDR 0x04
#define MAX_CHANNLE_COUNT 64
#define TRANSPOTSION_COUNT 32
#define DAC_24BIT 24
#define DAC_32BIT 32
#define CHANNEL_COUNT_24 24
#define CHANNEL_COUNT_32 32

extern "C"
{
	_declspec(dllexport) int GetUSBDevice();
	_declspec(dllexport) int GetIn(unsigned int *inData, int len,UCHAR DAC_BITS);
	_declspec(dllexport) int GetIn_no_transposition(unsigned int *inData, int len,UCHAR DAC_BITS);
	_declspec(dllexport) int XferTestData(UINT outData,  UCHAR DAC_BITS);
	_declspec(dllexport) void xFerToGPIFTest(unsigned int constData, unsigned char isSine);
	_declspec(dllexport) CCyUSBEndPoint *getEndPt();
};


int transpositionInt(unsigned int labView_in[32],  unsigned int test_int_data_out[32],char bitWide);
int XferData(UINT *outData, UCHAR len, UCHAR DAC_BITS);
