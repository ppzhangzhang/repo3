#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"
#include "layer_ctrl.h"
#include "boardstate.h"


ITUText* pointText;
//ITUSprite* testSprite;
int pointLoc = 0;//入口箭头
#define totalChoice 4


void freshPointText(){//更改箭头位置
	ituWidgetSetY(pointText, 100 + pointLoc * 30);
}
bool gl_turnRight(){
	if (pointLoc <= 0){
		pointLoc = totalChoice - 1;
	}
	else{
		pointLoc--;
	}
	freshPointText();
	return true;
}
bool gl_turnLeft(){
	if (pointLoc >= totalChoice - 1){
		pointLoc = 0;
	}
	else{
		pointLoc++;
	}
	freshPointText();
	return true;
}
void gotoprodmodLayer(){
	//findWidget(testSprite);//houda adda
	ituLayerGoto(prodmodLayer);//测试 UI
	//ituSpriteGoto(testSprite, 0);
}
bool gl_btnPress(){
	switch (pointLoc)
	{
	case 0:
		boardsetCycleCMD(cx_tst_ui);//houda add 1203
		ituLayerGoto(prodmodLayer);//测试 UI
		//ituSpriteGoto(testSprite, 0);
		break;
	case 1:
		boardsetCycleCMD(cx_tst_run);//测试 RUN FB
		break;
	case 2:
		boardsetCycleCMD(cx_machine_test);//测试 全流程 FC
		break;
	case 3:
		boardsetCycleCMD(cx_pcb_test);//测试 PCB FD
		break;
	default:
		
		break;
	}
	return true;
}

bool GototestOnEnter(ITUWidget* widget, char* param)
{
	//findWidget(testSprite);//houda modi 1203
	findWidget(pointText);
	pointLoc = 0;
	freshPointText();
	ButtonUpOnPress = &gl_turnRight;
	ButtonDownOnPress = &gl_turnLeft;
	ProcessButtomDown = &gl_btnPress;
	return true;
}

bool GototestOnTimer(ITUWidget* widget, char* param)
{
	return false;
}

bool GototestOnLeave(ITUWidget* widget, char* param)
{
	/*sceneTurnRight = &sceneTurnRightVoid;
	sceneTurnLeft = &sceneTurnLeftVoid;
	sceneBtnPress = &sceneBtnPressVoid;*/
	return true;
}

