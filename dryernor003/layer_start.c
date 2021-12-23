#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "projutils.h"
#include "layer_ctrl.h"
#include "boardstate.h"



#define RED_FOR_START		0xffe54d42	//HIGH
#define ORANGE_FOR_START	0xfff37b1d	//MIDIUM
#define YELLOW_FOR_START	0xfffbbd08	//LOW
#define BLUE_FOR_START		0xff0081ff	//NO HEAT

static ITUBackground* BackgroundStartWelcome;
static ITUBackground* BackgroundSSF;//选择程序
static ITUBackground* BackgroundSSS;//运行期间

static ITUText* ntcText;
static ITUText* TextTempENG;
static ITUText* TextTempESP;
static ITUText* TextPayShow;
static ITUText* TextPrice;

static ITUText* TextTimeShow, *TextTempShow;
static ITUText* TextSSTag;

static ITUIcon* IconSpark;

int showFlag;//Only WIN32
int moneyCost = 0;
int rmainTime = 0;
char NTCVlue [64] = {0};
int beforeRmainTime = 0;
static bool refreshLayerFlag = false;
static bool waitForReturnIdle = false;//开始运行后标记，流程板信号从end到Idle说明回到首页。
static bool waitForReturnTemp = false;//选择程序后标记，再次左右选择可以回到选择页。
bool isPaying = false;
bool isRunning = false;
bool isAfterPaying = false;
char payStr[32] = { 0 };

WP_EVENT_UI_LOGIC localUI = { 0 };
int welcomeTimer = 0;
#define SLEEPTIME 3600 //3600 = 1min
bool isFirstInStartLayer = true;

	static int outoldvalue =-1;
	static int inoldvalue =-1;
StartLayerType curUI = { -1, 0, 0, 0, 0, true };
typedef enum{
	SELECT = 0,
	PAYMENT,
	RUNNING,
}startStage;
typedef struct{
	bool screenon;
	bool promptPage;//提示界面
	CycleType temperature;
	startStage stage;
}startShowType;
startShowType startShow = { 0 };


void keyAction(){
	if (isFirstInStartLayer){
		isFirstInStartLayer = false;
	}
	if (welcomeTimer){
		welcomeTimer = 0;
	}
}
void setRefreshF(long ARGB, char* ENG, char* ESP){
	ituWidgetSetColor2((ITUWidget*)BackgroundSSF, ARGB);//红色ff0000
	ituTextSetString(TextTempENG, ENG);
	ituTextSetString(TextTempESP, ESP);
}
void refreshF(u8 i){
	printf("refreshF cycle Name :%d\n",i);
	switch (i){
	case HIGH:
		setRefreshF(RED_FOR_START, "HIGH TEMP", "ALTA TEMP");//红色ff0000
		break;
	case MIDIUM:
		setRefreshF(ORANGE_FOR_START, "MEDIUM TEMP", "MENZA MENZATA");//橙色ff007f
		break;
	case LOW:
		setRefreshF(YELLOW_FOR_START, "LOW TEMP", "UN POQITO HEAT");//黄色ff00ff
		break;
	case NOHEAT:
		setRefreshF(BLUE_FOR_START, "NO HEAT", "MUCHO FRIO");//蓝色00ff00
		break;
	default:
		setRefreshF(0xFFAAAAAA, "NO SUCH CYCLE", "-- ---- -----");//红色ff0000
		break;
	}
	refreshLayerFlag = true;
}
void setRefreshS(long ARGB, char* string){
	ituWidgetSetColor2((ITUWidget*)BackgroundSSS, ARGB);//红色ff0000
	ituTextSetString(TextTempShow, string);
}
void refreshS(u8 i){
	switch (i){
	case HIGH:
		setRefreshS(RED_FOR_START, "HIGH / ALTA");//红色ff0000
		break;
	case MIDIUM:
		setRefreshS(ORANGE_FOR_START, "MIDIUM / MEDIA");//橙色ff007f
		break;
	case LOW:
		setRefreshS(YELLOW_FOR_START, "LOW / BAJA");//黄色ff00ff
		break;
	case NOHEAT:
		setRefreshS(BLUE_FOR_START, "NO HEAT / SIN CALOR");//蓝色00ff00
		break;
	default:
		break;
	}
	refreshLayerFlag = true;
}

void clearStartShow(){
	startShow.promptPage = false;
	startShow.stage = SELECT;
}

void cancelPaying(){
	isPaying = false;
	TextPayShow->widget.visible = false;
}
static void showIcons(u8 bits){
	if (isFirstInStartLayer){
		ituWidgetSetVisible(BackgroundStartWelcome, true);
		ituWidgetSetVisible(BackgroundSSF, false);
		ituWidgetSetVisible(BackgroundSSS, false);
		return;
	}
	ituWidgetSetVisible(BackgroundStartWelcome, bits & 1);
	ituWidgetSetVisible(BackgroundSSF, bits & 2);
	ituWidgetSetVisible(BackgroundSSS, bits & 4);
	//TextPayShow->widget.visible = bits & 8;
}
static void makePayStr(u16 moneyCost){
	memset(payStr, 0, 32);
	if (cp.SHOW_DECIMAL == 0){
		sprintf(payStr, "$%d", moneyCost);
		return;
	}
	else if (cp.SHOW_DECIMAL == 1){
		int mcL = moneyCost % 100;
		int mcH = moneyCost / 100;
		sprintf(payStr, "$%d.%02d", mcH, mcL);
	}
	else{
		strcpy(payStr, "error");
	}
}

void updateData(){
	moneyCost = (int)ui.MoneyCostL + (int)ui.MoneyCostH*(int)0x100;
	rmainTime = (int)ui.RmainTimeL + (int)ui.RmainTimeH*(int)0x100;
	//请求获得
}

u8 isShowNtc()
{
	printf( "out:%d , in %d \n",cp.isShowTemp_OUT,cp.isShowTemp_IN);

	if(cp.isShowTemp_OUT||cp.isShowTemp_IN)
		{
			//printf("isShowNtc true\n");
			return 1;
		}
	return 0;
}

bool isNtcValuechange()
{

	int newvalue = 0;
	newvalue = mrs.OUT_NTC1_L+ (int)mrs.OUT_NTC1_H*0x100;
	if(outoldvalue!= newvalue)
		{
			outoldvalue = newvalue;
			printf("isNtcValuechange true\n");

			return true;
		}
	newvalue = mrs.IN_NTC1_L+ (int)mrs.IN_NTC1_H*0x100;
	if(inoldvalue!= newvalue)
		{
			inoldvalue = newvalue;
						printf("isNtcValuechange true\n");

			return true;
		}
	return false;
}
char * getNTCValue(){
	u8 len =0;
	memset(NTCVlue,0,64);

	printf( "out:%d , in %d \n",cp.isShowTemp_OUT,cp.isShowTemp_IN);
	if(cp.isShowTemp_OUT)
		{
			sprintf(NTCVlue,"NTC1: %d   ",mrs.OUT_NTC1_L+ (int)mrs.OUT_NTC1_H*0x100);
		}
		printf( "strlen(NTCVlue):%d  \n",strlen(NTCVlue));

	
	if(cp.isShowTemp_IN)
		{
			sprintf(NTCVlue+strlen(NTCVlue),"NTC2: %d ",mrs.IN_NTC1_L+ (int)mrs.IN_NTC1_H*0x100);
		}
	//moneyCost = ms.OUT_NTC1_L+ (int)ms.OUT_NTC1_H*0x100;
//	rmainTime = ms.IN_NTC1_L+ (int)ms.IN_NTC1_H*(int)0x100;
	//请求获得
	return NTCVlue;
}
void updateTextPrice(){
	makePayStr(curUI.MoneyCost);
	ituTextSetString(TextPrice, payStr);
}
void updateTextTime(){
	char str10[10];
	if (curUI.RmainTime != 0){
		sprintf(str10, "%d MIN", curUI.RmainTime);
		ituTextSetString(TextTimeShow, str10);
	}
	else{
		ituTextSetString(TextTimeShow, "End");
	}
}

u16 getRmainTime(){
	return (int)ui.RmainTimeL + (int)ui.RmainTimeH*(int)0x100;
}
u16 getMoneyCost(){
	return (int)ui.MoneyCostL + (int)ui.MoneyCostH*(int)0x100;
}
bool isWelcomeShow(){
	return ituWidgetIsVisible(BackgroundStartWelcome);
}
bool isSelectMode(){
	return ituWidgetIsVisible(BackgroundSSF);
}
bool isRunningMode(){
	return ituWidgetIsVisible(BackgroundSSS);
}
bool isPayStage(){
	return ituWidgetIsVisible(TextPayShow);
}


bool curUIisNULL(){
	if (curUI.RmainTime != 0)
		return false;
	if (curUI.CycleName != 0)
		return false;
	if (curUI.DryingStage != 0)
		return false;
	if (curUI.MoneyCost != 0)
		return false;
	if (curUI.RunMode != 0)
		return false;
	return true;
}


static void init(){//只显示 提示 界面
	ituWidgetSetColor2((ITUWidget*)BackgroundStartWelcome, BLUE_FOR_START);
	showIcons(1);//按位展示background
	if ((ui.RunMode == RM_RUNNING || ui.RunMode == RM_PAUSE) && isWelcomeShow()){//运行阶段却显示欢迎界面
		showIcons(4);//显示第二界面；隐藏其他界面；
	}
	//ituWidgetSetVisible(TextPayShow, false);
	ituWidgetSetVisible(IconSpark, false);
	if (curUIisNULL()){
		isFirstInStartLayer = true;
	}
#ifdef O_WIN32
	showFlag = 0;//展示标志置位
	clearStartShow();
#endif
	updateData();
}
//如果底板要求重选程序
//init()
//startShow.promptPage = true
//refreshStartLayer();

static bool refreshLayerNum = false;
void refreshStartLayer()
{
	if (!refreshLayerNum){
		refreshLayerNum = true;
	}
}

//void refreshStartLayerUsedByTimer(){
//
//	if (isFirstInStartLayer){
//		printf("first refresh DATA\n");
//		curUI.CycleName = ui.CycleName;//0 high  1 2 3 FA
//		curUI.DryingStage = ui.DryingStage;
//		curUI.RunMode = ui.RunMode;//02 running 03 pause
//		curUI.RmainTime = getRmainTime();
//		curUI.MoneyCost = getMoneyCost();
//	}
//	if ((ui.RunMode == RM_RUNNING || ui.RunMode == RM_PAUSE) && isWelcomeShow()){//运行阶段却显示欢迎界面
//
//		showIcons(4);//显示第二界面；隐藏其他界面；
//		printf("CycleName %d change to %d\n", curUI.CycleName, ui.CycleName);
//		curUI.CycleName = ui.CycleName;
//		refreshS(ui.CycleName);
//
//	}
//
//	switch (ui.RunMode)
//	{
//	case RM_OFF:
//		if (curUI.RunMode == ui.RunMode){
//			return;
//		}
//		break;
//	case RM_STANDBY:
//		if (isWelcomeShow() && !isFirstInStartLayer){ return; }
//		if (isSelectMode()){
//			updateData();
//			if (isPayStage()){//2
//				if (curUI.CycleName == ui.CycleName){
//					if (moneyCost == curUI.MoneyCost && curUI.MoneyCost > 0){
//						return;
//					}
//					else{
//						if (moneyCost != curUI.MoneyCost){
//							printf("MoneyCost %d change to %d\n", curUI.MoneyCost, moneyCost);
//							updateTextPrice();
//						}
//
//						if (!(curUI.MoneyCost > 0)){
//							//可以下一步
//							//BackgroundSSS->icon.widget.color = BackgroundSSF->icon.widget.color;//第二界面和第一界面颜色一致；
//
//							showIcons(4);//显示第二界面；隐藏其他界面；
//							printf("CycleName %d change to %d\n", curUI.CycleName, ui.CycleName);
//							curUI.CycleName = ui.CycleName;
//							refreshS(ui.CycleName);
//
//							beforeRmainTime = rmainTime;
//							char str10[10];
//							sprintf(str10, "%d MIN", rmainTime);
//							ituTextSetString(TextTimeShow, str10);
//							ituTextSetString(TextSSTag, ">|| TO START");
//							ituWidgetSetVisible(IconSpark, false);
//
//							//memcpy(TextTimeShow->stringSet->strings[0], str10, sizeof(str10));
//							//TextSSTag->stringSet->strings[0] = ">|| TO START";
//						}
//						else{
//							updateTextPrice();
//						}
//						refreshLayerFlag = true;
//						return;
//					}
//				}
//			}
//			if (moneyCost != curUI.MoneyCost){
//				printf("MoneyCost %d change to %d\n", curUI.MoneyCost, moneyCost);
//				updateTextPrice();
//			}
//			if (!isFirstInStartLayer && curUI.CycleName == ui.CycleName){//1
//				return;
//			}
//
//			if (curUI.CycleName != ui.CycleName || isFirstInStartLayer){
//				showIcons(2);//显示第一界面；隐藏其他界面；
//				printf("CycleName %d change to %d\n", curUI.CycleName, ui.CycleName);
//				curUI.CycleName = ui.CycleName;
//				refreshF(ui.CycleName);
//			}
//			if (isFirstInStartLayer){
//				printf("first refresh UI\n");
//				isFirstInStartLayer = false;
//			}
//			refreshLayerFlag = true;
//			return;
//		}
//		break;
//	case RM_RUNNING:
//		if (curUI.RunMode != ui.RunMode || isFirstInStartLayer){
//			curUI.RunMode = ui.RunMode;
//			ituTextSetString(TextSSTag, ">|| TO STOP");
//			//TextSSTag->stringSet->strings[0] = ">|| TO STOP";//
//			refreshLayerFlag = true;
//		}
//		if (curUI.CycleName != ui.CycleName || isFirstInStartLayer){
//			showIcons(4);
//			printf("CycleName %d change to %d\n", curUI.CycleName, ui.CycleName);
//			curUI.CycleName = ui.CycleName;
//			refreshS(ui.CycleName);
//		}
//		updateData();
//		if (!ituWidgetIsVisible(IconSpark)){
//			ituWidgetSetVisible(IconSpark, true);
//		}
//		refreshLayerFlag = true;
//
//		if (rmainTime == 0 && ui.DryingStage == DS_ending){//结束
//			outQsetType(ReturnIdle);
//			showIcons(2);//显示第一界面；隐藏其他界面；
//			//sendMsgtoOUTQ(EXTERNAL_DRY_RESET, 0, 0, 0);
//			ituTextSetString(TextSSTag, ">|| TO START");
//			//TextSSTag->stringSet->strings[0] = ">|| TO START";
//			ituWidgetSetVisible(IconSpark, false);
//			printf("CycleName %d change to %d\n", curUI.CycleName, ui.CycleName);
//			curUI.CycleName = ui.CycleName;
//			refreshF(ui.CycleName);
//
//		}
//		if (beforeRmainTime == rmainTime && !(isFirstInStartLayer)){
//			return;
//		}
//		beforeRmainTime = rmainTime;
//		char str10[10];
//		sprintf(str10, "%d MIN", rmainTime);
//		ituTextSetString(TextTimeShow, str10);
//		//memcpy(TextTimeShow->stringSet->strings[0], str10, sizeof(str10));
//		if (isFirstInStartLayer){
//			printf("first refresh UI\n");
//			isFirstInStartLayer = false;
//		}
//		refreshLayerFlag = true;
//		break;
//	case RM_PAUSE:
//		//printf("RM_PAUSE\n");
//		if (localUI.RmainTimeL != ui.RmainTimeL || localUI.RmainTimeH != ui.RmainTimeH){
//			//printf("RM_PAUSE1111\n");
//			localUI.RmainTimeL = ui.RmainTimeL;
//			localUI.RmainTimeH = ui.RmainTimeH;
//			updateData();
//			char str10[10];
//			sprintf(str10, "%d MIN", rmainTime);
//			ituTextSetString(TextTimeShow, str10);
//		}
//		ituWidgetSetVisible(IconSpark, false);
//		TextSSTag->stringSet->strings[0] = ">|| TO START";//
//		if (curUI.RunMode == ui.RunMode){
//			return;
//		}
//		curUI.RunMode = ui.RunMode;
//		break;
//	case RM_ADD:
//		if (curUI.RunMode == ui.RunMode){
//			return;
//		}
//		break;
//	default:
//		printf("it looks like an ERROR\n");
//		break;
//	}
//	refreshLayerFlag = true;
//	return;
//}

//static void processShowFlag(){
//	//win模拟时使用
//#ifndef O_WIN32
//	return;
//#endif
//	switch (showFlag)
//	{
//	case 0://ti shi
//		init();
//		break;
//	case 1://HIGH TEMP
//		outQsetTypeValue(Cycle, HIGH);
//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_Cycle, (u16)HIGH);
//		showIcons(2);
//		setRefreshF(RED_FOR_START, "HIGH TEMP", "ALTA TEMP");//红色ff0000
//
//		//BackgroundSSF->icon.widget.color = makeColor(RED_FOR_START);//红色ff0000
//		//TextTempENG->stringSet->strings[0] = "HIGH TEMP";
//		//TextTempESP->stringSet->strings[0] = "ALTA TEMP";
//		makePayStr(curUI.MoneyCost);
//		TextPrice->stringSet->strings[0] = payStr;
//		break;
//	case 2://MEDIUM TEMP
//		outQsetTypeValue(Cycle, MIDIUM);
//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_Cycle, (u16)MIDIUM);
//		showIcons(2);
//		setRefreshF(ORANGE_FOR_START, "MEDIUM TEMP", "MENZA MENZATA");//橙色ff007f
//
//		//BackgroundSSF->icon.widget.color = makeColor(ORANGE_FOR_START);//橙色ff007f
//		//TextTempENG->stringSet->strings[0] = "MEDIUM TEMP";
//		//TextTempESP->stringSet->strings[0] = "MENZA MENZATA";
//		makePayStr(curUI.MoneyCost);
//		TextPrice->stringSet->strings[0] = payStr;
//		break;
//	case 3://LOW TEMP
//		outQsetTypeValue(Cycle, LOW);
//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_Cycle, (u16)LOW);
//		showIcons(2);
//		setRefreshF(YELLOW_FOR_START, "LOW TEMP", "UN POQITO HEAT");//黄色ff00ff
//
//		//BackgroundSSF->icon.widget.color = makeColor(YELLOW_FOR_START);//黄色ff00ff
//		//TextTempENG->stringSet->strings[0] = "LOW TEMP";
//		//TextTempESP->stringSet->strings[0] = "UN POQITO HEAT";
//		makePayStr(curUI.MoneyCost);
//		TextPrice->stringSet->strings[0] = payStr;
//		break;
//	case 4://NO HEAT
//		outQsetTypeValue(Cycle, NOHEAT);
//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_Cycle, (u16)NOHEAT);
//		showIcons(2);
//		setRefreshF(BLUE_FOR_START, "NO HEAT", "MUCHO FRIO");//蓝色00ff00
//
//		//BackgroundSSF->icon.widget.color = makeColor(BLUE_FOR_START);//蓝色00ff00
//		//TextTempENG->stringSet->strings[0] = "NO HEAT";
//		//TextTempESP->stringSet->strings[0] = "MUCHO FRIO";
//		makePayStr(curUI.MoneyCost);
//		TextPrice->stringSet->strings[0] = payStr;
//		break;
//	case 11://HIGH TEMP
//		setRefreshS(RED_FOR_START, "HIGH / ALTA");//红色ff0000
//
//		//BackgroundSSS->icon.widget.color = BackgroundSSF->icon.widget.color;//第二界面和第一界面颜色一致；
//		//TextTempShow->stringSet->strings[0] = "HIGH / ALTA";
//		showIcons(4);//显示第二界面；隐藏其他界面；
//		TextTimeShow->stringSet->strings[0] = "8 MIN";
//		TextSSTag->stringSet->strings[0] = ">|| TO START";
//		break;
//	case 12://MEDIUM TEMP
//		setRefreshS(ORANGE_FOR_START, "MIDIUM / MEDIA");//橙色ff007f
//
//		//BackgroundSSS->icon.widget.color = BackgroundSSF->icon.widget.color;//第二界面和第一界面颜色一致；
//		//TextTempShow->stringSet->strings[0] = "MEDIUM / MEDIUM";
//		showIcons(4);//显示第二界面；隐藏其他界面；
//		TextTimeShow->stringSet->strings[0] = "8 MIN";
//		TextSSTag->stringSet->strings[0] = ">|| TO START";
//		break;
//	case 13://LOW TEMP
//		setRefreshF(YELLOW_FOR_START, "LOW TEMP", "UN POQITO HEAT");//黄色ff00ff
//
//		//BackgroundSSS->icon.widget.color = BackgroundSSF->icon.widget.color;//第二界面和第一界面颜色一致；
//		//TextTempShow->stringSet->strings[0] = "LOW / LOW";
//		showIcons(4);//显示第二界面；隐藏其他界面；
//		TextTimeShow->stringSet->strings[0] = "8 MIN";
//		TextSSTag->stringSet->strings[0] = ">|| TO START";
//		break;
//	case 14://NO HEAT
//		setRefreshS(BLUE_FOR_START, "NO HEAT / SIN CALOR");//蓝色00ff00
//
//		//BackgroundSSS->icon.widget.color = BackgroundSSF->icon.widget.color;//第二界面和第一界面颜色一致；
//		//TextTempShow->stringSet->strings[0] = "NO HEAT / NO HEAT";
//		showIcons(4);//显示第二界面；隐藏其他界面；
//		TextTimeShow->stringSet->strings[0] = "8 MIN";
//		TextSSTag->stringSet->strings[0] = ">|| TO START";
//		break;
//	default:
//		break;
//	}
//}

bool StartButtonUpOnPress(void){
	BUTTON_LOG"|A|"LOG_END;//up
	keyAction();
#if 1
	if (isWelcomeShow()){
		showIcons(2);//主要目的取消1；
		return true;
	}
	if (isSelectMode()){
		if (curUI.CycleName > 1) {
			outQsetTypeValue(UE_Cycle, ((curUI.CycleName) - 1));
			//ituWidgetSetVisible(TextPayShow, false);
			return true;
		}
		else{
			return false;
		}
	}
	if (isRunningMode() && curUI.RunMode != RM_RUNNING){
		showIcons(2);
		//ituWidgetSetVisible(TextPayShow, false);
		return true;
	}
	return false;
#endif
	if (isPaying == true || isAfterPaying == true){ return false; }
	showFlag %= 10;
	if (showFlag == 0){
		showFlag++;
	}
	else if (showFlag > 1){
		showFlag--;
	}
	else{
		return false;
	}
	//processShowFlag();
	//ituWidgetSetVisible(TextPayShow, false);
	//TextPayShow->widget.visible = false;
	return true;
}
bool StartButtonDownOnPress(void){
	BUTTON_LOG"|V|"LOG_END;//down
	keyAction();
#if 1
	if (isWelcomeShow()){
		showIcons(2);//主要目的取消1；
		return true;
	}
	if (isSelectMode()){
		if (curUI.CycleName < 4) {
			outQsetTypeValue(UE_Cycle, ((curUI.CycleName) + 1));
			//ituWidgetSetVisible(TextPayShow, false);
			return true;
		}
		else{
			return false;
		}
	}
	if (isRunningMode() && curUI.RunMode != RM_RUNNING){
		showIcons(2);
		//ituWidgetSetVisible(TextPayShow, false);
		return true;
	}
	return false;
#endif
	if (isPaying == true || isAfterPaying == true){ return false; }
	showFlag %= 10;
	if (showFlag < 4){
		showFlag++;
	}
	else{
		return false;
	}
	//processShowFlag();
	//TextPayShow->widget.visible = false;
	return true;
}
bool StartProcessButtomDown(void){
	updateData();
	BUTTON_LOG"|>|"LOG_END;//select
	keyAction();
#if 1
	if (isWelcomeShow()){
		showIcons(2);//主要目的取消1；
		return true;
	}
	if (isSelectMode() && curUI.RunMode == RM_STANDBY){
		outQsetTypeValue(UE_Cycle, (curUI.CycleName));
		printf("select---%d---\n", isPayStage());
		if (!isPayStage()){
			showIcons(4);
		}
		//TextPayShow->widget.visible = true;
		//ituWidgetSetVisible(TextPayShow,true);
		//if (!(moneyCost > 0)){
		//	refreshStartLayer();//为了 价格为0时 快速取消TextPayShow
		//}
		return true;
	}
	if (isRunningMode()){
		switch (curUI.RunMode)
		{
		case RM_STANDBY:
			outQsetTypeValue(UE_StartPause, START);
			break;
		case RM_RUNNING:
			outQsetTypeValue(UE_StartPause, PAUSE);
			break;
		case RM_PAUSE:
			outQsetTypeValue(UE_StartPause, START);
			break;
		default:
			break;
		}
		return true;
	}
	return true;
#endif
	//if (isPaying == true){
	//	//cancelPaying(); 
	//	//return false; 
	//}
	//if (showFlag == 0){//提示界面转选择页面首页
	//	showFlag++;
	//	//processShowFlag();
	//}
	//else if ((!(moneyCost > 0)) && isAfterPaying == false){//剩余价格为0时，选择后直接到启停界面
	//	showFlag %= 10;
	//	showFlag += 10;
	//	isPaying = false;
	//	isAfterPaying = true;
	//	//processShowFlag();

	//}
	//else if (showFlag > 0 && showFlag < 5){//&& moneyCost!=0    //按一次走if，按第二次走else(case11-14)
	//	if (TextPayShow->widget.visible == false){
	//		//TextPayShow->widget.visible = true;
	//		isPaying = true;
	//		//发送支付请求
	//	}
	//	else{//
	//		showFlag %= 10;
	//		showFlag += 10;
	//		if (true){//没支付
	//			//重新发送支付请求
	//			//return;
	//		}
	//		else{
	//			isPaying = false;
	//			isAfterPaying = true;
	//		}
	//	}
	//	//processShowFlag();
	//}
	//else if (showFlag > 10 && showFlag < 15){
	//	if (isRunning == false){
	//		outQsetTypeValue(UE_StartPause, START);
	//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_StartPause, (u16)START);
	//		isRunning = true;
	//		//TextSSTag->stringSet->strings[0] = ">|| TO STOP";//发送 启动
	//		printf("Dryer Start------\n");
	//		ituWidgetSetVisible(IconSpark, true);
	//	}
	//	else{
	//		outQsetTypeValue(UE_StartPause, PAUSE);
	//		//sendMsgtoOUTQ(EXTERNAL_DRY_NORMAL, 0, UIEVENT_StartPause, (u16)PAUSE);
	//		isRunning = false;
	//		//TextSSTag->stringSet->strings[0] = ">|| TO START";//发送 暂停
	//		printf("Dryer Stop-------\n");
	//		ituWidgetSetVisible(IconSpark, false);
	//	}
	//	//processShowFlag();
	//}
	//return true;
}

void initCurUI(){
	//if (ui.CycleName == 0){//开机时流程板程序为0，设置默认 为高温。不为0，给流程板设置一下原程序
	//	outQsetTypeValue(Cycle, HIGH);
	//}
	//else{
	//	outQsetTypeValue(Cycle, ui.CycleName);
	//}
	isFirstInStartLayer = true;
	curUI.CycleName = -1;

	curUI.MoneyCost = -1;
	updateTextPrice();

	curUI.RmainTime = -1;
	updateTextTime();

	curUI.PayShow = false;
	ituWidgetSetVisible(TextPayShow, curUI.PayShow);

	curUI.RunMode = RM_INIT;
	ituTextSetString(TextSSTag, ">|| TO START");
	ituWidgetSetVisible(IconSpark, false);

}
void returnIdleInitUI(){
	curUI.CycleName = ui.CycleName;
	curUI.MoneyCost = -1;
	updateTextPrice();
	curUI.RmainTime = -1;
	updateTextTime();
	curUI.PayShow = false;
	ituWidgetSetVisible(TextPayShow, curUI.PayShow);

	curUI.RunMode = ui.RunMode;
	ituTextSetString(TextSSTag, ">|| TO START");
	ituWidgetSetVisible(IconSpark, false);

	ituWidgetSetVisible(BackgroundSSF, true);
	ituWidgetSetVisible(BackgroundSSS, true);

}

bool StartOnEnter(ITUWidget* widget, char* param)
{

#ifdef O_WIN32
	printf("\n\n\n\n\n\n");
#endif
	ButtonUpOnPress = &StartButtonUpOnPress;
	ButtonDownOnPress = &StartButtonDownOnPress;
	ProcessButtomDown = &StartProcessButtomDown;
	findWidget(BackgroundStartWelcome);
	findWidget(BackgroundSSF);
	findWidget(BackgroundSSS);
	findWidget(TextTempENG);
	findWidget(TextTempESP);
	findWidget(TextPayShow);
	findWidget(TextPrice);
	findWidget(TextTimeShow);
	findWidget(TextTempShow);
	findWidget(TextSSTag);
	findWidget(IconSpark);
	findWidget(ntcText);
	ituTextSetString(ntcText, " ");
	outoldvalue =-1;
	inoldvalue =-1;
	//processShowFlag();//只在WIN32有效
	ituTextSetFontSize(TextTempENG,50);
	ituTextSetFontSize(TextTempESP,50);
	ituTextSetFontSize(TextPayShow,50);
	ituTextSetFontSize(TextPrice, 50);
	ituTextSetFontSize(TextTempShow,50);
	ituTextSetFontSize(TextTimeShow,80);
	ituTextSetFontSize(TextSSTag,50);
#ifndef O_WIN32
	//outQsetType(Nc);//第一次send
	init();
#endif
	initCurUI();
	return true;
}
bool checkTime(){
	if (curUI.RmainTime != getRmainTime()){
		curUI.RmainTime = getRmainTime();
		updateTextTime();
		return true;
	}
	return false;
}
bool checkCost(){
	if (curUI.MoneyCost != getMoneyCost()){
		curUI.MoneyCost = getMoneyCost();
		makePayStr(curUI.MoneyCost);
		ituTextSetString(TextPrice, payStr);
		return true;
	}
	return false;
}
bool checkPayShow(){
	if (curUI.PayShow){
		if (curUI.MoneyCost == 0){
			curUI.PayShow = false;
			ituWidgetSetVisible(TextPayShow, curUI.PayShow);
			return true;
		}
	}
	else{
		if (curUI.MoneyCost > 0){
			curUI.PayShow = true;
			ituWidgetSetVisible(TextPayShow, curUI.PayShow);
			return true;
		}
	}
	return false;
}
bool checkStartandPause(){
	bool ret = false;
	if (curUI.RunMode != ui.RunMode){
		curUI.RunMode = ui.RunMode;
		switch (curUI.RunMode)
		{
		case RM_OFF:

			break;
		case RM_STANDBY:

			break;
		case RM_RUNNING:
			ituTextSetString(TextSSTag, ">|| TO STOP");
			ituWidgetSetVisible(IconSpark, true);
			ret |= true;
			break;
		case RM_PAUSE:
			ituTextSetString(TextSSTag, ">|| TO START");
			ituWidgetSetVisible(IconSpark, false);
			ret |= true;
			break;
		case RM_ADD:

			break;
		default:
			break;
		}
	}
	return ret;
}

bool checkWelcome(){
	if (ituWidgetIsVisible(BackgroundStartWelcome)){

	}
	else{
		if (ui.RunMode == RM_STANDBY){
			welcomeTimer++;
			if (welcomeTimer >= SLEEPTIME){
				init();
				return true;
			}
		}
		else{
			if (welcomeTimer){
				welcomeTimer = 0;
			}
		}
	}

	return false;
}
bool checkCycleName(){
	if (curUI.CycleName != ui.CycleName){
		printf("CycleName %d change to %d", curUI.CycleName, ui.CycleName);
		curUI.CycleName = ui.CycleName;
		switch (curUI.CycleName)
		{
		case CN_NONE:
			printf("no deal maybe an error\n");
			showIcons(2);
			refreshF(curUI.CycleName);
			refreshS(curUI.CycleName);
			break;
		case CN_HIGH:
		case CN_MIDIUM:
		case CN_LOW:
		case CN_NOHEAT:
			showIcons(2);
			refreshF(curUI.CycleName);
			refreshS(curUI.CycleName);
			break;
		default:
			printf("no deal maybe an error\n");
			break;
		}
		printf("\n");
		return true;
	}
	return false;
}

bool checkDryingStage(){
	if (curUI.DryingStage != ui.DryingStage){
		printf("DryingStage %d change to %d\n", curUI.DryingStage, ui.DryingStage);
		curUI.DryingStage = ui.DryingStage;
		if (!ituWidgetIsVisible(TextSSTag)){
			ituWidgetSetVisible(TextSSTag, true);
		}
		switch (curUI.DryingStage)
		{
		case DS_idle:
			printf("DS_idle0\n");
			break;
		case DS_delay:
			printf("DS_delay1\n");
			break;
		case DS_sensing:
			printf("DS_sensing2\n");
			break;
		case DS_dry:
			printf("DS_dry3\n");
			break;
		case DS_cooling:
			printf("DS_cooling4\n");
			break;
		case DS_wrinklefree:
			printf("DS_wrinklefree5\n");
			break;
		case DS_unlock:
			printf("DS_unlock6\n");
			break;
		case DS_ending:
			printf("DS_ending7\n");
			{
				ituTextSetString(TextTimeShow, "End");
				ituWidgetSetVisible(TextSSTag,false);
				ituWidgetSetVisible(IconSpark, false);
			}
			//outQsetType(ReturnIdle);
			//curUI.CycleName = -1;
#if 0//def WIN32
			ui.CycleName = CN_HIGH;
			ui.RunMode = RM_STANDBY;
			ui.DryingStage = DS_idle;
#endif
			break;
		default:
			printf("no deal maybe an error\n");
			break;
		}
		return true;
	}
	return false;

}
bool checkRunMode(){
	//if (curUI.RunMode == RM_RUNNING){//补丁
	//	if (isFirstInStartLayer){
	//		isFirstInStartLayer = false;
	//	}
	//	if (!isRunningMode()){
	//		showIcons(4);
	//	}
	//	if (isSelectMode()){
	//		showIcons(4);
	//	}
	//}
	//printf("RunMode %d change to %d\n", curUI.RunMode, ui.RunMode);
	bool ret = false;
	switch (curUI.RunMode)
	{
	case RM_RUNNING:
		if (!ituWidgetIsVisible(IconSpark)){
			if (curUI.DryingStage != DS_ending){
				ituWidgetSetVisible(IconSpark, true);
				ret = true;
			}	
		}
		break;
	default:
		if (ituWidgetIsVisible(IconSpark)){
			ituWidgetSetVisible(IconSpark, false);
			ret = true;
		}
		break;
	}
	if (curUI.RunMode != ui.RunMode){
		printf("RunMode %d change to %d ", curUI.RunMode, ui.RunMode);
		curUI.RunMode = ui.RunMode;

		switch (curUI.RunMode)
		{
		case RM_OFF:
			printf("RM_OFF\n");
			ituTextSetString(TextSSTag, "RM_OFF");
			ituWidgetSetVisible(IconSpark, false);
			break;
		case RM_STANDBY:
			printf("RM_STANDBY\n");
			ituTextSetString(TextSSTag, ">|| TO START");
			ituWidgetSetVisible(IconSpark, false);
			if (waitForReturnIdle){//开门时，流程板从end状态变成standby
				waitForReturnIdle = false;
				returnIdleInitUI();
				StartProcessButtomDown();
			}
			break;
		case RM_RUNNING:
			printf("RM_RUNNING\n");
			if (isFirstInStartLayer){
				isFirstInStartLayer = false;
			}
			if (!waitForReturnIdle){
				waitForReturnIdle = true;
			}
			ituTextSetString(TextSSTag, ">|| TO STOP");
			ituWidgetSetVisible(IconSpark, true);
			if (!isRunningMode()){
				showIcons(4);
			}
			break;
		case RM_PAUSE:
			printf("RM_PAUSE\n");
			if (isFirstInStartLayer){
				isFirstInStartLayer = false;
			}
			ituTextSetString(TextSSTag, ">|| TO START");
			ituWidgetSetVisible(IconSpark, false);
			if (!isRunningMode()){
				showIcons(4);
			}
			break;
		case RM_ADD:
			printf("RM_ADD\n");
			ituTextSetString(TextSSTag, "RM_ADD");
			ituWidgetSetVisible(IconSpark, false);
			break;
		default:
			printf("no deal maybe an error\n");
			break;
		}
		ret = true;
	}
	return ret;
}

bool checkNtc()
{
	static u8 showNtc = 128;
	//printf("showNtc: %d",showNtc);
	if(showNtc !=isShowNtc())
		{
			printf("showNtc: %d",showNtc);

			showNtc = isShowNtc();
			if(isShowNtc())
				{
					printf("show ntcText:=======\n");

					ituWidgetSetVisible(ntcText, true);
					ituTextSetString(ntcText, getNTCValue());	
				}
			else
				{
					printf("hide ntcText:=======\n");
					ituWidgetSetVisible(ntcText, false);	
				}
				
		}
	else
		{
			if(ituWidgetIsVisible(ntcText))
				{
					if(isNtcValuechange())
						{
							printf("value change: %d",showNtc);
							ituTextSetString(ntcText, getNTCValue());	
						}
				}
		}
}
bool StartOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;

	ret |= checkTime();//剩余分钟变化时，UI刷新
	ret |= checkCost();//剩余花费变化时，UI刷新
	ret |= checkPayShow();//支付价格是否为零发生变化时，UI变化
	//ret |= checkStartandPause();//运行状态变化时，UI刷新
	ret |= checkWelcome();//超过一定待机时间时，UI待机
	ret |= checkCycleName();//程序变换时，回到first界面，并转换显示效果
	ret |= checkDryingStage();//运行阶段变化时，作出反应
	ret |= checkRunMode();//运行状态变化时，UI刷新
	ret |= checkNtc();

	//if (refreshLayerNum)//1 refreshLayerNum
	//{
	//	//refreshStartLayerUsedByTimer();
	//	refreshLayerNum = false;
	//	ret |= true;
	//}
	if (refreshLayerFlag){
		refreshLayerFlag = false;
		ret |= true;
	}
	return ret;
}

bool StartOnLeave(ITUWidget* widget, char* param)
{
	waitForReturnIdle = false;
	curUI.CycleName = -1;
	return true;
}
