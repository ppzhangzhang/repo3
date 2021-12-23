#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

#include "layer_ctrl.h"
#include "projutils.h"
#include "boardstate.h"

/*
CLOSEFCUERR		关闭FCU 显示板与流程板通讯报警
SCREEN_LOG		屏幕打印关键数据 比如UI数据
*/
//#define CLOSEFCUERR
#define SCREEN_LOG
//#define CLOSEALLERR

ITULayer* mainLayer;
ITULayer* startLayer;
ITULayer* prodmodLayer;
ITULayer* testModeLayer;
ITULayer* warnLayer;
ITULayer* gototestLayer;

ITULayer* specialModeLayer;

ITULayer* logLayer; ITUText* TextLogLine1, *TextLogLine2;

typedef enum
{
	English = 0,
	xibanya,//西班牙
	fayu,//法语
}UI_CTRL_LANG;//zpp




u16 ctrl_timer = 0;//用来实现闪烁效果的时间戳
//bool isbackfromservicemode = false;


extern void processWarn();

CtrlLayerType ctrlui = { 0 };

//dealDataType dealData = { 0 };
/*
prodmode
*/
extern void initAdmin();
static bool isinTestMode = false;
extern void ProdmodReset();
extern void gotoprodmodLayer();

void gotoCustomerLayer(){//3寸干衣机，不需要跳转选择
	boardsetCycleCMD(HIGH);
	if (theConfig.langselect == 0){//没有选择语言
		ituLayerGoto(startLayer);
	}
	else{//已经选择语言，直接到temp选择
		ituLayerGoto(startLayer);
	}
}

void gotogototestLayer(){
	printf("gotogototestLayer\n");
	//boardsetCycleCMD(cx_tst_ui);//流程板静音模式//houda modi 
	isinTestMode = true;
	initAdmin();
	ituLayerGoto(gototestLayer);
}

void gotogototestLayerandprodmodLayer(){
	//gotogototestLayer();
	boardsetCycleCMD(cx_tst_ui);//流程板静音模式//houda modi 1203
	gotoprodmodLayer();
}
void exitProdLayer(){
	printf("exitProdLayer\n");
	gotoCustomerLayer();
	initAdmin();
	isinTestMode = false;
	{//初始化
		ProdmodReset();
	}
}
/*
prodmodeend
*/
void outQsetType(u8 type)
{
	OutExternalEvent ev = { 0 };
	UPS_LOG "outQsetType Type:%d\n", type LOG_END;
	ev.type = type;
	ExternalOutSend(&ev);
}

void outQsetTypeValue(u8 type, u8 value)
{
#ifdef WIN32
	if (type == UE_Cycle){
		ui.CycleName = value;
		ui.RunMode = RM_STANDBY;
	}
	if (type == UE_StartPause){
		if (value == START){
			ui.RunMode = RM_RUNNING;
		}
		if (value == PAUSE){
			ui.RunMode = RM_PAUSE;
		}
	}
#endif
	OutExternalEvent ev = { 0 };
	UPS_LOG "outQsetTypeValue Type:%d value: %x\n", type, value LOG_END;
	ev.type = type;
	ev.arg0 = value;
	ExternalOutSend(&ev);
}

void boardsetCycleCMD(u8 ct)
{
#ifdef WIN32
	ui.CycleName = ct;
#endif
	outQsetTypeValue(UE_Cycle, ct);
}
void outQsetBuffer(u16 address, u8 datalen, u8* data)
{
	OutExternalEvent ev = { 0 };
	ev.type = EXTRA_WRITE_BUFFER;
	setU16(&ev.arg0, address);
	ev.arg3_buf1Size = datalen;
	memcpy(&ev.buf1, data, datalen);
	ExternalOutSend(&ev);
}
void outQsetOther(u8 type, u8 location)
{
	OutExternalEvent ev = { 0 };
	UPS_LOG "sendMsgtoOUTQ Type:%d arg0: %x\n", type, location LOG_END;
	ev.type = type;
	ev.arg0 = location;
	ExternalOutSend(&ev);
}
/*
ADDR100
*/
void outQquery(u16 qt)
{
	OutExternalEvent ev = { 0 };
	//UPS_LOG "outQquery QueryType:%d \n", qt LOG_END;
	ev.type = EXTRA_WASH_QUERY;
	setU16(&ev.arg0, qt);
	ExternalOutSend(&ev);
}
void out_write_buffer_to_board(){
	OutExternalEvent ev = { 0 };
	UPS_LOG "out_write_buffer_to_board\n" LOG_END;
	ev.type = EXTRA_WRITE_BUFFER;
	ExternalOutSend(&ev);
}
void out_write_buffer_struct_to_board(void*data, u8 len){
	OutExternalEvent ev = { 0 };
	ev.arg3_buf1Size = len;
	memcpy(ev.buf1, data, len);
	UPS_LOG "out_write_buffer_struct_to_board\n" LOG_END;
	ev.type = EXTRA_WRITE_BUFFER_STRUCT;
	ExternalOutSend(&ev);
}

bool busComOK = false;
bool bluetoothOK = false;
u8 backData[OVEN_MAX_LEN] = { 0 };
#define READSLEEPTIME 2 //ms
#define WAITDATATIMES (70/READSLEEPTIME)
readStructType readStruct = { READSLEEPTIME, { 0 }, 0, 0, true, false, false };

//void refreshUI(u8* data, int start, int length){
//	if ((start + length) > sizeof(ui)){
//		printf("out of UI length\n");
//		return;
//	}
//	u8* UIPoint = (u8*)&ui;
//	memcpy(UIPoint + start, data, length);
//}

//void printDataLog(u8* data, u8 num){
//	//for (u8 i = 0; i < num; i++)
//	//{
//	//	printf("%0x ", *(data + i));
//	//}
//	//printf("\n");
//
//	for (int i = 0; i < dealData.totalDataLen; i++)
//	{
//		printf("%0x ", dealData.resendbuf[i]);
//	}
//	printf("\n");
//}

//void checkAndSyncTimeAndPriceData(u8* data, u8 num, u8 offset){
//	bool change = false;
//	u8* point = ((u8*)&tAndp + offset);
//	for (size_t i = 0; i < num; i++)
//	{
//		if (*(point + i) != *(data + i)){
//			change |= true;
//			//printf("*****************************\n");
//			printf("*****i = %d  %x change to %x*****\n", i, *(point + i), *(data + i));
//			//printf("*****************************\n");
//			*(point + i) = *(data + i);
//		}
//	}
//	if (change){
//		printf("*****change offset = %x*****\n", offset);;
//		refreshMainLayer();
//		printDataLog(data, num);
//	}
//}
//bool checkAndSyncUIData(u8* data, u8 num, u8* recData){
//	bool change = false;
//	u8* point = (u8*)&ui;
//	for (size_t i = 0; i < num; i++)
//	{
//		if (*(point + i) != *(data + i)){
//			change |= true;
//			//printf("*****************************\n");
//			printf("*****i = %d  %x change to %x*****\n", i, *(point + i), *(data + i));
//			//printf("*****************************\n");
//			*(point + i) = *(data + i);
//		}
//	}
//	return change;
//}
//void syncUIDataAndDeal(u8* data, u8 num){//uilogic复用
//	switch (*(data + 1))//WP_EVENT_UI_LOGIC CycleName
//	{
//	case 0xFD:
//		if (checkAndSyncUIData(data, num, (u8*)&ui_tm)){
//			refreshTestModeLayer();
//			printDataLog(data, num);
//		}
//		break;
//	default:
//		if (checkAndSyncUIData(data, num, (u8*)&ui)){
//			refreshStartLayer();
//			printDataLog(data, num);
//		}
//		break;
//	}
//}
/*
判断CRC校验
*/
void processingData(){
	////checkCRC();
	//if (dealData.sendDataLen == 7){//EXTERNAL_DRY_NULL or RESET
	//	if (dealData.resendbuf[4] == 0x00 && dealData.resendbuf[7] == 0x01){
	//		busComOK = true;
	//	}
	//}
	//else if (dealData.sendDataLen == 8){//EXTERNAL_DRY_NORMAL

	//}
	//else if (dealData.sendDataLen == 5){//EXTERNAL_DRY_QUERYdealData.resendbuf[0]
	//	u16 address = (((((u16)dealData.resendbuf[0] & 0x0f) - 1) << 8) | dealData.resendbuf[1]);
	//	switch (address)
	//	{
	//	case addr100://将UI LOGIC数据取出，将通讯数据转化成名为“底板状态”的存储数据
	//		syncUIDataAndDeal(&dealData.resendbuf[6], dealData.resendbuf[2]);//82 00 20 xx xx 82 
	//		//memset(backData, 0, OVEN_MAX_LEN);
	//		//memcpy(backData, &dealData.resendbuf[6], dealData.resendbuf[2]);//复制数据
	//		//可以加一个判断，判断是否数据有变化。
	//		//refreshUI(backData, 0, dealData.resendbuf[2]);

	//	case addr170:
	//		memcpy(backData, &dealData.resendbuf[6], dealData.resendbuf[2]);//复制数据
	//		//可以加一个判断，判断是否数据有变化。
	//		//refreshWPWinformation(backData, 0, dealData.resendbuf[2]);
	//		break;
	//	case addr200:
	//	case addr200 + 0x12:
	//	case addr200 + 0x24:
	//	case addr200 + 0x36:
	//		checkAndSyncTimeAndPriceData(&dealData.resendbuf[6], dealData.resendbuf[2], address - addr200);
	//		//memcpy(backData, &dealData.resendbuf[6], dealData.resendbuf[2]);
	//		//可以加一个判断，判断是否数据有变化。
	//		//refreshTAndP(backData, dealData.resendbuf[1] / 0x12, dealData.resendbuf[2]);// 0 = HIGH
	//		//refreshMainLayer();
	//		//
	//		break;
	//	case addr500:
	//		switch (dealData.resendbuf[1])
	//		{
	//		case 0x0://SetRtc;//21
	//			//refresh_CTRLPARA_Rtc();
	//			//refresh
	//			break;
	//		case 0x20://SetMachineRunParameters;//28
	//			//refresh_CTRLPARA_MachineRunParameters();
	//			//refresh
	//			break;
	//		default:
	//			break;
	//		}
	//		break;
	//	default:
	//		break;
	//	}

	//}
	//readStruct.busTasking = false;
	//readStruct.busHaveTask = false;
}
void gotoprodmodeLayer(){
	/*exitNormalModeQuit = 1;
	exitProductModeQuit = 0;
	usleep(MS_PER_FRAME);
	ITULayer* prodmodLayer = ituSceneFindWidget(&theScene, "prodmodLayer");
	assert(prodmodLayer);
	ituLayerGoto(prodmodLayer);*/
}
u8 checkProdModFlag = 0;
bool maybeGotoProdMod = false;
char str_promod_content[7] = { 0 };
u8 maybeGotoProdModNum = 0;
void checkConfirmGotoProdMod(){
	char real_promod_content[7] = { 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };
	for (int i = 0; i < sizeof(real_promod_content); i++){
		if (str_promod_content[i] != real_promod_content[i]){
			return;
		}
	}
	gotoprodmodeLayer();
}
void checkProdMod(u8 readbuf){
	if (!maybeGotoProdMod){//先走这里，符合0xA5, 0xA5, 0x0C, 0x33后，再比对剩下的
		u8 str4[4] = { 0xA5, 0xA5, 0x0C, 0x33 };
		if (readbuf == str4[checkProdModFlag]){
			checkProdModFlag++;
			if (checkProdModFlag == sizeof(str4)){
				checkProdModFlag = 0;
				maybeGotoProdMod = true;
				memset(str_promod_content, 0, sizeof(str_promod_content));
				maybeGotoProdModNum = 0;
			}
		}
		else if (checkProdModFlag != 0){
			checkProdModFlag = 0;
		}
	}
	else{
		str_promod_content[maybeGotoProdModNum++] = readbuf;
		if (maybeGotoProdModNum == sizeof(str_promod_content)){
			maybeGotoProdMod = false;
			checkConfirmGotoProdMod();
		}
	}
}

void printCurBuffer(){
	RUISA_LOG"\n*******************\n"LOG_END
	for (int i = 0; i < readStruct.readbufLen; i++){
		RUISA_LOG"%02x ", readStruct.readbuf[i]LOG_END
	}
	RUISA_LOG"\n*******************\n"LOG_END
}
void checkGotoProdMod(){

}
/*
处理resend
*/
void judgeBusFree(int readResult)
{
	if (readResult <= 0){//无数据
		if (readStruct.readNullTimes < WAITDATATIMES){//判断空闲的时间
			readStruct.readNullTimes++;//无数据的次数和时间ms
		}
		else{
			RUISA_LOG"\nreadStruct.readNullTimes = %d\n", readStruct.readNullTimes LOG_END;
			readStruct.readNullTimes = 0;

			if (readStruct.busHaveTask){
				if (dealData.resendtimes == 0){
					RUISA_LOG"\nNO RESPOND 0\n" LOG_END;
					printCurBuffer();
					busComOK = false;
					readStruct.readbufLen = 0;//清空上一次
				}
				if (dealData.resendtimes >= 3){
					RUISA_LOG"\nNO RESPOND %d drop\n", dealData.resendtimes LOG_END;
					readStruct.readBusFree = true;//设置bus空闲状态
					dealData.resendtimes = 0;
					readStruct.busHaveTask = false;
					return;
				}
				//resend
				RUISA_LOG"\nNO RESPOND %d\n", dealData.resendtimes LOG_END;
				dealData.resendtimes++;
			}
		}
	}
	else{//有数据了
		if (readStruct.readNullTimes != 0){
			readStruct.readNullTimes = 0;//重置记录
		}
		if (readStruct.readBusFree){
			readStruct.readBusFree = false;//如果之前标记是空闲，改成繁忙。
		}
	}
}

void cleanReadBuffer(){//清理readStruct.readbuf 2.write前清理一下
	readStruct.readbufLen = 0;
}
void processBusData(){
	//checkGotoProdMod();//还没写
	if (readStruct.busTasking == false){
		if (readStruct.busHaveTask){
			readStruct.busTasking = true;
		}
		else{
			//当有没有预期的数据从底板传来时，打印清空readbuf 但不处理 (后期可以验证在这检测生产模式)
			RUISA_LOG "unexpect Data:" LOG_END;
			checkGotoProdMod();//还没写
			printCurBuffer();
			cleanReadBuffer();
		}
	}
	if (readStruct.busTasking){
		if (readStruct.readbufLen == dealData.totalDataLen){//一帧通讯内容足够完整
			//防止期望的数据之前还有数据，可以在write前cleanReadBuffer();
			RUISA_LOG "wanzheng Data num=%d:" LOG_END;
			memcpy(dealData.resendbuf, readStruct.readbuf, dealData.totalDataLen);//收集一帧数据，可能有bug
			memset(readStruct.readbuf, 0, OVEN_MAX_LEN);
			readStruct.readbufLen = 0;
			processingData();
		}
		else if (readStruct.readbufLen > dealData.totalDataLen){
			RUISA_LOG "overflow Data num=%d:", readStruct.readbufLen - dealData.totalDataLen LOG_END;
			printCurBuffer();//溢出数据打印
		}
		else{
			//数据还不够，再等待一个 READSLEEPTIME 时间
		}
	}
}
void buildDataLen(u8 sendDataLen, u8 receDataLen){
	dealData.sendDataLen = sendDataLen;
	dealData.receDataLen = receDataLen;
	dealData.totalDataLen = sendDataLen + receDataLen;
	cleanReadBuffer();
}
static int periodPerFrame = READSLEEPTIME;

//void *readBusTask(void* arg){//read data from board 再用processingData();将带有数据的内容更新当前的内容。
//	int delay;
//	uint32_t tick;
//
//	int readResult = 0;
//
//	OutExternalEvent ev = { 0 };//传递队列，带有待解码的信息
//
//	while (!exitNormalModeQuit){
//		tick = SDL_GetTicks();
//		//usleep(readStruct.readSleepTime * 1000);
//		readResult = read(ITP_DEVICE_UART1, ((u8*)(&readStruct.readbuf) + readStruct.readbufLen), OVEN_MAX_LEN);
//
//		if ((readResult > 0) && readStruct.readBusFree){
//			readStruct.readBusFree = false;//如果之前标记是空闲，改成繁忙。
//		}
//
//		if (readResult > 0){//有数据，无数据时返回值为-1；
//			readStruct.readbufLen += readResult;
//
//			RUISA_LOG "test see Data:" LOG_END;
//			printCurBuffer();
//
//			if (readStruct.readbufLen != 0){
//				processBusData();
//			}
//		}
//
//		judgeBusFree(readResult);
//
//
//		if (!readStruct.readBusFree || readStruct.busHaveTask){
//			delay = periodPerFrame - (SDL_GetTicks() - tick);
//			printf("bus loop delay=%d\n", delay);
//			if (delay > 0)
//				SDL_Delay(delay);
//			else
//				sched_yield();
//			continue;
//		}
//		//只有 时可以走到这里，相当于busfree = true;只有在串口休息 以上时，才开始下一个任务
//		if (mq_receive(extOutQueue, (char*)&ev, sizeof(OutExternalEvent), 0) > 0)
//		{
//			readStruct.busHaveTask = true;
//			u16 address = 0;
//			u8 readLen = 0;
//
//			switch (ev.type)
//			{
//			case Nc:
//			case Reset:
//			case ReturnIdle:
//				buildDataLen(7, 3);
//				writeSingleCmdToBoard(ev.type);
//				break;
//			case PowerButton:
//			case Cycle:
//			case CycleStart:
//			case StartPause:
//				buildDataLen(8, 3);
//				writeDoubleCmdToBoard(ev.type, ev.arg0);
//				break;
//			case EXTRA_QUERY:
//				address = getU16(&ev.arg0);
//				switch (address)
//				{
//				case addr100:
//					readLen = 0x20;//只需要改此项，查询长度；
//					buildDataLen(5, readLen + 3);//查询5个 回复 82 ...32... crc16 = 35 0x20+3 = 0x23
//					writeQueryCmdToBoard(address, readLen);//82 00 20
//					break;
//				case addr200:
//				case addr200 + 0x12:
//				case addr200 + 0x24:
//				case addr200 + 0x36:
//					readLen = 0x12;
//					buildDataLen(5, readLen + 3);//查询5个 回复 82 ...32... crc16 = 35 0x20+3 = 0x23
//					writeQueryCmdToBoard(address, readLen);//82 00 20
//					break;
//				default:
//					readStruct.busTasking = false;
//					readStruct.busHaveTask = false;
//					break;
//				}
//				break;
//			case EXTRA_SETBUFFER:
//				address = getU16(&ev.arg0);
//				if (address < 0x248 && address >= 0x200){
//					u8 TimePriceNum = 0;
//					u8 offset = 0;
//					u8* point = (u8*)&tAndp;
//					if (address < 0x212){
//						TimePriceNum = 1;
//						offset = address - 0x200;
//						point = (u8*)&tAndp;
//					}
//					else if (address < 0x224){
//						TimePriceNum = 2;
//						offset = address - 0x212;
//						point = (u8*)&tAndp + 0x12;
//					}
//					else if (address < 0x236){
//						TimePriceNum = 3;
//						offset = address - 0x224;
//						point = (u8*)&tAndp + 0x24;
//					}
//					else if (address < 0x248){
//						TimePriceNum = 4;
//						offset = address - 0x236;
//						point = (u8*)&tAndp + 0x36;
//					}
//					g_UIevent.UI_EventCounter = 0;
//					g_UIevent.UI_EventType = SetCycleParamters;
//					g_UIevent.UI_EventValue = TimePriceNum;
//					g_UIevent.UI_datalen = 0x12;
//
//					for (u8 i = 0; i < 0x12; i++)
//					{
//						if (i >= offset && i < (offset + ev.buf1Size)){
//							g_UIevent.UI_EventData[i] = ev.buf1[(i - offset)];
//						}
//						else{
//							g_UIevent.UI_EventData[i] = *(point + i);
//						}
//					}
//					char data[18] = { 0, 0x19, 0, 8, 0, 8, 1, 0, 0x1e, 0, 0x5a, 0, 0x96, 1, 0x5e, 1, 0, 0x78 };
//					memcpy(g_UIevent.UI_EventData, data, 0x12);
//					buildDataLen(26, 3);//1 0 15 0 e 1 0 0 0 0 0 0 1 0 0 0 d2 0 0 e2 ec d5 d2 d5 73 bc 8 + 0x12 +3 //01 00 15 00 0E 00(高温) (19 00)(25美分) +剩下16位数据+2位crc   6+18+2
//					write_buffer_to_board();//将g_UIevent发送给底板
//					outQquery(addr200 + TimePriceNum * 0x12 - 0x12);//紧接着一个查询
//				}
//				break;
//			default:
//				readStruct.busTasking = false;
//				readStruct.busHaveTask = false;
//				break;
//			}
//
//			//delay = SDL_GetTicks() - tick;//3
//			//printf("writeDatatoUart use time : %d\n", delay);//4
//		}
//		else{
//			printf("no queue task qtask\n");
//			outQquery(addr100);
//			if (ituWidgetIsVisible(mainLayer)){
//
//			}
//		}
//
//		delay = periodPerFrame - (SDL_GetTicks() - tick);
//		printf("bus loop delay=%d\n", delay);
//		if (delay > 0)
//			SDL_Delay(delay);
//		else
//			sched_yield();
//
//	}
//	return NULL;
//}

void* TestFuncBT(void* arg);
void* controllerManager(void* arg);


bool CtrlOnEnter(ITUWidget* widget, char* param)
{
	findWidget(mainLayer);
	findWidget(startLayer);
	findWidget(testModeLayer);
	findWidget(prodmodLayer);
	findWidget(specialModeLayer);
	findWidget(warnLayer);
	findWidget(logLayer);
	findWidget(TextLogLine1);
	findWidget(TextLogLine2);
	findWidget(gototestLayer);
	findWidget(settingWiFiSsidLayer);
#ifdef SCREEN_LOG//黑色背景，白色字体才能看见
#if 1//def WIN32
	TextLogLine1->widget.color = makeColor(0xffffffff);
#endif
#endif
	//ExternalInit();
	OutExternalInit();
#if 0//def CFG_NET_WIFI
	CreateWorkerThread(controllerManager, NULL);
#endif
#if 0//def bluetooth
	CreateWorkerThread(TestFuncBT, NULL);
#endif


	if (theConfig.prodmod != 0){
		gotogototestLayerandprodmodLayer();
	}

	return true;
}


static int busOKSum = 0;
#define FCU_ERR 122
bool checkWarn(){
	if (busComOK){
		if (busOKSum != 0){
			busOKSum = 0;
		}
		if (ui.FaultCode == FCU_ERR){
			ui.FaultCode = 0;
#ifdef WIN32
			ui.FaultCode = FCU_ERR;
#endif // WIN32
		}
	}
	else{
		if (busOKSum < 600){
			busOKSum++;
		}
		else{
			busOKSum = 600;
#ifndef CLOSEFCUERR
			ui.FaultCode = FCU_ERR;//
#endif
		}
	}
	switch (ui.FaultCode)
	{
	case 0:
		if (ituWidgetIsVisible(warnLayer))
		{
			ituWidgetSetVisible(warnLayer, false);
			return true;
		}
		break;
	default:
		//if (ui.FaultCode > 99 && ui.FaultCode <= 130){
		if (1){
			processWarn();
			if (!ituWidgetIsVisible(warnLayer)){
				ituWidgetSetVisible(warnLayer, true);
				return true;
			}
		}
		break;
	}
	return false;
}
static int pressTimes = 0;
typedef struct
{
	u8 code;
	u8 offset;//位偏移量
	int timer;//超时清零
	u8 record;//记录录入几个
}adminCtrl;
adminCtrl admin = { 0, 0, 0 };
void initAdmin(){
	admin.code = 0;
	admin.offset = 0;
	admin.timer = 0;
	admin.record = 0;
}
//记录按键，进入工厂测试
void setAdminCode(bool boolcode){
	admin.timer = 0;
	admin.record++;
	if (boolcode){
		setbit(admin.code, admin.offset);
	}
	else{
		clrbit(admin.code, admin.offset);
	}
	if (admin.offset > 6){
		admin.offset = 0;
	}
	else{
		admin.offset++;
	}
}

void processAdmin()
{
	printf("processAdmin\n");
	/*admin.code = 0xAF;
	admin.offset = 4;
	admin.timer = 150;
	admin.record = 8;*/
	if (ituWidgetIsVisible(testModeLayer)){
		if (!isinTestMode){
			isinTestMode = true;
		}
	}

	if (ituWidgetIsVisible(prodmodLayer) || ituWidgetIsVisible(settingWiFiSsidLayer)){//在工厂测试长按退出
		//exitProdLayer();
		return;
	}
	if (isinTestMode){//其他测试长按退出
		outQsetTypeValue(UE_Cycle, HIGH);
		ituLayerGoto(startLayer);
		initAdmin();
		isinTestMode = false;
		return;
	}

	if (admin.record < 8){
		initAdmin();
		return;
	}
	if (admin.timer > 200){
		initAdmin();
		return;
	}
	u8 realcode = (admin.code >> admin.offset) | (admin.code << (8 - admin.offset));
	printf("%s()->realcode=0x%x\n", __FUNCTION__, realcode);
	switch (realcode)
	{
	case 0xFA:
		if (ituWidgetIsVisible(gototestLayer)){
			gotoCustomerLayer();
		}
		else
		{
			gotogototestLayer();
		}
		break;
	case 0xFC:
		//outQsetTypeValue(UE_Cycle, 0xfc);
		break;
	default:
		break;
	}
	admin.record = 0;
}//end processAdmin
void fastGotoTest(){
	setAdminCode(false);
	setAdminCode(true);
	setAdminCode(false);
	setAdminCode(true);
	setAdminCode(true);
	setAdminCode(true);
	setAdminCode(true);
	setAdminCode(true);
	processAdmin();
}

extern StartLayerType curUI;
void printScreenLog(){
	if (!ituWidgetIsVisible(logLayer)){
		ituWidgetSetVisible(logLayer, true);
	}
	u8* point1 = (u8*)&ui;
	u8* point2 = (u8*)&curUI;
	char temp1[256] = { 0 };
	char temp2[256] = { 0 };

	for (int i = 0, j = 0; i < 0x40; i++){//UILENGTH
		sprintf(&temp1[j], "%.2x-", *(point1 + i));
		j += 3;
	}
	for (int i = 0, j = 0; i < sizeof(StartLayerType); i++){//UILENGTH
		sprintf(&temp2[j], "%.2x-", *(point2 + i));
		j += 3;
	}

	sprintf(&temp1[120], "%.2x-", dryerKeyValue1);
	sprintf(&temp1[123], "%.2x-", dryerKeyValue2);

	sprintf(&temp2[120], "%.2x-", dryerKeyValue1);
	sprintf(&temp2[123], "%.2x-", dryerKeyValue2);
	ituTextSetString(TextLogLine1, temp1);
	ituTextSetString(TextLogLine2, temp2);
}

bool checkSpecial132(){
	switch (ui.Choice2)
	{
	case 0x10:
		if (!ituWidgetIsVisible(specialModeLayer)){
			ituLayerGoto(specialModeLayer);
			//ituWidgetSetVisible(specialModeLayer, true);
			return true;
		}
	default:
		break;
	}
	return false;
}

static u16 miaoFlag5 = 250;
static u16 miaoFlag1 = 0;
u8 ctrlKeyBton = 0;//按钮
u8 ctrlKeyKnob = 0;//旋钮
bool CtrlOnTimer(ITUWidget* widget, char* param)
{
	//if (miaoFlag5++ >= 300)
	//{
	//	miaoFlag5 = 0;
	//	outQquery(addr600);
	//}
	//if (miaoFlag1++ >= 60)//5s
	//{
	//	miaoFlag1 = 0;
	//	outQquery(0x160);
	//}
	bool result = false;

#ifdef SCREEN_LOG
#if 1//def WIN32
	UIDataChange = true;
#endif
	if (theConfig.screenLog){
		if (UIDataChange){
			result |= true;
			printScreenLog();
			UIDataChange = false;
		}
	}
	else{
		if (ituWidgetIsVisible(logLayer)){
			ituWidgetSetVisible(logLayer, false);
			result = true;
		}
	}
	
#endif

	ctrl_timer += MS_PER_FRAME;//用来控制闪烁效果


	//printf("%d %d\n", dryerKeyValue1, dryerKeyValue2);
	admin.timer++;//特殊模式进入，用来防止超时。
	if (ctrlKeyBton != dryerKeyValue1){
		UIDataChange |= true;
		if (dryerKeyValue1 == 2){
			if (ProcessButtomDown != NULL){
				ProcessButtomDown();
			}
		}
		//2 -> 0
		pressTimes = 0;
		ctrlKeyBton = dryerKeyValue1;
	}
	else if (dryerKeyValue1 == 2){//长时间为2
		pressTimes++;
		if (pressTimes == 100)
		{
			pressTimes = 0;
			processAdmin();
			/*if (ituWidgetIsVisible(prodmodLayer)){
			
			}
			else{
			ituLayerGoto(prodmodLayer);
			}*/
		}
	}
#ifdef WIN32
	if (keyLongPress == 3){
		pressLeftMidFlag = 1;
	}
	else{
		pressLeftMidFlag = 0;
	}
#endif
	//printf("%d\n", pressLeftMidFlag);
	if (ctrlKeyKnob != dryerKeyValue2){
		pressTimes = 0;
		UIDataChange |= true;

		//printf("last:%x,cur: %x  %x,%x \n", ctrlKeyKnob, ui.KeyValue2, (u8)(ui.KeyValue2 - ctrlKeyKnob), (u8)(ctrlKeyKnob - ui.KeyValue2));
		u8 times = (u8)(dryerKeyValue2 - ctrlKeyKnob);
		if (times < 10){
			if (ButtonDownOnPress != NULL){
				setAdminCode(true);
				ButtonUpOnPress();
			}
			ctrlKeyKnob++;
		}
		else if (times > 246){
			if (ButtonUpOnPress != NULL){
				setAdminCode(false);
				ButtonDownOnPress();
			}
			ctrlKeyKnob--;
		}
		else{
			printf("times is amazing!times = %d now = %x last = %x\n", times, dryerKeyValue2, ctrlKeyKnob);
			ctrlKeyKnob = dryerKeyValue2;
		}
	}
	else if (pressLeftMidFlag == 1){//长时间为1
		pressTimes++;
		printf("keyLongPress = %d\n", pressLeftMidFlag);

		if (pressTimes == 180)
		{
			pressTimes = 0;
			pressLeftMidFlag = 0;
			printf("outQsetTypeValue(WASH_TestOptionsSwitch, 0x03);\n");
#ifdef WIN32
			if (!ui.Choice2){
				ui.Choice2 = 0x10;
			}
			else{
				ui.Choice2 = 0x0;
			}
			
#endif
		outQsetTypeValue(WASH_TestOptionsSwitch, 0x03);
		}
	}
	//printf("keyLongPress = %d\n", keyLongPress);
	//判断一些界面跳转;
	result |= checkSpecial132();//移交流程板管理的一些模式
	if (ituWidgetIsVisible(specialModeLayer)){
		return result;
	}
	printf("ui.CycleName :%d\n",ui.CycleName);
	switch (ui.CycleName){
	case 0xFA:
		if (!ituWidgetIsVisible(mainLayer)){//服务模式且没有到mainLayer
			ituLayerGoto(mainLayer);
			result |= true;
		}
		break;
	case cx_tst_initial:
		if (!ituWidgetIsVisible(testModeLayer)){
			ituLayerGoto(testModeLayer);
		}
		if (ctrlui.CycleName != ui.CycleName){
			printf("ctrlui.CycleName change to %x \n", ui.CycleName);
			ctrlui.CycleName = ui.CycleName;
		}
		break;
	case 0xF0:
	case 0xFB:
	case 0xFC:
	case 0xFD:
	case 0xFF:
		if (!ituWidgetIsVisible(testModeLayer)){
			ituLayerGoto(testModeLayer);
		}
		if (ctrlui.CycleName != ui.CycleName){
			printf("ctrlui.CycleName change to %x \n", ui.CycleName);
			ctrlui.CycleName = ui.CycleName;
		}
		break;
	default:

		break;
	}
	if (ituWidgetIsVisible(prodmodLayer) || ituWidgetIsVisible(settingWiFiSsidLayer) || ituWidgetIsVisible(gototestLayer)){
		printf("return here ===================\n");
		return result;
	}

#ifndef CLOSEALLERR
	if (!isinTestMode){
		result |= checkWarn();
	}
#endif // CLOSEALLERR

	if (ui.CycleName < 0xf0){
		if (!ituWidgetIsVisible(startLayer)){//且没有在startLayer
			ituLayerGoto(startLayer);
			if (isinTestMode){
				isinTestMode = false;
			}
			result |= true;
		}
	}
	//switch (ui.CycleName)
	//{
	//case 0x0:
	//case 0x1:
	//case 0x2:
	//case 0x3:
	//case 0x4:
	//	if (!ituWidgetIsVisible(startLayer)){//且没有在startLayer
	//		ituLayerGoto(startLayer);
	//		if (isinTestMode){
	//			isinTestMode = false;
	//		}
	//		result |= true;
	//	}
	//	break;
	//default:
	//	break;
	//}

	return result;
}
bool CtrlOnLeave(ITUWidget* widget, char* param)
{
	
	return true;
}
