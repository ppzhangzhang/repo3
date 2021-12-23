#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "layer_ctrl.h"
#include "boardstate.h"

#define SIZEOFUI_SPECIAL 0x10//总长度60 校验码2
static char showData[SIZEOFUI_SPECIAL] = { 0 };
static bool result = false;

static flag160 = false;
static u8 databefore[20] = { 0 };
static u8 dataafter[48] = { 0 };
ITUTextBox* specialModeTextBox;

static uint8_t gbstr[] = { 0xb2, 0xfa, 0xcf, 0xdf, 0xbc, 0xec, 0xb2, 0xe2, 0x0a, 0xb3, 0xcc, 0xd0, 0xf2, 0xb0, 0xe6, 0xb1, 0xbe, 0x3a, 0x38, 0x41, 0x31, 0x39, 0x24, 0 };
//uint8_t gbstr[] = { 0xb2, 0xfa, 0xcf, 0xdf, 0xbc, 0xec, 0xb2, 0xe2};
static uint8_t step2[] = { 0x32, 0x2e, 0xb0, 0xda, 0xd7, 0xaa, 0xbc, 0xb0, 0xbb, 0xf4, 0xb6, 0xfb, 0xbc, 0xec, 0xb2, 0xe2, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x24 };

static uint8_t dstest[] = { 0x73, 0x70, 0x69, 0x6e, 0x20, 0x73, 0x65, 0x74, 0x20, 0x73, 0x70, 0x65, 0x65, 0x64, 0x3a, 0x20, 0x30, 0x30, 0x30, 0x30, 0xd, 0xa };

void sysnUIDataAndBoardData160(){
	flag160 = true;
}
extern int
gb2312ToUtf8(
char*  ptDestText,
int  nDestLength,
char*  ptSrcText,
int  nSrcLength
);
void DateEncodeChange160(){
	memset(databefore, 0, sizeof(databefore));
	memset(dataafter, 0, sizeof(dataafter));
	memcpy(databefore, &ui_data160, SIZEOFUI_SPECIAL);
#if 1//ndef WIN32
	gb2312ToUtf8(dataafter, 48, databefore, SIZEOFUI_SPECIAL);//00 01 02 03 04 05 06 07 08 09 //uilogic 改成+10 测试改成+0
#endif
}
void sysnUIDataAndBoardData160byTimer(){
	DateEncodeChange160();
}
void sml_turnRight(){
	printf("up\n");
	outQsetTypeValue(WASH_TstSpecialNum, 0x01);
}
void sml_turnLeft(){
	printf("down\n");
	outQsetTypeValue(WASH_TstSpecialNum, 0x02);
}
void sml_btnPress(){
	printf("press\n");
}
extern void setQueryFlag(u8 flag);
extern void closeQueryFlag(u8 flag);
bool specialModeLayerOnEnter(ITUWidget* widget, char* param)
{
	ButtonUpOnPress = &sml_turnRight;//MainButtonUpOnPress
	ButtonDownOnPress = &sml_turnLeft;//MainButtonDownOnPress
	ProcessButtomDown = &sml_btnPress;
	findWidget(specialModeTextBox);
	setQueryFlag(QUERY_160_TO_170_10);
    return true;
}
void sceneTestSpecial(){

}
static u16 miaoFlag = 0;
bool specialModeLayerOnTimer(ITUWidget* widget, char* param)
{
	//if (miaoFlag++ >= 5)//5s
	//{
	//	miaoFlag = 0;
	//	outQquery(0x160);
	//}
	if (!specialModeTextBox){
		findWidget(specialModeTextBox);
	}

	if (ui.Choice2 != 0x10){
		ituLayerGoto(startLayer);
		return true;
	}
	bool result = false;
	if (flag160 || strcompare(databefore, (u8*)&ui_data160, 16) != -1){
		if (flag160){
			//sysnUIDataAndBoardData160byTimer();
			printf("nononononono\n");
			flag160 = false;
		}
		sysnUIDataAndBoardData160byTimer();
		if (strlen((char*)dataafter))
		{
			ituTextBoxSetString(specialModeTextBox, dataafter);
			result |= true;
		}
	}
//#ifdef WIN32
//	memset(databefore, 0, sizeof(databefore));
//	memset(dataafter, 0, sizeof(dataafter));
//	memcpy(databefore, dstest, sizeof(dstest));//step2//gbstr//dstest
//	gb2312ToUtf8(dataafter, 160, databefore, sizeof(databefore)-2);//
//#endif
	//if (strlen((char*)dataafter))
	//{
	//	//ituTextSetString(ftResultText, utf8Resultstr);
	//	ituTextBoxSetString(specialModeTextBox, dataafter);
	//	result |= true;

	//}
	if (result){
		printf("specialModeLayerOnTimer result true\n");
	}
	return result;
}

bool specialModeLayerOnLeave(ITUWidget* widget, char* param)
{
	closeQueryFlag(QUERY_160_TO_170_10);
	return true;
}
