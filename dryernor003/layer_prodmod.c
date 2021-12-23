#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "layer_ctrl.h"

#pragma execution_character_set("utf-8")

#define FIRST_POINT_PM 27
#define FIRST_POINT_ADD_PM 75

ITUSprite* testSprite;
int nextSprite = 0;

char prodmodStr[6][32] = {
	{ "Pass" }, { "Fail" },//{ "串口接收正常" }, { "串口无数据" },
	{ "Network is ready" }, { "Network is not ready" },
	{ "Bluetooth is ready" }, { "Bluetooth is not ready" },
};
#ifdef WIN32
char* prodTips[] = { "Ethernet", "WiFi", "Error", "None" };
#else
char* prodTips[] = { "测试网口，按下重启初始化WiFi", "测试WiFi，左右旋转正常，按下不重启", "奇怪的模式", "" };
#endif

static bool result = false;
static bool local_busComOK = false;
static bool local_networkisready = false;
static bool local_bluetoothOK = false;

ITUText* TextEthernetOK;
ITUText* TextBusComOK;
ITUText* TextBluetoothOK;
ITUText* TextPointWLA;
ITUBackground* testBackground;
ITUBackground* Backgroundbt1, *Backgroundbt2, *Backgroundbt3;

ITUText* TextTestFlag;//test times
ITUText* TextExitFlag;//exit test times
ITUText* TextProdTips;//Content Assist

extern void CreateWorkerBTThread();
extern void CreateWorkerWIFIThread();

extern void TestFuncBTFuncExit();
extern bool TestFuncBTFuncisRun();
extern void TestFuncWIFIFuncExit();
extern bool TestFuncWIFIFuncisRun();


void initSetNetText(){
	if (local_networkisready){
		ituTextSetString(TextEthernetOK, prodmodStr[0]);
		ituWidgetSetColor(TextEthernetOK, 0xFF, 0xFF, 0xFF, 0xFF);
	}
	else{
		ituTextSetString(TextEthernetOK, prodmodStr[1]);
		ituWidgetSetColor(TextEthernetOK, 0xFF, 0xFF, 0x00, 0x00);
	}
}
bool checkSetNetText(){
	////写死
	//if (local_networkisready != true){
	//	local_networkisready = true;
	//	initSetNetText();
	//	return true;
	//}
	//return true;
	////写死 end
#ifdef WIN32
	if (local_networkisready != true){
		local_networkisready = true;

#else
	if (local_networkisready != NetworkIsReady()){
#ifdef CFG_NET_ENABLE
		local_networkisready = NetworkIsReady();
#else
		local_networkisready = false;
#endif
#endif
		initSetNetText();
		return true;
	}
	return false;
}

void initSetBusText(){
	if (local_busComOK){
		ituTextSetString(TextBusComOK, prodmodStr[0]);
		ituWidgetSetColor(TextBusComOK, 0xFF, 0xFF, 0xFF, 0xFF);
	}
	else{
		ituTextSetString(TextBusComOK, prodmodStr[1]);
		ituWidgetSetColor(TextBusComOK, 0xFF, 0xFF, 0x00, 0x00);
	}
}
int busNotOkTimes = 0;
bool checkSetBusText(){
	if (busNotOkTimes != 0){
		printf("local_busComOK = %d,busComOK = %d,busNotOkTimes = %d\n", local_busComOK, busComOK, busNotOkTimes);
	}
	if (local_busComOK != busComOK){
		local_busComOK = busComOK;

		if (local_busComOK == true){
			if (busNotOkTimes != 0){
				busNotOkTimes = 0;
			}
			initSetBusText();
			return true;
		}
	}

	if (local_busComOK == false){
		busNotOkTimes++;
		if (busNotOkTimes >= 2){
			initSetBusText();
			return true;
		}
	}
	return false;
}

void initSetBTText(){
	if (local_bluetoothOK){
		ituTextSetString(TextBluetoothOK, prodmodStr[0]);
		ituWidgetSetColor(TextBluetoothOK, 0xFF, 0xFF, 0xFF, 0xFF);
	}
	else{
		ituTextSetString(TextBluetoothOK, prodmodStr[1]);
		ituWidgetSetColor(TextBluetoothOK, 0xFF, 0xFF, 0x00, 0x00);
	}
}
bool checkSetBTText(){
	if (local_bluetoothOK != bluetoothOK){
		//printf("local_busComOK = %d\n", local_busComOK);
		local_bluetoothOK = bluetoothOK;
		initSetBTText();
		return true;
	}
	return false;
}


static int curSprite = 0;//
static int totalSprite = 9;//测试项目数量
static int TextPointWLA_Y = FIRST_POINT_PM;
static bool testKeyFlag = false;
#define wifitestSprite 8 //totalSprite-2


int getSpriteNum(int num){
	if (testKeyFlag){
		return 0;
	}
	if (ituWidgetIsVisible(TextPointWLA)){
		if (TextPointWLA_Y == FIRST_POINT_PM && num == -1){//当指针在最顶部且还要后退时，sprite后退一个，退出
			curSprite += num;
			ituWidgetSetVisible(TextPointWLA, false);
		}
		else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 3 + FIRST_POINT_PM) && num == 1){//达到上限
			//TextPointWLA_Y += (FIRST_POINT_ADD_PM * num);
		}
		else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 2 + FIRST_POINT_PM)){
			if (num == 1){
				TextPointWLA_Y += (75 * num);
			}
			else if (num == -1){
				TextPointWLA_Y += (FIRST_POINT_ADD_PM * num);
				CreateWorkerBTThread();
				CreateWorkerWIFIThread();
			}
			else{
				return 0;
			}
			
		}
		else{
			TextPointWLA_Y += (FIRST_POINT_ADD_PM * num);
			if (TextPointWLA_Y > (FIRST_POINT_ADD_PM * 3 + FIRST_POINT_PM)){
				TextPointWLA_Y -= (FIRST_POINT_ADD_PM * num);
			}
		}
		ituWidgetSetY(TextPointWLA, TextPointWLA_Y);
	}
	else{
		curSprite += num;
		if (curSprite < 0){
			curSprite = 0;
			return 0;
		}
		else if (curSprite >= totalSprite){
			curSprite = totalSprite - 1;
			return 0;
		}
	}
	
	switch (curSprite)
	{
	//case 6://totalSprite
	//	/*if (VideoOldTest->playing){
	//		ituVideoStop(VideoOldTest);
	//	}*/
	//	if (!ituWidgetIsVisible(settingWifiSsidLayer)){
	//		ituWidgetSetVisible(settingWifiSsidLayer, true);
	//	}
	//	break;
	//case 7://old test
	//	if (!ituWidgetIsVisible(prodmodLayer)){
	//		ituLayerGoto(prodmodLayer);
	//	}
	//	ituSpriteGoto(testSprite, 6);
	//	//ituVideoPlay(VideoOldTest, 0);
	//	break;
	default:
		if (!ituWidgetIsVisible(prodmodLayer)){
			ituLayerGoto(prodmodLayer);
		}
		ituSpriteGoto(testSprite, curSprite);
		if (curSprite == wifitestSprite){//指针 wifi 老化 测试界面
			/*ituWidgetSetVisible(Backgroundbt1, true);
			ituWidgetSetVisible(Backgroundbt2, true);
			ituWidgetSetVisible(Backgroundbt3, true);*/
			ituWidgetSetVisible(TextPointWLA, true);
		}
		
		break;
	}
	return curSprite;
}
extern void ConfigSaveExit(void);
static int curWiFiOKFlag = 0;
bool testKeyFuc(u8 flag){//按左键时，两个退出动作。
	if (curSprite == 0){
		if (flag == 3){//按下的情况
			if (curWiFiOKFlag != 1){
				theConfig.prodtestWiFi = 1;//下次开机测试WiFi
				ConfigSave();
				ConfigSaveExit();
			}
		}
		else{//旋转的情况
			if (curWiFiOKFlag != 1){
				return true;
			}
			else{
				//正常流程
			}
		}
	}
	if (ituWidgetIsVisible(settingWiFiSsidLayer)){//测试WiFi界面退回
		if (flag == 1){
			nextSprite = wifitestSprite;
			ituLayerGoto(prodmodLayer);
			//ituWidgetSetVisible(TextPointWLA, false);
			return true;
		}
		return true;
	}
	if (curSprite == totalSprite){//老化测试退回上一sprite
		if (flag == 1){
			curSprite = (totalSprite - 1);
			ituSpriteGoto(testSprite, curSprite);
			return true;
		}
		return true;
	}
	return false;

}
bool PMButtonUpOnPress(void){

	if (testKeyFuc(1)){
		return true;
	}
	
	getSpriteNum(-1);
	return true;
}
bool PMButtonDownOnPress(void){
	
	if (testKeyFuc(2)){
		return true;
	}

	getSpriteNum(+1);
	return true;
}
extern void exitProdLayer();
bool PMProcessButtomDown(void){
	if (testKeyFuc(3)){
		return true;
	}

	if (curSprite == (totalSprite - 1)){
		if (TextPointWLA_Y == FIRST_POINT_PM){
			if (!ituWidgetIsVisible(settingWiFiSsidLayer)){
				ituLayerGoto(settingWiFiSsidLayer);
			}
		}
		else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 1 + FIRST_POINT_PM)){//去到老化界面，wifi停止
			curSprite = totalSprite;
			ituSpriteGoto(testSprite, curSprite);
			TestFuncBTFuncExit();
			TestFuncWIFIFuncExit();
		}
		/*else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 2 + FIRST_POINT_PM)){
			testKeyFlag = true;
			ituWidgetSetVisible(Backgroundbt1, true);
			ituWidgetSetVisible(Backgroundbt2, true);
			ituWidgetSetVisible(Backgroundbt3, true);
		}*/
		else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 2 + FIRST_POINT_PM)){//记录测试
			theConfig.prodtestSum += 1;
			ConfigSave();
			textSetIntNumber(TextTestFlag, theConfig.prodtestSum);
		}
		else if (TextPointWLA_Y == (FIRST_POINT_ADD_PM * 3 + FIRST_POINT_PM)){
			theConfig.prodmod = 0;
			ConfigSave();
			exitProdLayer();//退出测试
		}
		
	}
	return true;
}
static ITUBackground* BackgroundColor1;//白色
static ITUBackground* BackgroundColor2;//红
static ITUBackground* BackgroundColor3;//绿
static ITUBackground* BackgroundColor4;//蓝
static ITUBackground* BackgroundColor5;//彩色
static ITUBackground* BackgroundColor6;//黑色

bool ProdmodOnEnter(ITUWidget* widget, char* param)
{
	theConfig.prodmod = 1;
	ConfigSave();
	ButtonUpOnPress = &PMButtonUpOnPress;
	ButtonDownOnPress = &PMButtonDownOnPress;
	ProcessButtomDown = &PMProcessButtomDown;
	findWidget(BackgroundColor2);
	findWidget(BackgroundColor3);
	findWidget(BackgroundColor4);
	ituWidgetSetColor2((ITUWidget*)BackgroundColor2, 0x00FF0000);
	ituWidgetSetColor2((ITUWidget*)BackgroundColor3, 0x0000FF00);
	ituWidgetSetColor2((ITUWidget*)BackgroundColor4, 0x000000FF);

	findWidget(TextBusComOK);
	findWidget(TextEthernetOK);
	
	findWidget(TextBluetoothOK);
	findWidget(TextPointWLA);
	findWidget(testBackground);
	findWidget(settingWiFiSsidLayer);
	findWidget(testSprite);
	findWidget(TextTestFlag);
	findWidget(TextExitFlag);
	findWidget(TextProdTips);
	if (nextSprite){//从wifi界面返回涉及到layer转换，enter
		curSprite = nextSprite;
		nextSprite = 0;
		ituSpriteGoto(testSprite, curSprite);

	}
	else{
		if (theConfig.prodtestWiFi == 0){
			curSprite = 0;
			ituSpriteGoto(testSprite, curSprite);
			ituTextSetString(TextProdTips,prodTips[0]);
			//从第一页sprite开始展示 网口串口蓝牙测试页面 开机初始化网口测试
		}
		else if (theConfig.prodtestWiFi == 1){
			curWiFiOKFlag = theConfig.prodtestWiFi;
			theConfig.prodtestWiFi = 0;//下次开机恢复正常流程。
			ConfigSave();
			//从第二页开始展示，跳过第一页，并且开机初始化wifi测试
			curSprite = 1;
			ituSpriteGoto(testSprite, curSprite);
			ituTextSetString(TextProdTips, prodTips[1]);
		}
		else{
			ituTextSetString(TextProdTips, prodTips[2]);
			theConfig.prodtestWiFi = 0;
			ConfigSave();
			curSprite = 0;
			ituSpriteGoto(testSprite, curSprite);
		}

	}
	
	findWidget(Backgroundbt1);
	findWidget(Backgroundbt2);
	findWidget(Backgroundbt3);
	ituWidgetSetVisible(Backgroundbt1, false);
	ituWidgetSetVisible(Backgroundbt2, false);
	ituWidgetSetVisible(Backgroundbt3, false);
	if (curSprite == 0 || curSprite == 1){
		ituWidgetSetVisible(TextPointWLA, false);
	}
	
	//DelayToSetConfigLang(0, 5);
	CreateWorkerBTThread();
	CreateWorkerWIFIThread();

	initSetBusText();
	initSetNetText();
	initSetBTText();
	ituWidgetSetColor2((ITUWidget*)testBackground, 0xFF0000FF);
	textSetIntNumber(TextTestFlag, theConfig.prodtestSum);
	textSetIntNumber(TextExitFlag, theConfig.prodexitSum);
	return true;
}
//int oldTestNum = 0;
//int coloroffset = 0;
//long color[20] = { 0xFF19CAAD, 0xFF8CC7B5, 0xFFA0EEE1, 0xFFBEE7E9, 0xFFBEEDC7, 0xFFD6D5B7, 0xFFD1BA74, 0xFFE6CEAC, 0xFFECAD9E, 0xFFF4606C ,
//0xFF2ae0c8, 0xFFa2e1d4, 0xFFacf6ef, 0xFFcbf5fb, 0xFFbdf3d4, 0xFFe6e2c3, 0xFFe3c887, 0xFFfad8be, 0xFFfbb8ac, 0xFFfe6673 };

int sendtimes = 0;
bool ProdmodOnTimer(ITUWidget* widget, char* param)
{
	//printf("busComOK = %d networkisready = %d \n", busComOK, local_networkisready);
	if (result)
		result = false;

	result |= checkSetBusText();
	result |= checkSetNetText();
	result |= checkSetBTText();

	return result;
}

bool ProdmodOnLeave(ITUWidget* widget, char* param)
{
	//wifi测试是另一个layer，无法在这里控制
	TestFuncBTFuncExit();
	return true;
}

void ProdmodReset()
{
	curSprite = 0;//
	TextPointWLA_Y = FIRST_POINT_PM;
	ituWidgetSetY(TextPointWLA, TextPointWLA_Y);
	testKeyFlag = false;
	//changeSpriteNum(0);
	TestFuncBTFuncExit();
	TestFuncWIFIFuncExit();

	//记录退出
	theConfig.prodexitSum += 1;
	ConfigSave();
}