#include "boardstate.h"
#include <assert.h>

#define USE_BUS

bool UIDataChange = false;
#define READ_UI_LENGTH 0x32//50=0x32 96=0x60
u8 dryerKeyValue1 = 0;
u8 dryerKeyValue2 = 0;
u8 keyLongPress = 0;
u8 pressLeftMidFlag = 0;
WP_EVENT_UI_LOGIC ui = { 0 };
WP_EVENT_UI_LOGIC ui_tm = { 0 };//testmode
WPEVENT_UI_DATA160 ui_data160 = { 0 };	//上传 DATA 0x160~16F

WPEVENT_PROGRAM_PARAMRTER pp = { 0 };	//上传 程序参数 0x200
WPEVENT_DISCOUNT_PARAMRTER dp = { 0 };	//上传 折扣参数 0x300
WPEVENT_INFO_PARAMRTER ip = { 0 };		//上传 统计参数 0x400
WPEVENT_CTRL_PARAMRTER cp = { 0 };		//上传 控制 0x500
WPEVENT_MACHINE_STATE ms = { 0 };		//上传 机器运行 0x600
WPEVENT_MACHINE_RUN_STATE mrs = { 0 };	//上传 机器运行状态 0x700
WPW_INFO wpwi;
cache theCache;

dealDataType dealData = { 0 };
UIEVENT g_UIevent = { 0 };


MASTER_STATE_TYPE mst_state;
uint8_t para_rsttime[64];

uint8_t para_uilogic[64];

uint8_t para_testmode[96];

uint8_t para_special_display[16];

uint8_t para_info[128];

#define UARTMAXLEN 256
static  uint8_t readbuf[128] = { 0 };
static  uint8_t uart0readData[UARTMAXLEN] = { 0 };
u8 replysize = 0;
u8 readsize = 0;
u8 write_buf[128];
u8 resendbuf[128];
extern bool busComOK;
u8 spcialdisplay = 0;
bool connflag = false;
OutExternalEvent ev = { 0 };//传递队列，带有待解码的信息


mqd_t extInQueue = -1;
mqd_t extOutQueue = -1;
mqd_t extPMQueue = -1;//productmod

#define EXT_MAX_QUEUE_SIZE 8

volatile bool extQuit = 0;
volatile bool exitNormalModeQuit = 0;
volatile bool exitProductModeQuit = 0;

u8 getDoorState()
{
	if((ui.StateA&0x80) == 0x80)
		{
			return 1; //closed
		}
	else
		{
			return 0;//open
		}
}
u8 getDoorLockState()
{
	if((ui.StateA&0x40) == 0x40)
		{
			return 1;//lock
		}
	else
		{
			return 0; //unlock
		}
}
u8 getCashBoxState()
{
	if((ui.StateA&0x20) == 0x20)
		{
			return 1;//inserted
		}
	else
		{
			return 0; //not exist
		}
}
int ExternalOutSend(OutExternalEvent* ev)
{
	assert(ev);
#ifdef WIN32
	//return -1;
#endif // WIN32

	if (extQuit)
		return -1;
	if (extOutQueue == -1)
	{
		printf("extOutQueue not ready \n");
		return -1;
	}
	return mq_send(extOutQueue, (char*)ev, sizeof(OutExternalEvent), 0);
}

bool is_cx_service_mode()//服务模式
{
	return (ui.CycleName == 0xfa);
}
bool is_cx_normal_mode()//
{
	return (ui.CycleName < 0x04);
}

/*
170 -> 82
200-> 83
*/
u8 getReadCMD(u16 offset){
	return (((u8)((offset >> 8) | 0x80)) + 1);
}
/*
170 -> 70
200-> 00
*/
u8 getReadADDR(u16 offset){
	return ((u8)offset);
}


u8 write_buf[128];


typedef struct
{
	UI_QUERY_NAME QueryName;
	bool QuaryFlag;
}UI_QUERY_TYPE;
typedef struct
{
	UI_QUERY_TYPE Q0;
	UI_QUERY_TYPE Q1;
	UI_QUERY_TYPE Q2;
	UI_QUERY_TYPE Q3;
	UI_QUERY_TYPE Q4;
	UI_QUERY_TYPE Q5;
	UI_QUERY_TYPE Q6;
	UI_QUERY_TYPE Q7;
	UI_QUERY_TYPE Q8;
	UI_QUERY_TYPE Q9;
	UI_QUERY_TYPE Q10;
}UI_QUERY_LIST;
typedef struct
{
	UI_QUERY_LIST uql;
	u8 CurQueryNum;
	bool sendFlag;
}UI_QUERY_CONFIG;
//UI_QUERY_LIST uql = { { QUERY_200_TO_22C_2C, 0 }, { QUERY_300_TO_33C_3C, 0 }, { QUERY_400_TO_43C_3C, 0 }, { QUERY_43C_TO_468_2C, 0 }, { QUERY_500_TO_53E_3E, 0 }, { QUERY_600_TO_52E_2E, 0 }, { 0, 0 }, { 0, 0 }, 0 };
/*
UI_QUERY_CONFIG uqc = { \
{ \
{ 0, 0 }, \
{ QUERY_200_TO_22C_2C, 1 }, \
{ QUERY_300_TO_33C_3C, 1 }, \
{ QUERY_400_TO_43C_3C, 1 }, \
{ QUERY_43C_TO_468_2C, 1 }, \
{ QUERY_500_TO_515_15, 1 }, \
{ QUERY_520_TO_545_25, 1 }, \
{ QUERY_600_TO_52E_2E, 1 }, \
{ QUERY_160_TO_170_10, 0 }, \
{ QUERY_700_TO_711_12, 1 }, \
{ QUERY_170_TO_1FF_90, 1 }, \
{ 0, 0 }, \
}, \
0, 0 \
};
*/
UI_QUERY_CONFIG uqc = { \
{ \
{ 0, 0 }, \
{ QUERY_200_TO_22C_2C, 1 }, \
{ QUERY_300_TO_33C_3C, 1 }, \
{ QUERY_400_TO_43C_3C, 1 }, \
{ QUERY_43C_TO_468_2C, 1 }, \
{ QUERY_500_TO_515_15, 1 }, \
{ QUERY_520_TO_545_25, 1 }, \
{ QUERY_600_TO_52E_2E, 1 }, \
{ QUERY_160_TO_170_10, 0 }, \
{ QUERY_700_TO_711_12, 1 }, \
{ QUERY_170_TO_1FF_90, 1 }, \
},\
0, 0 \
};
static u8 setQueryFlagNum = 0;
static u8 closeQueryFlagNum = 0;
void setQueryFlag(u8 flag){//1-7
	setQueryFlagNum = flag;
}
void closeQueryFlag(u8 flag){//1-7
	closeQueryFlagNum = flag;
}
static UI_QUERY_NAME getQueryTarget(){
	if (setQueryFlagNum){
		switch (setQueryFlagNum)
		{
		case QUERY_160_TO_170_10:
			uqc.uql.Q8.QuaryFlag = true;
			uqc.uql.Q1.QuaryFlag = false;
			uqc.uql.Q2.QuaryFlag = false;
			uqc.uql.Q3.QuaryFlag = false;
			uqc.uql.Q4.QuaryFlag = false;
			uqc.uql.Q5.QuaryFlag = false;
			uqc.uql.Q6.QuaryFlag = false;
			uqc.uql.Q7.QuaryFlag = false;
			break;
		default:
			break;
		}
		setQueryFlagNum = 0;
	}
	if (closeQueryFlagNum){
		switch (closeQueryFlagNum)
		{
		case QUERY_160_TO_170_10:
			uqc.uql.Q8.QuaryFlag = false;
			uqc.uql.Q1.QuaryFlag = true;
			uqc.uql.Q2.QuaryFlag = true;
			uqc.uql.Q3.QuaryFlag = true;
			uqc.uql.Q4.QuaryFlag = true;
			uqc.uql.Q5.QuaryFlag = true;
			uqc.uql.Q6.QuaryFlag = true;
			uqc.uql.Q7.QuaryFlag = true;
			break;
		default:
			break;
		}
		closeQueryFlagNum = 0;
	}
	for (size_t i = 0; i < 11/*sizeof(UI_QUERY_LIST) / sizeof(UI_QUERY_TYPE)*/; i++)
	{
		uqc.CurQueryNum++;
		if (uqc.CurQueryNum >= 10){
			uqc.CurQueryNum = 0;
		}
		UI_QUERY_TYPE* queryPtr = ((UI_QUERY_TYPE*)&uqc.uql) + uqc.CurQueryNum;
		if (queryPtr->QuaryFlag == 1){
			if (queryPtr->QueryName != QUERY_NONE){
				return queryPtr->QueryName;//查询表里面下一个可以用的且打开的程序
			}
			else{//虽然程序显示打开了，但是程序名为QUERY_NONE，有错误，但继续轮询。
				continue;
			}

		}
	}
	return QUERY_NONE;//全部QuaryFlag都为零，查询一个轮回发现都为零，则返回QUERY_NONE = 0
}



/*
********************************************************************************************************
*/

int writeDatatoUart(u8* data, u8 len){
	u8 wlen = 0;
	int i = 0;
	//#ifdef WRITEBUS_DEBUG
	printf("write to UART1:");
	for (; i<len; i++)
		printf("%x ", data[i]);
		printf("\n");
		//#endif
	while (wlen < len)
	{
		wlen = write(ITP_DEVICE_UART1, data + wlen, len - wlen);
		WRITEBUS_LOG"total is %d  write size is %d \n\n", len, wlen LOG_END
	}

	return wlen;
}

void write_NullCmd_To_Board(){// 01 00 02 00 00 00 9c
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = addr000;
	index++;//for write len
	write_buf[index++] = 0x0;
	write_buf[index++] = 0x0;
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("write_buf[%d] is %x\t", i, write_buf[i]);
	}
	printf("write_NullCmd_To_Board\n");
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);
#ifdef USE_BUS
	replysize = index + 3;
#endif
	mst_state = MASTER_STATE_WTIRE_NULLCMD;
	writeDatatoUart(write_buf, index);
}

void write_ResetCmd_To_Board(){// 01 00 02 00 01 00 9c
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = addr000;
	index++;//for write len
	write_buf[index++] = 0x0;
	write_buf[index++] = 0x01;
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("write_buf[%d] is %x\t", i, write_buf[i]);
	}
	printf("write_ResetCmd_To_Board\n");
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	writeDatatoUart(write_buf, index);
}

void write_IdleCmd_To_Board(){// 01 00 02 00 0c 00 9c
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = addr000;
	index++;//for write len
	write_buf[index++] = 0x0;
	write_buf[index++] = 0x0c;
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("write_buf[%d] is %x\t", i, write_buf[i]);
	}
	printf("write_IdleCmd_To_Board\n");
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	writeDatatoUart(write_buf, index);
}
/*
********************************************************************************************************
*/


/*
NULLCMD RESETCMD IDLECMD
01 00 02 00 0xcmdType 00 9c
*/
void writeSingleCmdToBoard(UI_EVENT_TYPE cmdType){
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = addr000;
	index++;//for write len
	write_buf[index++] = 0x0;
	write_buf[index++] = (u8)cmdType;
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
	}
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	printf("writeSingleCmdToBoard cmdType:%d ", cmdType);
	printf("write_buf:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("\n");
#ifdef USE_BUS
	replysize = index + 3;//1 0 2 0 0 0 9c 01 c0 c1
#endif
	writeDatatoUart(write_buf, index);
}

void writeDoubleCmdToBoard(UI_EVENT_TYPE type, u8 value)
{// type and value
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = WRITE_ADDR;
	index++;//for write len
	write_buf[index++] = 0;			//EventCounter
	write_buf[index++] = (u8)type;	//EventType
	write_buf[index++] = (u8)value;	//EventValue
	write_buf[2] = index - 3;
	
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("[%x]", write_buf[i]);
	}
	//printf("\n");
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	printf("writeDoubleCmdToBoard type:%d value:%d->", type, value);
	printf("write_buf:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("\n");
#ifdef USE_BUS
	replysize = index + 3;//1 0 3 xx xx xx crc crc 01 c0 c1
#endif
	writeDatatoUart(write_buf, index);
}

void write_buffer_to_board(){//flexible
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = WRITE_ADDR;
	index++;//for write len
	write_buf[index++] = g_UIevent.UI_EventCounter;//EventCounter
	write_buf[index++] = g_UIevent.UI_EventType;//EventType
	write_buf[index++] = g_UIevent.UI_EventValue;//EventValue
	for (u8 i = 0; i < g_UIevent.UI_datalen; i++){
		write_buf[index++] = g_UIevent.UI_EventData[i];
	}
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("%x ", write_buf[i]);
	}
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("write_buffer_to_board type:%d,value:%d,datalen:%d\n", g_UIevent.UI_EventType, g_UIevent.UI_EventValue, g_UIevent.UI_datalen);

#ifdef USE_BUS
	replysize = index + 3;//01 c0 c1
#endif
	writeDatatoUart(write_buf, index);
}

void write_buffer_to_board_withoutValue(){//flexible
	u8 write_buf[128] = { 0 };
	u8 len = 0;
	u8 index = 0;
	u16 crc = 0;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = WRITE_ADDR;
	index++;//for write len
	write_buf[index++] = g_UIevent.UI_EventCounter;			//EventCounter
	write_buf[index++] = g_UIevent.UI_EventType;//EventType
	for (u8 i = 0; i < g_UIevent.UI_datalen; i++){
		write_buf[index++] = g_UIevent.UI_EventData[i];
	}
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("write_buf[%d] is %x\t", i, write_buf[i]);
	}
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	printf("write_buffer_to_board type:%d,value:%d,datalen:%d ", g_UIevent.UI_EventType, g_UIevent.UI_EventValue, g_UIevent.UI_datalen);
	printf("write_buf:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("\n");

#ifdef USE_BUS
	replysize = index + 3;//01 c0 c1   我发送的加上对方确认的3个字节
#endif
	writeDatatoUart(write_buf, index);
}
void write_buffer_struct_to_board(void* data, u8 len){//flexible
	u8 write_buf[128] = { 0 };
	u8 index = 0;
	u16 crc = 0;
	u8* ptr = data;
	write_buf[index++] = WRITECMD;
	write_buf[index++] = WRITE_ADDR;
	index++;//for write len
	write_buf[index++] = 0;			//EventCounter
	for (u8 i = 0; i < len; i++){
		write_buf[index++] = ptr[i];
	}
	write_buf[2] = index - 3;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//printf("write_buf[%d] is %x\t", i, write_buf[i]);
	}
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	printf("write_buffer_struct_to_board ");
	printf("write_buf:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("\n");

#ifdef USE_BUS
	replysize = index + 3;//01 c0 c1   我发送的加上对方确认的3个字节
#endif
	writeDatatoUart(write_buf, index);
}

void writeQueryCmdToBoard(DataOffset offset, u8 size){//query  82 00 20 F0 A1  82 20 crc1 crc2 
	u8 index = 0;
	write_buf[index++] = ((u8)((offset >> 8) | 0x80)) + 1;//170 3 -> 82 70 03
	write_buf[index++] = (u8)offset;
	write_buf[index++] = size;

	u16 crc = 0;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
		//UPS_LOG"write_buf[%d] is %x\t", i, write_buf[i] LOG_END
	}
	//UPS_LOG"\n"LOG_END;
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);
	/*printf("writeQueryCmdToBoard:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", write_buf[i]);
	}
	printf("\n");*/
	
#ifdef USE_BUS
	replysize = size + 3 + index;//part +crc16 size+3+5   我发送的加上 我要查询的长度  再加上3个字节 一个头两个校验
#endif
	writeDatatoUart(write_buf, index);
}

void writeQueryCmdToBoardWithNoLog(DataOffset offset, u8 size){//query  82 00 20 F0 A1
	u8 index = 0;
	write_buf[index++] = ((u8)((offset >> 8) | 0x80)) + 1;//170 3 -> 82 70 03
	write_buf[index++] = (u8)offset;
	write_buf[index++] = size;

	u16 crc = 0;
	for (u8 i = 0; i < index; i++)
	{
		CalcCrc16(&crc, write_buf[i]);
	}
	write_buf[index++] = MSB(crc);
	write_buf[index++] = LSB(crc);

	WRITEBUS_LOG"writeQueryCmdToBoard ");
	WRITEBUS_LOG"write_buf:");
	for (u8 i = 0; i < index; i++)
	{
		WRITEBUS_LOG"%x ", write_buf[i]);
	}
	WRITEBUS_LOG"\n");

#ifdef USE_BUS
	replysize = size + 3 + index;//part +crc16 size+3+5   我发送的加上 我要查询的长度  再加上3个字节 一个头两个校验
#endif
	writeDatatoUart(write_buf, index);
}
///*
//********************************************************************************************************
//*/
//int writeDatatoUart(u8* data, u8 len)
//{
//	u8 wlen = 0;
//	int i = 0;
//#if 1//def RUISA_DEBUG
//	WRITEBUS_LOG"write to UART1:"LOG_END
//	for (; i < len; i++)
//		WRITEBUS_LOG"%x ", data[i]LOG_END;
//	WRITEBUS_LOG"\n"LOG_END;
//#endif
//	while (wlen < len)
//	{
//		wlen = write(ITP_DEVICE_UART1, data + wlen, len - wlen);
//		WRITEBUS_LOG"total is %d  write size is %d \n\n", len, wlen LOG_END
//	}
//	return wlen;
//}
//int writeDatatoUartWithNoLog(u8* data, u8 len)
//{//饱和查询时调用，不打log
//	u8 wlen = 0;
//	int i = 0;
//	while (wlen < len)
//	{
//		wlen = write(ITP_DEVICE_UART1, data + wlen, len - wlen);
//	}
//	return wlen;
//}

/*
NULLCMD RESETCMD IDLECMD
01 00 02 00 0xcmdType 00 9c
*/
//void writeSingleCmdToBoard(UI_EVENT_TYPE cmdType)
//{
//	u8 len = 0;
//	u8 index = 0;
//	u16 crc = 0;
//	dealData.write_buf[index++] = WRITE_CMD;
//	dealData.write_buf[index++] = WRITE_ADDR;
//	index++;//for write len
//	dealData.write_buf[index++] = 0x0;
//	dealData.write_buf[index++] = (u8)cmdType;
//	dealData.write_buf[2] = index - 3;
//	for (u8 i = 0; i < index; i++)
//	{
//		CalcCrc16(&crc, dealData.write_buf[i]);
//#ifdef WRITEBUS_LOG
//		printf("write_buf[%d] is %x\t", i, dealData.write_buf[i]);
//#endif
//	}
//#ifdef WRITEBUS_LOG
//	printf("\n");
//#endif
//	dealData.write_buf[index++] = MSB(crc);
//	dealData.write_buf[index++] = LSB(crc);
//
//	writeDatatoUart(dealData.write_buf, index);
//}

//void writeDoubleCmdToBoard(UI_EVENT_TYPE type, u8 value)
//{// type and value
//	u8 len = 0;
//	u8 index = 0;
//	u16 crc = 0;
//	dealData.write_buf[index++] = WRITE_CMD;
//	dealData.write_buf[index++] = WRITE_ADDR;
//	index++;//for write len
//	dealData.write_buf[index++] = 0;			//EventCounter
//	dealData.write_buf[index++] = (u8)type;	//EventType
//	dealData.write_buf[index++] = (u8)value;	//EventValue
//	dealData.write_buf[2] = index - 3;
//	for (u8 i = 0; i < index; i++)
//	{
//		CalcCrc16(&crc, dealData.write_buf[i]);
//#ifdef WRITEBUS_LOG
//		printf("write_buf[%d] is %x\t", i, dealData.write_buf[i]);
//#endif
//	}
//#ifdef WRITEBUS_LOG
//	printf("\n");
//#endif
//	dealData.write_buf[index++] = MSB(crc);
//	dealData.write_buf[index++] = LSB(crc);
//
//	writeDatatoUart(dealData.write_buf, index);
//}

//void write_buffer_to_board()
//{//flexible
//	u8 len = 0;
//	u8 index = 0;
//	u16 crc = 0;
//	dealData.write_buf[index++] = WRITE_CMD;
//	dealData.write_buf[index++] = WRITE_ADDR;
//	index++;//for write len
//	dealData.write_buf[index++] = g_UIevent.UI_EventCounter;			//EventCounter
//	dealData.write_buf[index++] = g_UIevent.UI_EventType;//EventType
//	dealData.write_buf[index++] = g_UIevent.UI_EventValue;//EventValue
//	for (u8 i = 0; i < g_UIevent.UI_datalen; i++){
//		dealData.write_buf[index++] = g_UIevent.UI_EventData[i];
//	}
//	dealData.write_buf[2] = index - 3;
//	for (u8 i = 0; i < index; i++)
//	{
//		CalcCrc16(&crc, dealData.write_buf[i]);
//		//#ifdef RUISA_DEBUG
//		//		printf("write_buf[%d] is %x\t", i, dealData.write_buf[i]);
//		//#endif
//	}
//	//	printf("\n");
//	dealData.write_buf[index++] = MSB(crc);
//	dealData.write_buf[index++] = LSB(crc);
//
//	writeDatatoUart(dealData.write_buf, index);
//}


//void writeQueryCmdToBoard(DataOffset offset, u8 size)
//{//query  82 00 20 F0 A1
//	u8 index = 0;
//	dealData.write_buf[index++] = ((u8)((offset >> 8) | 0x80)) + 1;//170 3 -> 82 70 03
//	dealData.write_buf[index++] = (u8)offset;
//	dealData.write_buf[index++] = size;
//
//	u16 crc = 0;
//	for (u8 i = 0; i < index; i++)
//	{
//		CalcCrc16(&crc, dealData.write_buf[i]);
//		//UPS_LOG"write_buf[%d] is %x\t", i, dealData.write_buf[i] LOG_END
//	}
//	//UPS_LOG"\n"LOG_END
//	dealData.write_buf[index++] = MSB(crc);
//	dealData.write_buf[index++] = LSB(crc);
//
//	writeDatatoUartWithNoLog(dealData.write_buf, index);
//}
/*
********************************************************
*/


void readDataFromBoard(DataOffset offset, u8 size)
{
	u8 index = 0;
	u8 i = 0;
	u8 wlen = 0;
	u16 crc = 0;
	u8 buf[6] = { 0 };

	buf[index++] = ((u8)((offset >> 8) | 0x80)) + 1;//170 3 -> 82 70 03
	buf[index++] = (u8)offset;
	buf[index++] = size;

	for (i = 0; i < index; i++)
	{
		CalcCrc16(&crc, buf[i]);
	}
	buf[index++] = MSB(crc);
	buf[index++] = LSB(crc);
#if 0//debug
	printf("readDataFromBoard:");
	for (u8 i = 0; i < index; i++)
	{
		printf("%x ", buf[i]);
	}
	printf("\n");
#endif
#ifdef USE_BUS
	replysize = size + 3 + index;//part +crc16
#endif
	wlen = writeDatatoUart(buf, index);
	memset(resendbuf, 0, 128);
	memcpy(resendbuf, write_buf, wlen);
}
void read_board_data(u8 resend)
{
	u8 read = 0;
	//RUISA_LOG"read_board_data mst_state: %d  \n", mst_state LOG_END;
	//RUISA_LOG"read_board_data resend: %d  \n", resend LOG_END;

	//if(qCount>0&&(resend==0)&&(mst_state !=MASTER_STATE_WRITE_CTRLFLOW_PACK)) //houda 0116
	DataOffset offset = 0x000;
	u8 size = 0;
	switch (mst_state)
	{
	case MASTER_STATE_READ_UI_LOGIC://UILENGTH
		offset = 0x100;
		size = 0x40;//0x32=50 0x60=96
		break;
	case MASTER_STATE_READ_RST_TIME:
		offset = 0x000;
		size = 0;
		break;
	case MASTER_STATE_READ_SPECIAL_DISPLAY:
		offset = 0x160;
		size = 16;
		break;
	case MASTER_STATE_READ_INFO:
		offset = 0x170;
		size = 5;
		break;
	case MASTER_STATE_READ_INFO_DATA:
		offset = 0x000;
		size = 0;
		break;
	case MASTER_STATE_READ_MODE_TEST:
		offset = 0x000;
		size = 0;
		break;
	case MASTER_STATE_READ_MAC_STATE:
		offset = addr600;
		size = 0x2E;//46个
		break;
	case MASTER_STATE_WTIRE_NULLCMD:
		break;
	case MASTER_STATE_READ_NORMAL_CMD://MASTER_STATE_WRITE_NORMAL_CMD:
	printf(" uqc.sendFlag  : %d\n",uqc.sendFlag );
		if (uqc.sendFlag){
			uqc.sendFlag = false;
			switch (uqc.CurQueryNum)
			{
			case QUERY_200_TO_22C_2C:
				writeQueryCmdToBoardWithNoLog(addr200, 0x48);
				break;
			case QUERY_300_TO_33C_3C:
				writeQueryCmdToBoardWithNoLog(addr300, 0x3C);
				break;
			case QUERY_400_TO_43C_3C:
				writeQueryCmdToBoardWithNoLog(addr400, 0x3c);
				break;
			case QUERY_43C_TO_468_2C:
				writeQueryCmdToBoardWithNoLog(addr43C, sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT));//4*8 = 32
				break;
			case QUERY_500_TO_515_15:
				writeQueryCmdToBoardWithNoLog(addr500, 0x15);
				break;
			case QUERY_520_TO_545_25:
				writeQueryCmdToBoardWithNoLog(addr520, 0x1C);
				break;
			case QUERY_600_TO_52E_2E:
				writeQueryCmdToBoardWithNoLog(addr600, sizeof(WPEVENT_MACHINE_STATE));
				break;
			case QUERY_700_TO_711_12:
				writeQueryCmdToBoardWithNoLog(addr700, (sizeof(WPEVENT_MACHINE_RUN_STATE)));
				break;
			case QUERY_160_TO_170_10:
				writeQueryCmdToBoardWithNoLog(addr160, 0x10);
				break;
			case QUERY_170_TO_1FF_90:
				if (wpwi.MessageLength == 0){
					writeQueryCmdToBoardWithNoLog(addr170, 0x05);
				}
				else{
					writeQueryCmdToBoardWithNoLog(addr170, 0x05 + wpwi.MessageLength);
				}
				break;
			default:
				printf("ERROR");
				break;
			}
			return;
		}
		writeQueryCmdToBoardWithNoLog(addr160, 0x10);//出问题就查询一个短的 例如重发
		printf("ERROR:should not reach here");
		return;
	case MASTER_STATE_WRITE_NORMAL_CMD:
		switch (ev.type)
		{
		case WASH_Nc:
		case WASH_Reset:
		case WASH_ReturnIdle:
			writeSingleCmdToBoard(ev.type);
			break;
		case WASH_PowerButton:
		case WASH_Cycle:
		case WASH_CycleStart:
		case WASH_StartPause:
		case WASH_ResetParamters:
		case WASH_TestOptionsSwitch:
		case WASH_TstSpecialNum:
		case WASH_UniversalDialogBox:
		case WASH_LoadControlCommand:
			writeDoubleCmdToBoard(ev.type, ev.arg0);
			break;
		case WASH_WSCycle:
			writeDoubleCmdToBoard(ev.type, ev.arg0);
			break;
		case EXTRA_WASH_QUERY:
			if (ev.arg0 == ADDR100){
				writeQueryCmdToBoardWithNoLog(addr100, 0x40);//82 00 20
			}
			else if (ev.arg0 == (u8)ADDR200){
				writeQueryCmdToBoardWithNoLog(addr200, 0x48);//
			}
			else if ((ev.arg0 >= ADDR300) && (ev.arg0 <= ADDR300A54)){
				writeQueryCmdToBoardWithNoLog(addr300, 0x3C);//
			}
			else if (ev.arg0 == ADDR400){
				writeQueryCmdToBoardWithNoLog(addr400, 0x3c);//outQquery(ADDR43C);
			}
			else if (ev.arg0 == ADDR43C){
				writeQueryCmdToBoardWithNoLog(addr43C, sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT));//outQquery(ADDR43C);
			}
			else if (ev.arg0 == ADDR500){
				writeQueryCmdToBoardWithNoLog(addr500, 0x15);//outQquery(ADDR43C);
			}
			else if (ev.arg0 == ADDR520){
				writeQueryCmdToBoardWithNoLog(addr520, 0x1C);//outQquery(ADDR43C);
			}
			else if (ev.arg0 == ADDR600){
				writeQueryCmdToBoardWithNoLog(addr600, sizeof(WPEVENT_MACHINE_STATE));//
			}
			else if (ev.arg0 == ADDR160){
				writeQueryCmdToBoardWithNoLog(addr160, 0x10);//
			}
			break;
		case EXTRA_WRITE_BUFFER:
			switch (g_UIevent.UI_EventType)
			{
			case UE_SetRtc:
			case UE_SetMachineRunParameters:
			case UE_SetLoadContorlCommand:
				write_buffer_to_board_withoutValue();
				break;
			default:
				write_buffer_to_board();
				break;
			}
			break;
		case EXTRA_WRITE_BUFFER_STRUCT:
			write_buffer_struct_to_board((void*)ev.buf1, (u8)ev.arg3_buf1Size);
			break;
		default:
			//writeCmdToBoard(0,&g_UIevent);
			break;
		}
		return;
	default:
		return;
	}

	//if(resend)
	//RUISA_LOG "read : %s\n",logstr[mst_state] LOG_END
	
	if (offset == 0 || size == 0){
		printf("offset=%d size=%d\n", offset, size);
		return;
	}
	readDataFromBoard(offset, size);
}
void refreshUI(u8* data, int start, int length){
	if ((start + length) > sizeof(ui)){
		printf("out of UI length\n");
		return;
	}
	u8* UIPoint = (u8*)&ui;
	memcpy(UIPoint + start, data, length);
}
void printDataLog(u8* data, u8 num){
	for (u8 i = 0; i < num + 2; i++)//num + 2 将两个crc打印出来
	{
		printf("%0x ", data[i]);
	}
	printf("\n");
}

void checkAndSync160(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > sizeof(WPEVENT_UI_DATA160) ? sizeof(WPEVENT_UI_DATA160) : num;
	if (1){
		u8* point = (u8*)&ui_data160;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****UI160 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			//printf("*****change offset = %x*****\n", offset);// 82 60 offset = 60;
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_UI_DATA160)){
				printf("read size = %d struct WPEVENT_UI_DATA160 size = %d ", num, sizeof(WPEVENT_UI_DATA160));
			}
		}
	}
}
void checkAndSync200(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > sizeof(WPEVENT_PROGRAM_PARAMRTER) ? sizeof(WPEVENT_PROGRAM_PARAMRTER) : num;
	if (1){
		u8* point = ((u8*)&pp + offset);
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****PP200 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_PROGRAM_PARAMRTER)){
				printf("read size = %d struct WPEVENT_PROGRAM_PARAMRTER size = %d ", num, sizeof(WPEVENT_PROGRAM_PARAMRTER));
			}
		}
	}
}

void checkAndSync300(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > sizeof(WPEVENT_DISCOUNT_PARAMRTER) ? sizeof(WPEVENT_DISCOUNT_PARAMRTER) : num;
	if (1){
		u8* point = (u8*)&dp;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****DP300 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_DISCOUNT_PARAMRTER)){
				printf("read size = %d struct WPEVENT_DISCOUNT_PARAMRTER size = %d ", num, sizeof(WPEVENT_DISCOUNT_PARAMRTER));
			}
		}
	}
}

void checkAndSync400(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > 60 ? 60 : num;
	if (1){
		u8* point = (u8*)&ip.info1;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****IP400 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != 60){
				printf("read size = %d struct size = %d ", num, 60);
			}
		}
	}
}

void checkAndSync43C(u8* data, u8 num, u8 offset){
	bool change = false;
	//printDataLog(data, num);
	u8 smallSize = num > sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT) ? sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT) : num;
	if (1){
		u8* point = (u8*)&ip.count;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****IP43C i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT)){
				printf("read size = %d struct WPEVENT_INFO_PARAMRTER_TYPE_COUNT size = %d ", num, sizeof(WPEVENT_INFO_PARAMRTER_TYPE_COUNT));
			}
		}
	}
}

#ifdef LOG500DEBUG
#define PRINT_LOG_500    printf(
#else
#define PRINT_LOG_500   (void)(1 ? 0 :
#endif
void checkAndSync500(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > 0x15 ? 0x15 : num;
	if (1){
		u8* point = (u8*)&cp;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				PRINT_LOG_500"*****CP500 i = %d  %x change to %x*****\n", i, point[i], data[i]LOG_END
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
#ifdef LOG500DEBUG
			printDataLog(data, num);
#endif
			if (num != 0x15){//检验
				printf("read size = %d struct 500 size = %d ", num, 0x15);
			}
		}
	}
}

#define DATA520SIZE 0x1C
void checkAndSync520(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > DATA520SIZE ? DATA520SIZE : num;
	printf("cp.isShowTemp_OUT:%d\n",cp.isShowTemp_OUT);
	printf("cp.isShowTemp_in:%d\n",cp.isShowTemp_IN);
	if (1){
		u8* point = ((u8*)&cp + offset);
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****CP520 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_CTRL_PARAMRTER)){
				printf("read size = %d struct 520 size = %d ", num, DATA520SIZE);
			}
		}
	}
}

void checkAndSync600(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > sizeof(WPEVENT_MACHINE_STATE) ? sizeof(WPEVENT_MACHINE_STATE) : num;
	if (1){
		u8* point = (u8*)&ms;
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****MS600 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != sizeof(WPEVENT_MACHINE_STATE)){
				printf("read size = %d struct WPEVENT_MACHINE_STATE size = %d ", num, sizeof(WPEVENT_MACHINE_STATE));
			}
		}
	}
}

bool checkAndSyncUIData(u8* data, u8 num, u8* localData){
	bool change = false;
	u8* point = localData;
	for (size_t i = 0; i < num; i++)
	{
		if (point[i] != data[i]){
			change |= true;
			//printf("*****************************\n");
			printf("*****i = %d  %x change to %x*****\n", i, point[i], data[i]);
			//printf("*****************************\n");
			point[i] = data[i];
		}
	}
	UIDataChange |= change;
	return change;
}
void refreshTestModeLayer();

void syncUIDataAndDeal(u8* data, u8 num){//uilogic复用
	if (data[1] >= 0xF0){
		if (checkAndSyncUIData(data, num, (u8*)&ui_tm)){
			ui.CycleName = ui_tm.CycleName;
			ui.KeyValue1 = ui_tm.KeyValue1;
			ui.KeyValue2 = ui_tm.KeyValue2;
			printf("FN %d DATA:", num);//normal
			refreshTestModeLayer();
			printDataLog(data, num);
		}
	}
	else{
		if (checkAndSyncUIData(data, num, (u8*)&ui)){
			ui_tm.CycleName = 0;//使得下一次进入ui_tm时，能够有数字变化
			printf("NM %d DATA:", num);//normal
			//refreshStartLayer();
			printDataLog(data, num);
		}
	}
	//switch (*(data + 1))//WP_EVENT_UI_LOGIC CycleName
	//{
	//case 0xFA:
	//	if (checkAndSyncUIData(data, num, (u8*)&ui)){
	//		printf("FA %d DATA:",num);
	//		printDataLog(data, num);
	//	}
	//	break;
	//case 0xFB:
	//	if (checkAndSyncUIData(data, num, (u8*)&ui_tm)){
	//		ui.CycleName = ui_tm.CycleName;
	//		refreshTestModeLayer();
	//		printDataLog(data, num);
	//	}
	//	break;
	//case 0xFD:
	//	if (checkAndSyncUIData(data, num, (u8*)&ui_tm)){
	//		ui.CycleName = ui_tm.CycleName;
	//		refreshTestModeLayer();
	//		printDataLog(data, num);
	//	}
	//	break;
	//case 0xFE:
	//	if (checkAndSyncUIData(data, num, (u8*)&ui_tm)){
	//		ui.CycleName = ui_tm.CycleName;
	//		refreshTestModeLayer();
	//		printDataLog(data, num);
	//	}
	//	break;
	//default:
	//	if (checkAndSyncUIData(data, num, (u8*)&ui)){
	//		printf("NM %d DATA:", num);//normal
	//		printDataLog(data, num);
	//	}
	//	break;
	//}
}
bool checkAndSync600Data(u8* data, u8 num, u8* recData){
	bool change = false;
	u8* point = recData;
	for (size_t i = 0; i < num; i++)
	{
		if (point[i] != data[i]){
			change |= true;
			//printf("*****************************\n");
			printf("*****MS600 i = %d  %x change to %x*****\n", i, point[i], data[i]);
			//printf("*****************************\n");
			point[i] = data[i];
		}
	}
	return change;
}

void sync600DataAndDeal(u8* data, u8 num){//
	if (checkAndSync600Data(data, num, (u8*)&ms)){
		printDataLog(data, num);
	}
}
void checkAndSync700(u8* data, u8 num, u8 offset){
	bool change = false;
	u8 smallSize = num > (sizeof(WPEVENT_MACHINE_RUN_STATE)) ? (sizeof(WPEVENT_MACHINE_RUN_STATE)) : num;
	u8* point = ((u8*)&mrs);
	printf("%s\n",__FUNCTION__);

	if (1){
		for (size_t i = 0; i < smallSize; i++)
		{
			if (point[i] != data[i]){
				change |= true;
				//printf("*****************************\n");
				printf("*****MRS700 i = %d  %x change to %x*****\n", i, point[i], data[i]);
				//printf("*****************************\n");
				point[i] = data[i];
			}
		}
		if (change){
			printDataLog(data, num);
			if (num != (sizeof(WPEVENT_MACHINE_RUN_STATE))){
				printf("read size = %d struct 700 size = %d ", num, sizeof(WPEVENT_MACHINE_RUN_STATE));
			}
		}
	}
}

bool checkOutQueue(){

	if (mq_receive(extOutQueue, (char*)&ev, sizeof(OutExternalEvent), 0) > 0){
		printf("checkOutQueue type :%d\n",ev.type);
		return true;
	}
	return false;
}
static void printReadData(u8 len){
	for (u8 i = 0; i < len; i++)//emeng test
	{
		READBUS_LOG"%02x ", uart0readData[i] LOG_END
	}

	READBUS_LOG"=> recvlen=%d mst_state=%d\n", len, mst_state LOG_END;
}
void* Uart1In_Task(void* arg)
{
	uint8_t len = 0;
	uint8_t  readcount = 0;
	u16 tcrc = 0;
	bool resendflag = 0;
	static u8 recvlen = 0;
	u8 resendtimes = 0;
	MASTER_STATE_TYPE oldstate;
	uint8_t temp[] = { 0x01, 0x00, 0x02, 0x00, 0x00, 0x00, 0x9c, 0x01, 0xc0, 0xc1 };
	static struct timespec last_report_tm = { 0 };
#ifdef WIN32
	memcpy(uart0readData, temp, sizeof(temp));
	recvlen = sizeof(temp);
#endif
	memset(para_rsttime, 0x0, 64); //

	memset(para_uilogic, 0x0, 64);  //

	memset(para_testmode, 0x0, 96);//

	memset(para_special_display, 0, 16);//

	memset(para_info, 0x0, 128);//
	//usleep(50000);
	write_NullCmd_To_Board();
	oldstate = mst_state;
	//clock_gettime(CLOCK_MONOTONIC, &last_report_tm);
	usleep(5000);
#ifdef RUISA_DEBUG
	printf("Uart0In_Task in loop ===================\n ");
#endif

	while (1)
	{
		//UIEVENT uievt;
		OutExternalEvent ev = { 0 };
		int data_len = 0;
		if (0 == clock_gettime(CLOCK_MONOTONIC, &last_report_tm))
		{
		}
		memset(readbuf, 0, 128);
#ifndef WIN32
		len = read(ITP_DEVICE_UART1, readbuf, 128);
#else
		len = 1;
#endif
		readcount++;
		if (len > 0)
		{//debug
			readcount = 0;//读到数据清除记录
			int i = 0;

			printf( "(%d %d)", replysize, len );//emeng test

			for (i = 0; i< len; i++)
			{
			printf("%02x ", readbuf[i] LOG_END
			}

			printf("=> mst_state is %d\n", mst_state );

			if (resendflag)
				resendflag = 0;
			memcpy(uart0readData + recvlen, readbuf, len);
#ifndef WIN32
			recvlen += len;
#endif
#ifdef RUISA_DEBUG
			//printf( "inc recvlen = %d \n",recvlen ); 
#endif
#ifdef WIN32
			memcpy(uart0readData, temp, sizeof(temp));
			//recvlen = sizeof(temp);
			recvlen = replysize;
#endif

			if (recvlen >= replysize)
			{
				//printReadData(recvlen);//test

				uint8_t count = 0;
				bool find = false;//crc
				if (recvlen > replysize){
					RUISA_LOG " recvlen = %d  replysize =%d \n", recvlen, replysize LOG_END;
					printReadData(recvlen);
				}
				while (count <= recvlen - replysize)//假设需要10，收到20，依次验证，一般不会出现
				{
					//RUISA_LOG " in while count is : %d \n",count LOG_END 
					int read_size = uart0readData[2];
					int sendnum = read_size + 5;//
					int offset = sendnum + count;
					switch (mst_state)
					{
					case MASTER_STATE_WTIRE_NULLCMD:// 01 00 02 00 00 00 9c 01 c0 c1

						printReadData(recvlen);
						if (uart0readData[offset] == WRITECMD)
						{
							tcrc = (uart0readData[offset + 1] << 8) | uart0readData[offset + 2]; //houda add 0806
							if (checkCRC(uart0readData + offset, 1, tcrc)) //donot check  01 -> c0c1
							{
								find = true;
								printf("NULLCMD checkCRC PASS!\n");
							}
						}
						break;
					case MASTER_STATE_WRITE_NORMAL_CMD:
					case MASTER_STATE_WRITE_CTRLFLOW_PACK:////houda add 0114
						if (uart0readData[offset] == WRITECMD)//01 c0 c1
						{
							tcrc = (uart0readData[offset + 1] << 8) | uart0readData[offset + 2]; //houda add 0806
							if (checkCRC(uart0readData + offset, 1, tcrc)) //check  01 -> c0c1
							{
								find = true;
								//printf("checkCRC PASS!\n");
							}
							else{
								printf("checkCRC MASTER_STATE_WRITE_NORMAL_CMD WRITECMD fail\n");
								printReadData(recvlen);
							}
						}
						else{//01 00 15 00 0E 00(高温) +18 +2
							sendnum = 5;
							offset = sendnum + count;
							read_size = uart0readData[2];
							tcrc = (uart0readData[offset + read_size + 1] << 8) | uart0readData[offset + read_size + 2]; //houda add 0806
							if (checkCRC(uart0readData + offset, read_size + 1, tcrc)) //no check  01 -> c0c1
							{
								find = true;
							}
							else{
								printf("offset%d read_size%d tcrc%d\n", offset, read_size, tcrc);
								printReadData(recvlen);
								printf("checkCRC MASTER_STATE_WRITE_NORMAL_CMD read fail\n");
							}
						}
						break;

					case MASTER_STATE_READ_RST_TIME:
					{
													   if (uart0readData[count] == 0x81)
													   {
														   tcrc = (uart0readData[count + readsize + 1] << 8) | uart0readData[count + readsize + 2];
														   //if(checkCRC(uart0readData+count,readsize+1,tcrc))//houda 
														   find = true;
													   }
					}
						break;
					case MASTER_STATE_READ_NORMAL_CMD:
					case MASTER_STATE_READ_UI_LOGIC:
					case MASTER_STATE_READ_SPECIAL_DISPLAY:
					case MASTER_STATE_READ_INFO:
					case MASTER_STATE_READ_INFO_DATA:
						sendnum = 5;
						offset = sendnum + count;
						read_size = uart0readData[2];
						if (uart0readData[offset] == uart0readData[0]/*0x82*/)
						{
							tcrc = (uart0readData[offset + read_size + 1] << 8) | uart0readData[offset + read_size + 2]; //houda add 0806
							if (checkCRC(uart0readData + offset, read_size + 1, tcrc)) //no check  01 -> c0c1
							{
								find = true;
							}
							else {
								printf("offset%d read_size%d tcrc%d\n", offset, read_size, tcrc);
								printReadData(recvlen);
								printf("checkCRC MASTER_STATE_READ_UI_LOGIC read fail\n");
							}
						}
						else if (uart0readData[offset] == 0x87){
							tcrc = (uart0readData[offset + read_size + 1] << 8) | uart0readData[offset + read_size + 2]; //houda add 0806
							if (checkCRC(uart0readData + offset, read_size + 1, tcrc)) //no check  01 -> c0c1
							{
								find = true;
							}
							else {
								printf("offset%d read_size%d tcrc%d\n", offset, read_size, tcrc);
								printReadData(recvlen);
								printf("checkCRC MASTER_STATE_READ_UI_LOGIC0x87 read fail\n");
							}
						}
						break;
					}
#ifdef WIN32
					find = true;
#endif
					if (find)
					{
						busComOK = true;
						readcount = 0;
						//if(is_Normal_Mode())
						{
							//RUISA_LOG "find rsp   > count ---%d\n",count LOG_END
							switch (mst_state)
							{
							case MASTER_STATE_WTIRE_NULLCMD:
							{

												   oldstate = mst_state;
												   mst_state = MASTER_STATE_READ_UI_LOGIC;
							}
								break;
							case MASTER_STATE_READ_RST_TIME:
							{
															   // if(parse_RstTime(uart0readData+count+1))
								{
									oldstate = mst_state;
									mst_state = MASTER_STATE_READ_UI_LOGIC;
								}
							}
								break;
							case MASTER_STATE_READ_NORMAL_CMD:
																/*
								选择程序
								01 00 03 00 04 00 55 03 >> 01 C0 C1
								01 00 03 00 04 01 95 C2 >> 01 C0 C1
								01 00 03 00 04 02 94 82 >> 01 C0 C1

								启动
								01 00 03 00 06 01 F5 C3 >> 01 C0 C1

								暂停
								01 00 03 00 06 00 35 02 >> 01 C0 C1

								查询
								83 AA AA AA AA AA (count) 83 BB BB BB BB ...
								*/
								oldstate = mst_state;
								mst_state = MASTER_STATE_READ_UI_LOGIC;
								//printReadData(recvlen);
#if 0 //test 0x87
								{
									char data[] = { 0x87, 0x0, 0x2e, 0x35, 0x30, 0x87, 0x00, 0x00, 0x15, 0x14, 0x01, 0x00 \
										, 0x01, 0x02, 0x01, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01\
										, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01\
										, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01\
										, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01\
										, 0xff, 0xff };
									memcpy(uart0readData, data, 54);
								}
#endif
						
								switch (uart0readData[0])
								{
								case 0x82:
									if (uart0readData[1] == 0x60){//160
										checkAndSync160(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//82 60 0f 2c c8
									}
									break;
								case 0x83://200
									checkAndSync200(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//83 0 2c 35 f0
									break;
								case 0x84://300
									checkAndSync300(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//84 0 3C F0 38 40 
									break;
								case 0x85://400
									if (uart0readData[1] == 0x00){
										checkAndSync400(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//85 0 2C 34 10
									}
									else if (uart0readData[1] == 0x3C){
										checkAndSync43C(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//85 3c 3C 34 1
									}
									break;
								case 0x86://500
									if (uart0readData[1] == 0x00){//夏令时
										checkAndSync500(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//86 0 3e 39 60
									}
									else if (uart0readData[1] == 0x20){//一些设置
										checkAndSync520(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//86 0 3e 39 60
									}
									break;
								case 0x87://600
									//printReadData(recvlen);
									sync600DataAndDeal(&uart0readData[count + 6], uart0readData[count + 2]);
									//checkAndSyncMSData(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//87 0 2e 35 30
									break;
								case 0x88://700
									checkAndSync700(&uart0readData[count + 6], uart0readData[2], uart0readData[1]);//
									break;
								default:
									
									break;
								}
								break;
							case MASTER_STATE_READ_UI_LOGIC:
								/*
								*这里在高频下可能会出现杂波数据
								*正常数据为0x82 A A A A	(1ms)	0x82 B B B B ...
								*杂波数据为0x82 A A A A	(CCC)	0x82 B B B B ...
								*/
								if (uart0readData[0] == 0x82){
									if (uart0readData[count + 5] == 0x82){
										syncUIDataAndDeal(&uart0readData[count + 6], uart0readData[2]);//82 00 20 F0 A1 82
									}
									else{
										printf("[0]==0x82 count==%d [count+5]==0x%x\n", count, uart0readData[count + 5]);
										mst_state = MASTER_STATE_READ_UI_LOGIC;//重发
									}
								}
								else{
									/*
									理论上真机这里不走
									*/
#ifdef WIN32

#else
									printf("error uart0readData[0]==0x%x\n", uart0readData[0]);
#endif

									if (uart0readData[count + 5] == 0x82){
										syncUIDataAndDeal(&uart0readData[count + 6], 50);//82 00 20 F0 A1 82 
									}
									else{
										mst_state = MASTER_STATE_READ_UI_LOGIC;//重发
#ifndef WIN32
										break;
#endif
									}
								}
								//00 00 01 00 00 00 01 00
								//00 00 64 00 00 00 00 00 (0f)
								//00 00 00 00 00 00 00 00
								//00 00 00 00 00 00 00 00 (1f)
								//44 02
								bool writeFlag = checkOutQueue();//查询是否有写任务的队列
								//printf("write:type = %x\n",ev.type);
								if (writeFlag){
									oldstate = mst_state;
									mst_state = MASTER_STATE_WRITE_NORMAL_CMD;
									break;
								}
								UI_QUERY_NAME status = getQueryTarget();
								switch (status)
								{
								case QUERY_200_TO_22C_2C:
								case QUERY_300_TO_33C_3C:
								case QUERY_400_TO_43C_3C:
								case QUERY_43C_TO_468_2C:
								case QUERY_500_TO_515_15:
								case QUERY_520_TO_545_25:
								case QUERY_600_TO_52E_2E:
								case QUERY_160_TO_170_10:
								case QUERY_700_TO_711_12:
								case QUERY_170_TO_1FF_90:
									uqc.sendFlag = true;
									mst_state = MASTER_STATE_READ_NORMAL_CMD;
									//printf("%d", status);
									break;
								case QUERY_NONE:
								default:
									mst_state = MASTER_STATE_READ_UI_LOGIC;//
									//printf("%d", status);
									break;
								}
								break;
							case MASTER_STATE_READ_MAC_STATE:
								sync600DataAndDeal(&uart0readData[count + 6], uart0readData[count + 2]);

								{
									oldstate = mst_state;
									mst_state = MASTER_STATE_READ_UI_LOGIC;
								}
								break;
							case MASTER_STATE_READ_SPECIAL_DISPLAY:
							{
																	  oldstate = mst_state;
																	  mst_state = MASTER_STATE_READ_UI_LOGIC; //houda add test 0608
																	  //parse_Special_data(uart0readData+count+1);
							}
								break;
							case MASTER_STATE_READ_INFO:
							{
														   mst_state = MASTER_STATE_READ_UI_LOGIC;//meng disable MASTER_STATE_READ_INFO_DATA

							}
								break;
							case MASTER_STATE_READ_INFO_DATA:
							{
																oldstate = mst_state;
																//  parse_Infomation_data(uart0readData+1);
																mst_state = MASTER_STATE_READ_UI_LOGIC;
							}
								break;
							case MASTER_STATE_WRITE_NORMAL_CMD:
								oldstate = mst_state;
								mst_state = MASTER_STATE_READ_UI_LOGIC;//重发 
								break;
							default:
								break;
							}
						}
						//	else   //in test mode  
						//	{

						//	}
						//readcount = 0;
						recvlen = recvlen - count - replysize;
#ifdef RUISA_DEBUG
						//printf( "rest recvlen : %d\n",recvlen );
#endif
						if (recvlen > 0)
						{
							char buf[UARTMAXLEN] = { 0 };
							memcpy(buf, uart0readData + count + replysize, recvlen);
							memset(uart0readData, 0, UARTMAXLEN);
							memcpy(uart0readData, buf, recvlen);
						}
						else
						{
							memset(uart0readData, 0, UARTMAXLEN);
						}
						break;///houda break while 
					}
					else//if (find)
					{
						count++;
					}
				}
				//RUISA_LOG " find valid data :  %d --readcount %d-\n",find,readcount LOG_END
				printf( " oldstate :  %d --mst_state %d-\n",oldstate,mst_state );

				if (oldstate != mst_state)
				{
					//UIEVENT qheader;
					//RUISA_LOG "oldstate : %d  mst_state : %d \n",oldstate,mst_state LOG_END
					resendtimes = 0;//houda add 0810
					oldstate = mst_state;
					readcount = 0;
					//memset(uart0readData,0,UARTMAXLEN);

					if (!connflag)
					{
						connflag = true;
					}
					//stopBoard_connTimer();    
					read_board_data(0);
				}
				else
				{
					//count++;
					if (MASTER_STATE_WRITE_CTRLFLOW_PACK == mst_state)
					{
						readcount = 0; //houda add 016
						resendtimes = 0;//houda add 0810

						if (g_UIevent.UI_EventCounter > 255)//houda 0117
							g_UIevent.UI_EventCounter = 1;
						else
							g_UIevent.UI_EventCounter++; //houda add test 0117

						read_board_data(0);//houda add test 0117 0 -> 1
					}
				}

			}
		}
		else
		{
#ifdef RUISA_DEBUG
			//printf("UART0read 0:%d\n ",readcount);
#endif

			if (readcount >= 20/*12*/)//houda add 10* //houda add test 0117
			{
				busComOK = false;
				readcount = 0;
				resendflag = 1;
				recvlen = 0;
				//RUISA_LOG " readcount %d ---resendtimes: %d \n", readcount, resendtimes LOG_END;
				memset(uart0readData, 0, UARTMAXLEN);
				readcount = 0;

				busComOK = false;
				resendtimes++;
				RUISA_LOG ">%d<", resendtimes LOG_END;
				if (resendtimes >= 10)
				{
					switch (mst_state)
					{
					case MASTER_STATE_WTIRE_NULLCMD:
						mst_state = MASTER_STATE_READ_UI_LOGIC;
						break;
						{
#ifndef __UIDISPLAY_BY_BOARDSTATE__ //houda attention 0606
							mst_state = MASTER_STATE_READ_RST_TIME;
#else
							mst_state = MASTER_STATE_READ_UI_LOGIC;
#endif
						}
						break;
					case MASTER_STATE_READ_RST_TIME:
					{
													   //tcrc = (uart0readData[readsize+1]<<8)|uart0readData[readsize+2];
													   mst_state = MASTER_STATE_READ_UI_LOGIC;
					}
						break;
					case MASTER_STATE_READ_UI_LOGIC:
						if (spcialdisplay)
						{
							mst_state = MASTER_STATE_READ_SPECIAL_DISPLAY;
						}
						else
						{
							mst_state = MASTER_STATE_READ_INFO;
						}
						break;
					case MASTER_STATE_READ_SPECIAL_DISPLAY:
						mst_state = MASTER_STATE_READ_UI_LOGIC; //houda modi  MASTER_STATE_READ_INFO 0808
						break;
					case MASTER_STATE_READ_INFO:
					{
												   mst_state = MASTER_STATE_READ_UI_LOGIC;
					}
						break;
					case MASTER_STATE_WRITE_NORMAL_CMD:
						mst_state = MASTER_STATE_READ_UI_LOGIC;
						break;
					case MASTER_STATE_WRITE_CTRLFLOW_PACK:
					{
															 //sendcycledata =false;//houda add 
															 mst_state = MASTER_STATE_READ_UI_LOGIC;
					}
						break;
					default:
						mst_state = MASTER_STATE_READ_UI_LOGIC;
						break;
					}
					resendtimes = 0; //houda add 
					RUISA_LOG " resend time reach max---change to mst_state:%d\n", mst_state LOG_END;
				}

				read_board_data(resendflag); //houda modi
				resendflag = 0;
				//	if(connflag)
				//	start_boardconn_timer();
			}
		}
		//      RUISA_LOG " sleep 5000---" LOG_END
#if 0
		if (0 == clock_gettime(CLOCK_MONOTONIC, &tsnow))
		{
			long msec = 0;
			//msec  = (long)(now.tv_sec - tsnow.tv_sec) * 1000;
			msec = (long)((tsnow.tv_nsec - last_report_tm.tv_nsec) / 1000);
			printf("cycle time : %ld\n", msec);
		}
#endif
		usleep(5000);//houda change from 5 to 2
	}
	return NULL;
}

void boardthreadInit(){

	/*CreateWorkerThread(readBusTask, NULL);
	CreateWorkerThread(writeToBusTask, NULL);
	CreateWorkerThread(External500msTask, NULL);*/
	//CreateWorkerThread(External1000msTask, NULL);
	CreateWorkerThread(Uart1In_Task, NULL);
}




void threadInit(){
	//CreateWorkerThread(readBusTask, NULL);
}


void queueInit(){
	struct mq_attr qattr;
	qattr.mq_flags = 0;
	qattr.mq_maxmsg = EXT_MAX_QUEUE_SIZE;
	qattr.mq_msgsize = sizeof(OutExternalEvent);
	extInQueue = mq_open("external_in", O_CREAT | O_NONBLOCK, 0644, &qattr);
	assert(extInQueue != -1);
	extOutQueue = mq_open("external_out", O_CREAT | O_NONBLOCK, 0644, &qattr);//开启与底板线程A
	assert(extOutQueue != -1);
	extPMQueue = mq_open("external_pm", O_CREAT | O_NONBLOCK, 0644, &qattr);//生产模式队列
	assert(extPMQueue != -1);
}
//开启线程(boardstate线程)+初始化队列extOutQueue
void OutExternalInit(void)
{
	extQuit = false;//关闭后无法发送结构体到队列中
	//CreateWorkerThread(TimerTask, NULL);
	queueInit();
	boardthreadInit();
}
