#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "boardstate.h"
#include "layer_ctrl.h"

static uint8_t itemIndex = 0;
static  ITUText* ftItemText;
static  ITUText* ftNameText;
static  ITUText* ftResultText;
static  ITUText* RunningText;
static ITUTextBox* FMonTextBox = NULL;
static  ITUBackground * ftTestbg;
static ITUSprite *sp;
static bool refreshLayerNum = false;


#define TESTMAXNUM 6
#ifdef WIN32
char * testItems[] =
{
	"1111",
	"2222",
	"3333",
	"4444",
	"5555",
	"6666",

};
char * runningText = "check...";
uint8_t gbstr[] = { 0xb2, 0xfa, 0xcf, 0xdf, 0xbc, 0xec, 0xb2, 0xe2, 0x0a, 0xb3, 0xcc, 0xd0, 0xf2, 0xb0, 0xe6, 0xb1, 0xbe, 0x3a, 0x38, 0x41, 0x31, 0x39, 0x24, 0 };
//uint8_t gbstr[] = { 0xb2, 0xfa, 0xcf, 0xdf, 0xbc, 0xec, 0xb2, 0xe2};
uint8_t step2[] = { 0x32, 0x2e, 0xb0, 0xda, 0xd7, 0xaa, 0xbc, 0xb0, 0xbb, 0xf4, 0xb6, 0xfb, 0xbc, 0xec, 0xb2, 0xe2, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x24 };

uint8_t dstest[] = { 0x73, 0x70, 0x69, 0x6e, 0x20, 0x73, 0x65, 0x74, 0x20, 0x73, 0x70, 0x65, 0x65, 0x64, 0x3a, 0x20, 0x30, 0x30, 0x30, 0x30, 0xd, 0xa };
#else
char * testItems[] =
{
	"1、进水阀检测",
	"2、摆转及霍尔检测",
	"3、丝杆及霍尔检测",
	"4、直流无刷检测",
	"5、上盖霍尔检测",
	"6、排水阀检测",

};
char * runningText = "检测中...";
#endif
bool tml_turnLeftFE(){//0xCB
	outQsetTypeValue(WASH_UniversalDialogBox, 0);
	return true;
}
bool tml_turnMiddleFE(){
	outQsetTypeValue(WASH_UniversalDialogBox, 1);
	return true;
}
bool tml_btnRightFE(){
	outQsetTypeValue(WASH_UniversalDialogBox, 2);
	return true;
}

void setTitle(){
#ifdef WIN32
	switch (ui.CycleName)
	{
	case 0xFB:
		ituTextSetString(ftNameText, "test run");
		break;
	case 0xFC:
		ituTextSetString(ftNameText, "test whole");
		break;
	case 0xFD:
		ituTextSetString(ftNameText, "test pcb");
		break;
	default:
		ituTextSetString(ftNameText, "test WP");
		break;
	}
#else
	switch (ui.CycleName)
	{
	case 0xFB:
		ituTextSetString(ftNameText, "试运行");
		break;
	case 0xFC:
		ituTextSetString(ftNameText, "整机测试");
		break;
	case 0xFD:
		ituTextSetString(ftNameText, "pcb测试");
		break;
	default:
		ituTextSetString(ftNameText, "test WP");
		break;
	}
#endif
}

bool testModeLayerOnEnter(ITUWidget* widget, char* param)
{

	printf("testModeLayerOnEnter\n");
	switch (ui.CycleName)
	{
	case 0xFB:
	case 0xFC:
	case 0xFD:
	case 0xFE:
		printf("testModeLayerOnEnter = 0xFE");
		ButtonUpOnPress = &tml_turnMiddleFE;//tml_turnLeftFE
		ButtonDownOnPress = &tml_turnLeftFE;//tml_turnMiddleFE
		ProcessButtomDown = &tml_btnRightFE;
		break;
	default:
		break;
	}
	/*sceneTurnRight = &tml_turnRight;
	sceneTurnLeft = &tml_turnLeft;
	sceneBtnPress = &tml_btnPress;*/
	switch (ui.CycleName)
	{
	default:
		break;
	}
	if (ftItemText == NULL)
	{
		ftItemText = (ITUText*)ituSceneFindWidget(&theScene, "ftItemText");
		assert(ftItemText);
	}
	if (ftNameText == NULL)
	{
		ftNameText = (ITUText*)ituSceneFindWidget(&theScene, "ftNameText");
		assert(ftNameText);
	}
	setTitle();
	if (ftResultText == NULL)
	{
		ftResultText = (ITUText*)ituSceneFindWidget(&theScene, "ftResultText");
		assert(ftResultText);
	}
	if (RunningText == NULL)
	{
		RunningText = (ITUText*)ituSceneFindWidget(&theScene, "RunningText");
		assert(RunningText);
	}
	if (FMonTextBox == NULL)
	{
		FMonTextBox = (ITUTextBox*)ituSceneFindWidget(&theScene, "FMonTextBox");
		assert(FMonTextBox);
	}

	if (sp == NULL)
	{
		sp = (ITUSprite*)ituSceneFindWidget(&theScene, "ftSprite");
		assert(sp);
	}
	if (ftTestbg == NULL)
	{
		ftTestbg = (ITUBackground*)ituSceneFindWidget(&theScene, "ftTestbg");
		assert(ftTestbg);
	}
	itemIndex = 0;
#ifdef WIN32
	//ituWidgetSetVisible(ftTestbg, true);
	ituWidgetSetVisible(ftItemText, false);
#else
	ituWidgetSetVisible(ftTestbg, false);
	ituWidgetSetVisible(ftItemText, false);
#endif
	//	ituTextSetString(ftItemText, testItems[itemIndex]);
	return true;
}

void setTestParaItem(uint8_t type)
{
	//set_machine_runpara(UIEVENT_TstStep, type);
}
void setTestParaItemPre(uint8_t type)
{

	//	0=上一步(last step)/1=下一步(next step)
	//2=启停(start,pause)/3=返回(back)
	//set_machine_runpara(UIEVENT_TstStep,0);
}
bool testItemNext(ITUWidget* widget, char* param)
{
	printf("testItemNext===========");

#ifdef __UIDISPLAY_BY_BOARDSTATE__
	//if(itemIndex+1>=TESTMAXNUM)// how to cycle
	{
		//setRunningParaType( 1);
	}
	// else
	{
		itemIndex++;
		if (itemIndex >= TESTMAXNUM)
		{
			itemIndex = 0;
		}
		ituSpriteGoto(sp, itemIndex);
		setTestParaItem(TstStep_NEXT);
		ituWidgetSetVisible(ftTestbg, false);
		//	ituWidgetSetVisible(ftItemText, true);
	}
#else
	itemIndex++;
	if (itemIndex >= TESTMAXNUM)
	{
		itemIndex = 0;
	}
	//setRunningParaType(itemIndex);
	//	ituTextSetString(ftNameText, testItems[itemIndex]);
	ituSpriteGoto(sp, itemIndex - 1);
#endif


	return true;
}

bool startTestItem(ITUWidget* widget, char* param)
{
	printf("startTestItem===========");
	ituWidgetSetVisible(ftTestbg, false);
	ituWidgetSetVisible(ftItemText, false);
	//setTestParaItem(TstStep_START);
	return true;
}

void refreshTestModeLayer(){
	if (!refreshLayerNum){
		refreshLayerNum = true;
	}
}
#define SIZEOFUI_TM 0x60//总长度60 校验码2
u8  testResultHint[SIZEOFUI_TM];
u8  utf8Resultstr[164];//转换后变长
extern int
gb2312ToUtf8(
char*  ptDestText,
int  nDestLength,
char*  ptSrcText,
int  nSrcLength
);
static char showData[SIZEOFUI_TM + 2] = { 0 };
static bool result = false;
void DateEncodeChange(u8* data)
{
	int i = 0;
	/*printf("test recv:\n");
	for (; i < SIZEOFUI_TM; i++)
	printf("%02x  ", ((u8*)data)[i]);
	printf("\n");*/
	memset(testResultHint, 0, SIZEOFUI_TM);
	memset(utf8Resultstr, 0, 164);
	memcpy(testResultHint, data, SIZEOFUI_TM);
#if 1//ndef WIN32
	gb2312ToUtf8(utf8Resultstr, 160, testResultHint + 10, SIZEOFUI_TM - 10);//00 01 02 03 04 05 06 07 08 09 //uilogic 改成+10 测试改成+0 减去校验码两个字节
#endif

#if 0//test
	memset(utf8Resultstr, 0, 164);
	//memcpy(testResultHint, gbstr, sizeof(gbstr));
	//memcpy(testResultHint, step2, sizeof(step2));
	uint8_t dstest[] = { 0x73, 0x70, 0x69, 0x6e, 0x20, 0x73, 0x65, 0x74, 0x20, 0x73, 0x70, 0x65, 0x65, 0x64, 0x3a, 0x20, 0x30, 0x30, 0x30, 0x30, 0xd, 0xa };
	memcpy(testResultHint, dstest, sizeof(dstest));
	gb2312ToUtf8(utf8Resultstr, 160, testResultHint, SIZEOFUI_TM - 12);//00 01 02 03 04 05 06 07 08 09 //uilogic 改成+10 测试改成+0
#endif
	/*printf("UTF8:\n");
	for (i = 0; i<SIZEOFUI_TM; i++)
		printf("%02x  ", utf8Resultstr[i]);
	printf("\n");*/
}

void refreshTestModeLayerUsedByTimer(){
	if (strcompare(showData, &ui_tm, sizeof(showData)) != -1){
		result = true;
		memcpy(showData, &ui_tm, sizeof(showData));
		DateEncodeChange((u8*)&ui_tm);
	}
}



bool testModeLayerOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;
	result = false;
	if (refreshLayerNum){
		refreshTestModeLayerUsedByTimer();
		refreshLayerNum = false;
		ret |= true;
	}
	//memcpy((u8*)&ui_tm + 10, dstest, sizeof(step2));//step2//gbstr
	refreshTestModeLayerUsedByTimer();

	// printf("startTestItem===========");
//#ifdef WIN32
//	gb2312ToUtf8(utf8Resultstr, 160, gbstr, sizeof(gbstr));
//#endif
//#ifdef WIN32
//	int len = 0;
//	uint8_t line = 0;
//
//	while ((step2[len] != 0) && (step2[len] != 0x24))
//	{
//		if (step2[len] == 0xa)
//		{
//			line++;
//		}
//		len++;
//
//	}
//	printf("recv len is %d \n", len);
//
//	gb2312ToUtf8(utf8Resultstr, 160, step2, sizeof(step2));
//#endif
//#ifdef WIN32
//	memset(testResultHint, 0, SIZEOFUI_TM);
//	memset(utf8Resultstr, 0, 164);
//	memcpy(testResultHint, dstest, sizeof(dstest));//step2//gbstr//dstest
//	gb2312ToUtf8(utf8Resultstr, 160, testResultHint, SIZEOFUI_TM - 2);//
//#endif
	if (result){
		result = false;
		if (strlen((char*)utf8Resultstr))
		{
			//ituTextSetString(ftResultText, utf8Resultstr);
			ituTextBoxSetString(FMonTextBox, utf8Resultstr);
			ret |= true;
		}
	}
	//ituWidgetSetVisible(ftTestbg, true);
	//ituWidgetSetVisible(ftItemText, false);
	//setTestParaItem(TstStep_START);
	return ret;
}