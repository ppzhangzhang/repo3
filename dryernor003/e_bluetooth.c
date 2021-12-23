#include <sys/ioctl.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "ite/ith.h"
#include "ite/itp.h"
#include "bt/bluetooth.h"
#include "projutils.h"
#include "layer_ctrl.h"

extern bool bluetoothOK;

#define AT_NUM 7
#define BT_UART ITP_DEVICE_UART2
static bool TestFuncBTExit = false;
static bool TestFuncBTisRun = false;


//#define WTRTEBT_DEBUG //write to uart log
#ifdef WTRTEBT_DEBUG
#define WRITEBT_LOG printf(
#else
#define WRITEBT_LOG   (void)(1 ? 0 :
#endif
static void printDataLog(u8* data, u8 num){
	for (size_t i = 0; i < num; i++)//READ_UI_LENGTH
	{
		printf("%0x ", data[i]);
	}
	printf("\n");
}
int writeBTDatatoUart2(u8* data, u8 len){
	u8 wlen = 0;
	int i = 0;
	WRITEBT_LOG"write to UART2:"LOG_END
	for (; i < len; i++)
		WRITEBT_LOG"%x ", data[i]LOG_END
		WRITEBT_LOG"\n"LOG_END
	while (wlen < len)
	{
		wlen = write(ITP_DEVICE_UART2, data + wlen, len - wlen);
		WRITEBT_LOG"total is %d  write size is %d \n\n", len, wlen LOG_END
	}
	return wlen;
}

void* TestFuncBT(void* arg)
{
	TestFuncBTExit = false;
	TestFuncBTisRun = true;
	int i = 0;
	char getstr[256];
	//char sendtr[256];
	int len = 0;
	char atc[40];
	bool readFin = true;
	const char *atc0[AT_NUM] = { //[27] = { 
#if CFG_ESP32
		//"AT\r\n",
		//"AT+GMR\r\n",
		"AT+CWMODE=3\r\n",
		"AT+CWJAP=\"dd-wrt-ITE\",\"ff75525613\"\r\n",
		"AT+CWLAP\r\n",
#else
		"AT\r\n",
		"AT+VERSION\r\n",
		"AT+LADDR\r\n",
		"AT+NAME?\r\n",
		"AT+BAUD?\r\n",
		"AT+ROLE?\r\n",
		"AT+SCAN=1\r\n",
		//"AT+PIN\r\n",
		//"AT+TYPE\r\n",
		//"AT+STOP\r\n",
		//"AT+PARI\r\n",
		//"AT+ADVI\r\n",
		//"AT+NOTI\r\n",
		//"AT+NOTP\r\n",
		//"AT+IMME\r\n",
		//"AT+START\r\n",
		//"AT+UUID\r\n",
		//"AT+CHAR\r\n",
		//"AT+IBEA\r\n",
		//"AT+IBE0\r\n",
		//"AT+IBE1\r\n",
		//"AT+IBE2\r\n",
		//"AT+IBE3\r\n",
		//"AT+MARJ\r\n",
		//"AT+MINO\r\n",
		//"AT+MEA\r\n",
		//"AT+PWRM\r\n",
		//"AT+INQ\r\n"
		//"\r\n",
#endif
	};
	printf("Start uart test!\n");

	/*itpRegisterDevice(BT_UART, &itpDeviceUart2);
	ioctl(BT_UART, ITP_IOCTL_INIT, NULL);
	ioctl(BT_UART, ITP_IOCTL_RESET, CFG_UART2_BAUDRATE);*/
	//usleep(5000000);
	//fd = bt_dev_open(BT_UART);
	/*if (fd)
	printf("open device success\n");
	else
	printf("open device fail\n");*/
	int times = 0;
	int offset;
	while (!TestFuncBTExit){
		offset = i++ % AT_NUM;
		if (readFin)
		{
			//printf("=>BT=>");
			//printf("%s\n", atc0[offset]);
			strcpy(atc, "AT\r\n");//循环
			//strcpy(atc, atc0[offset]);//循环
			writeBTDatatoUart2(atc, (size_t)strlen(atc));
			readFin = false;
		}
		memset(getstr, 0, 256);
		times = 0;
		//printf("before readlen=%d\n", len);
		while ((len <= 0) && (times < 10)){
			times++;
			usleep(100000);

			len = read(ITP_DEVICE_UART2, getstr, 128);
			//printf("uart read  len=%d\n", len);

			/*sprintf(getstr, "%s", "mengfanbao");
			len = strlen("mengfanbao");*/
			if (len > 0){
				break;
			}
		}
		//printf("uart read cmd=%d len=%d,retry times=%d: %s\n", offset, len, times, getstr);
		if (len > 0){
			//printDataLog(getstr, len);
			len = 0;
		}
		//memcpy(getstr,"OK123",5);
		if (getstr[0] == 0x4f && getstr[1] == 0x4b){
			bluetoothOK = true;
		}
		else{
			bluetoothOK = false;
		}
		readFin = true;
		usleep(1000000);
	}
	TestFuncBTisRun = false;
	return NULL;
}

void TestFuncBTFuncExit(){
	TestFuncBTExit = true;
}

bool TestFuncBTFuncisRun(){
	return TestFuncBTisRun;
}

void CreateWorkerBTThread(){
	if (TestFuncBTisRun){//已经在运行则退出
		return;
	}
	CreateWorkerThread(TestFuncBT, NULL);
}