#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

#include "boardstate.h"
#include "layer_ctrl.h"
#include "layer_main.h"
#include "projutils.h"
static char strmac[18];

static bool refreshServiceLayer = false;
#define MOVETEXTSIZE 40
#define BLINKLAST 500 //0.5s

#define YELLOW_FOR_MAIN 0xffffff00
#define BLACK_FOR_MAIN 0xff000000

#define GREEN_FOR_HAPPYDAY		0xffadff2f	//
#define RED_FOR_HAPPYDAY		0xffff4500	//

u8 TIME_ShowTypeampm;
u8 TIME_ShowType1224;

u8 editMode=0;

typedef struct{
	u8 data114;
	u8 data1213;
	u8 data1227;
	u8 data13401;
	u8 data13414;
	u8 data14201;
	int data1333;//zpp
	//u8 data133407;//zpp
	WPEVENT_PROGRAM_PARAMRTER dataPP;//200
	WPEVENT_DISCOUNT_PARAMRTER dataDP;//300
	WPEVENT_INFO_PARAMRTER dataIP;//400
	WPEVENT_INFO_PARAMRTER_TYPE_COUNT data43C;
	WPEVENT_CTRL_PARAMRTER dataCP;//500
	WPEVENT_MACHINE_STATE dataMS;//600
	WPEVENT_MACHINE_RUN_STATE dataMRS;//700
}ShowData;


//char cmac[6]={0};
char cmac[6]={2};

char * getMacStr()
 {
  	char i;
	#ifdef CFG_NET_WIFI
 	wifiMgr_get_Mac_address(cmac);
	#else
	
	#endif
	//
	memset(strmac,0,18);
	for(i=0;i<6;i++)
		{printf("----------------------------------------------------------------\n");
		
                    printf("61mac:%02x\n",cmac[i]);

			sprintf(&strmac[i*3],"%02x-",(unsigned char)cmac[i]);
			printf("64mac:%s\n",strmac);

		}
	printf("67mac:%s\n",strmac);
		//printf("mac:%\n",cmac);

	return strmac;
 }//macaddress_zpp


ShowData UICtrlShowData = { 0 };
/*
温度指针
*/
TimePriceType* tp;//四种温度同时只显示一个，所以将此指针指向 boardstate.c->pp 某一温度来调取各种参数。
TimePriceType* UI_tp;//四种温度同时只显示一个，所以将此指针指向 UICtrlShowData->dataPP 某一温度来调取各种参数。
/*
折扣指针
*/
WPEVENT_DISCOUNT_PARAMRTER_TYPE* hht = &dp.dc0;//10个折扣中的一个
WPEVENT_DISCOUNT_PARAMRTER_TYPE* UI_hht = &(UICtrlShowData.dataDP.dc0);
u8 dateVlue[8];//zpp
u8 timeVlue[8];

u8 dateVlueH1 = 2;
u8 dateVlueH2 = 0;//zpp



u8 flag11111 = 0;//11111设置项，有四种状态：未选中，设置百位，设置十位，设置个位；
u8 flag1131 = 0; //有6种状态 08 30 A
u8 flag1132 = 0;//有6种状态 12 30 P
int8_t flag1134 = 0;//有8种状态 M Tu W Th F Sa Su exit
u8 offset8 = 0x01;//0x01 按位移动 最多种状态。
static bool refreshLayerFlag = false; //如果某一次刷新出现延迟，在指令后加上 refreshLayerFlag = true; 可以在下一帧刷新。可能17ms。
u8 buffer[5] = { 0 };

typedef struct MainShow{//菜单
	int currentFocus;
	int firstMenuNum;
	int lastMenuNum;
	ITUBackground *MenuMoveBackground;
	ITUBackground *CurrentShowArea;
}forShow;
forShow ShowConfig;



void TextToYellow(ITUText* text){
	ituWidgetSetColor2((ITUWidget*)text, YELLOW_FOR_MAIN);
	//text->widget.color = makeColor(YELLOW_FOR_MAIN);
}
void TextToBlack(ITUText* text){
	ituWidgetSetColor2((ITUWidget*)text, BLACK_FOR_MAIN);
	//text->widget.color = makeColor(BLACK_FOR_MAIN);
}
void textOfBG2Yellow(ITUBackground *background){
	if (((ITUText *)background)->widget.type == ITU_TEXT){//text变色
		TextToYellow(((ITUText *)background));
		return;
	}
	if (background->icon.widget.type == ITU_BACKGROUND && background->icon.widget.tree.child != NULL){
		for (ITUBackground* i = (ITUBackground*)background->icon.widget.tree.child; i != NULL; i = (ITUBackground *)i->icon.widget.tree.sibling){//background下的一层background的第一层text都变色
			for (ITUText* j = (ITUText*)i->icon.widget.tree.child; j != NULL; j = (ITUText *)j->widget.tree.sibling){
				if (j->widget.type == ITU_TEXT){
					TextToYellow(j);
				}
			}
		}
		for (ITUText * i = (ITUText *)background->icon.widget.tree.child; i != NULL; i = (ITUText *)i->widget.tree.sibling){//background下的第一层text都变色
			if (i->widget.type == ITU_TEXT){
				TextToYellow(i);
			}
		}
		return;
	}
}
void textOfBG2Black(ITUBackground *background){
	if (((ITUText *)background)->widget.type == ITU_TEXT){
		TextToBlack(((ITUText *)background));
		return;
	}
	if (background->icon.widget.type == ITU_BACKGROUND && background->icon.widget.tree.child != NULL){
		for (ITUBackground* i = (ITUBackground*)background->icon.widget.tree.child; i != NULL; i = (ITUBackground *)i->icon.widget.tree.sibling){//background下的一层background的第一层text都变色
			for (ITUText* j = (ITUText*)i->icon.widget.tree.child; j != NULL; j = (ITUText *)j->widget.tree.sibling){
				if (j->widget.type == ITU_TEXT){
					TextToBlack(j);
				}
			}
		}
		for (ITUText * i = (ITUText *)background->icon.widget.tree.child; i != NULL; i = (ITUText *)i->widget.tree.sibling){//background下的第一层text都变色
			if (i->widget.type == ITU_TEXT){
				TextToBlack(i);
			}
		}
		return;
	}

}
static const char* textleft[] =
{
	"Main Menu", "PROGRAM SETTINGS", "TIME/PRICING", "HIGH", "MEDIUM", "LOW", "NO HEAT",
	"TEMPERATURE SETTINGS",//TEMPERATURE SETTINGS TEMP SETTINGS
	"HAPPY HOUR", "1.1.1.2", "1.1.1.3", "1.1.1.4", "1.1.1.5",//8
	"1.1.1.1.1", "1.1.1.1.2", "1.1.1.1.3", "1.1.1.1.4", "1.1.1.1.5", "1.1.1.1.6", "1.1.1.1.7",
	"", "", "", "", "",
};


static const char* textright[] =
{
	"1.1", "1.2", "1.3", "1.4", "1.5",//0 4
	"1.1.1", "1.1.2", "1.1.3", "1.1.4", "1.1.5",//5 9
	"1.1.1.1", "1.1.1.2", "1.1.1.3", "1.1.1.4", "1.1.1.5",//10 14
	"1.1.1.1.1", "1.1.1.1.2", "1.1.1.1.3", "1.1.1.1.4", "1.1.1.1.5", "1.1.1.1.6", "1.1.1.1.7",//15 21
	"1.1.1.2.1", "1.1.1.2.2", "1.1.1.2.3", "1.1.1.2.4", "1.1.1.2.5", "1.1.1.2.6", "1.1.1.2.7",//15+theCache.temp*7 21+theCache.temp*7
	"1.1.1.3.1", "1.1.1.3.2", "1.1.1.3.3", "1.1.1.3.4", "1.1.1.3.5", "1.1.1.3.6", "1.1.1.3.7",
	"1.1.1.4.1", "1.1.1.4.2", "1.1.1.4.3", "1.1.1.4.4", "1.1.1.4.5", "1.1.1.4.6", "1.1.1.4.7",//36 42
	"1.1.2.1", "1.1.2.2", "1.1.2.3", "1.1.2.4", "1.1.2.5",//43 47
	"1.1.2.1.1", "1.1.2.1.2", "1.1.2.1.3", "1.1.2.1.4", "1.1.2.1.5",//48 52
	"1.1.2.2.1", "1.1.2.2.2", "1.1.2.2.3", "1.1.2.2.4", "1.1.2.2.5",//48+theCache.temp*5 52+theCache.temp*5 
	"1.1.2.3.1", "1.1.2.3.2", "1.1.2.3.3", "1.1.2.3.4", "1.1.2.3.5",
	"1.1.2.4.1", "1.1.2.4.2", "1.1.2.4.3", "1.1.2.4.4", "1.1.2.4.5",//63 67
	"1.1.3.1", "1.1.3.2", "1.1.3.3", "1.1.3.4", "1.1.3.5",//68 72
	"1.1.3.1a", "1.1.3.2a", "1.1.3.3a", "1.1.3.4a", "1.1.3.5a",//68+theCache.promo*5  72+theCache.promo*5
	"1.1.3.1b", "1.1.3.2b", "1.1.3.3b", "1.1.3.4b", "1.1.3.5b",
	"1.1.3.1c", "1.1.3.2c", "1.1.3.3c", "1.1.3.4c", "1.1.3.5c",
	"1.1.3.1d", "1.1.3.2d", "1.1.3.3d", "1.1.3.4d", "1.1.3.5d",
	"1.1.3.1e", "1.1.3.2e", "1.1.3.3e", "1.1.3.4e", "1.1.3.5e",
	"1.1.3.1f", "1.1.3.2f", "1.1.3.3f", "1.1.3.4f", "1.1.3.5f",
	"1.1.3.1g", "1.1.3.2g", "1.1.3.3g", "1.1.3.4g", "1.1.3.5g",
	"1.1.3.1h", "1.1.3.2h", "1.1.3.3h", "1.1.3.4h", "1.1.3.5h",
	"1.1.3.1i", "1.1.3.2i", "1.1.3.3i", "1.1.3.4i", "1.1.3.5i",//113 117
	"", "", "", "", "",
};
char stringpromo[32] = "PROMO 1";

void promocpy(){
	sprintf(stringpromo + 6, "%d", theCache.promo + 1);
	ituTextSetString(TextMenu11313, stringpromo);
}

void leftSetString(const char* string){
	char str[32] = { 0 };
	strcpy(str, string);
	ituTextSetString(TextLogoLeft, str);
}
void rightSetString(const char* string){
	char str[32] = { 0 };
	strcpy(str, string);
	ituTextSetString(TextLogoRight, str);
}
void updateLogo(){//补丁
	switch (theCache.temp){
	case HIGH:
		leftSetString("HIGH");
		break;
	case MIDIUM:
		leftSetString("MEDIUM");
		break;
	case LOW:
		leftSetString("LOW");
		break;
	case NOHEAT:
		leftSetString("NO HEAT");
		break;
	default:
		break;
	}

	if (theCache.promo != (u8)(-1)){//十个打折
		promocpy();
	}
	switch (ShowConfig.currentFocus){
	case 11://Main Menu 1.1
		leftSetString("Main Menu");
		rightSetString("1.1");
		break;
	case 12:
		rightSetString("1.2");
		break;
	case 13:
		rightSetString("1.3");
		break;
	case 14:
		rightSetString("1.4");
		break;
	case 15:
		rightSetString("1.5");
		break;
	case 111://PROGRAM SETTINGS 1.1.1
		leftSetString("PROGRAM SETTINGS");
		rightSetString("1.1.1");
		break;
	case 112:
		rightSetString("1.1.2");
		break;
	case 113:
		rightSetString("1.1.3");
		break;
	case 114:
		rightSetString("1.1.4");
		break;
	case 115:
		rightSetString("1.1.5");
		break;
	case 1111://PROGRAM SETTINGS 1.1.1.1
		leftSetString("TIME/PRICING");
		rightSetString("1.1.1.1");
		break;
	case 1112:
		rightSetString("1.1.1.2");
		break;
	case 1113:
		rightSetString("1.1.1.3");
		break;
	case 1114:
		rightSetString("1.1.1.4");
		break;
	case 1115:
		rightSetString("1.1.1.5");
		break;
	case 11111:
		rightSetString(textright[8 + theCache.temp * 7]);
		break;
	case 11112:
		rightSetString(textright[9 + theCache.temp * 7]);
		break;
	case 11113:
		rightSetString(textright[10 + theCache.temp * 7]);
		break;
	case 11114:
		rightSetString(textright[11 + theCache.temp * 7]);
		break;
	case 11115:
		rightSetString(textright[12 + theCache.temp * 7]);
		break;
	case 11116:
		rightSetString(textright[13 + theCache.temp * 7]);
		break;
	case 11117:
		rightSetString(textright[14 + theCache.temp * 7]);
		break;
	case 1121://1-2
		leftSetString("TEMP SETTINGS");
		rightSetString("1.1.2.1");
		break;
	case 1122:
		rightSetString("1.1.2.2");
		break;
	case 1123:
		rightSetString("1.1.2.3");
		break;
	case 1124:
		rightSetString("1.1.2.4");
		break;
	case 1125:
		rightSetString("1.1.2.5");
		break;
	case 11211:
		rightSetString(textright[43 + theCache.temp * 5]);
		break;
	case 11212:
		rightSetString(textright[44 + theCache.temp * 5]);
		break;
	case 11213:
		rightSetString(textright[45 + theCache.temp * 5]);
		break;
	case 11214:
		rightSetString(textright[46 + theCache.temp * 5]);
		break;
	case 11215:
		rightSetString(textright[47 + theCache.temp * 5]);
		break;
	case 1131:
		leftSetString("HAPPY HOUR");
		rightSetString(textright[68 + theCache.promo * 5]);
		break;
	case 1132:
		rightSetString(textright[69 + theCache.promo * 5]);
		break;
	case 1133:
		rightSetString(textright[70 + theCache.promo * 5]);
		break;
	case 1134:
		rightSetString(textright[71 + theCache.promo * 5]);
		break;
	case 1135:
		rightSetString(textright[72 + theCache.promo * 5]);
		break;
	case 121://
		leftSetString("STATISTICS");
		rightSetString("1.2.1");
		break;
	case 122://
		rightSetString("1.2.2");
		break;
	case 123://
		rightSetString("1.2.3");
		break;
	case 124://
		rightSetString("1.2.4");
		break;
	case 125://
		rightSetString("1.2.5");
		break;
	case 1211://
		leftSetString("COLLECTIONS");
		rightSetString("1.2.1.1");
		break;
	case 1212://
		rightSetString("1.2.1.2");
		break;
	case 1213://
		rightSetString("1.2.1.3");
		break;
	case 1214://
		rightSetString("1.2.1.4");
		break;
	case 1221://
		leftSetString("COUNTERS");
		rightSetString("1.2.2.1");
		break;
	case 1222://
		rightSetString("1.2.2.2");
		break;
	case 1223://
		rightSetString("1.2.2.3");
		break;
	case 1224://
		rightSetString("1.2.2.4");
		break;
	case 1225://
		rightSetString("1.2.2.5");
		break;
	case 1226://
		rightSetString("1.2.2.6");
		break;
	case 1227://
		rightSetString("1.2.2.7");
		break;
	case 1228://
		rightSetString("1.2.2.8");
		break;
	case 1231://
		leftSetString("COIN BOX ACCESS LOG");
		rightSetString("1.2.3.1");
		break;
	case 1232://
		rightSetString("1.2.3.2");
		break;
	case 1233://
		rightSetString("1.2.3.3");
		break;
	case 1234://
		rightSetString("1.2.3.4");
		break;
	case 1235://
		rightSetString("1.2.3.5");
		break;
	case 1241://
		leftSetString("ERROR LOG");
		rightSetString("1.2.4.1");
		break;
	case 1242://
		rightSetString("1.2.4.2");
		break;
	case 1243://
		rightSetString("1.2.4.3");
		break;
	case 1244://
		rightSetString("1.2.4.4");
		break;
	case 1245://
		rightSetString("1.2.4.5");
		break;
	case 131:
		leftSetString("CONFIGURATION");
		rightSetString("1.3.1");
		break;
	case 1311:
		leftSetString("PAYMENT SETTINGS");
		rightSetString("1.3.1.1");
		break;
	case 1312:
		rightSetString("1.3.1.2");
		break;
	case 1313:
		rightSetString("1.3.1.3");
		break;
	case 1314:
		rightSetString("1.3.1.4");
		break;
	case 1315:
		rightSetString("1.3.1.5");
		break;
	case 1321:
		leftSetString("CONN SETTINGS");
		rightSetString("1.3.2.1");
		break;
	case 1322:
		rightSetString("1.3.2.2");
		break;
	case 1323:
		rightSetString("1.3.2.3");
		break;
	case 13231:
		leftSetString("WIFI");
		rightSetString("1.3.2.3.1");
		break;
	case 13232:
		rightSetString("1.3.2.3.2");
		break;
	case 13233:
		rightSetString("1.3.2.3.3");
		break;
	case 13234:
		rightSetString("1.3.2.3.4");
		break;
	case 13261:
		leftSetString("BLUETOOTH");
		rightSetString("1.3.2.6.1");
		break;
	case 13262:
		rightSetString("1.3.2.6.2");
		break;
	case 13263:
		rightSetString("1.3.2.6.3");
		break;
	case 1324:
		rightSetString("1.3.2.4");
		break;
	case 1325:
		rightSetString("1.3.2.5");
		break;
	case 1326:
		rightSetString("1.3.2.6");
		break;
	case 1327:
		rightSetString("1.3.2.7");
		break;
	case 132:
		rightSetString("1.3.2");
		break;
	case 133:
		rightSetString("1.3.3");
		break;
	case 1331:
		leftSetString("SET CLOCK");
		rightSetString("1.3.3.1");
		break;
	case 1332:
		rightSetString("1.3.3.2");
		break;
	case 1333:
		rightSetString("1.3.3.3");
		break;
	case 1334:
		rightSetString("1.3.3.4");
		break;
	case 1335:
		rightSetString("1.3.3.5");
		break;
	case 1336:
		rightSetString("1.3.3.6");
		break;
	case 134:
		rightSetString("1.3.4");
		break;
	case 13401:
		leftSetString("OTHER SETTINGS");
		rightSetString("1.3.4.1");
		break;
	case 13402:
		rightSetString("1.3.4.2");
		break;
	case 13403:
		rightSetString("1.3.4.3");
		break;
	case 13404:
		rightSetString("1.3.4.4");
		break;
	case 13405:
		rightSetString("1.3.4.5");
		break;
	case 13406:
		rightSetString("1.3.4.6");
		break;
	case 13407:
		rightSetString("1.3.4.7");
		break;
	case 13408:
		rightSetString("1.3.4.8");
		break;
	case 13409:
		rightSetString("1.3.4.9");
		break;
	case 13410:
		rightSetString("1.3.4.A");
		break;
	case 13411:
		rightSetString("1.3.4.B");
		break;
	case 13412:
		rightSetString("1.3.4.C");
		break;
	case 13413:
		rightSetString("1.3.4.D");
		break;
	case 13414:
		rightSetString("1.3.4.E");
		break;
	case 13415:
		rightSetString("1.3.4.F");
		break;
	case 13416:
		rightSetString("1.3.4.G");
		break;
	case 135:
		rightSetString("1.3.5");
		break;
	case 1351:
		leftSetString("PIN CODES");
		rightSetString("1.3.5.1");
		break;
	case 1352:
		rightSetString("1.3.5.2");
		break;
	case 1353:
		rightSetString("1.3.5.3");
		break;
	case 136:
		rightSetString("1.3.6");
		break;
	case 141:
		leftSetString("SERVICE MODE");
		rightSetString("1.4.1");
		break;
	case 14101:
		leftSetString("ACTIVATE OUTPUTS");
		rightSetString("1.4.1.1");
		break;
	case 14102:
		rightSetString("1.4.1.2");
		break;
	case 14103:
		rightSetString("1.4.1.3");
		break;
	case 14104:
		rightSetString("1.4.1.4");
		break;
	case 14105:
		rightSetString("1.4.1.5");
		break;
	case 14201:
		leftSetString("SHOW INPUTS");
		rightSetString("1.4.2.1");
		break;
	case 14202:
		rightSetString("1.4.2.2");
		break;
	case 14203:
		rightSetString("1.4.2.3");
		break;
	case 14204:
		rightSetString("1.4.2.4");
		break;
	case 14205:
		rightSetString("1.4.2.5");
		break;
	case 14206:
		rightSetString("1.4.2.6");
		break;
	case 14207:
		rightSetString("1.4.2.7");
		break;
	case 14208:
		rightSetString("1.4.2.8");
		break;
	case 14209:
		rightSetString("1.4.2.9");
		break;
	case 14210:
		rightSetString("1.4.2.A");
		break;
	case 143:
		rightSetString("1.4.3");
		break;
	case 144:
		rightSetString("1.4.4");
		break;
	case 1441:
		leftSetString("UPDATE");
		rightSetString("1.4.4.1");
		break;
	case 145:
		rightSetString("1.4.5");
		break;
	default:
	{
			   int i = ShowConfig.currentFocus;
			   int num = 0;
			   int j = 0;
			   char str[32] = { 0 };

			   while (i > 0){//123 to 3.2.1.
				   num = i % 10;
				   i /= 10;
				   sprintf(&str[j], "%d.", num);
				   j += 2;
			   }
			   j--;
			   str[j] = '\0';//3.2.1. to 3.2.1

			   strreversal(str);//3.2.1 to 1.2.3
			   rightSetString(str);
	}
		break;
	}
}
void initLogo(){
	updateLogo();
}

void initShowConfig(int firstMenuNum, int lastMenuNum, ITUBackground *MenuMoveBackground){
	ShowConfig.currentFocus = firstMenuNum;
	ShowConfig.firstMenuNum = firstMenuNum;
	ShowConfig.lastMenuNum = lastMenuNum;
	ShowConfig.MenuMoveBackground = MenuMoveBackground;
	ShowConfig.CurrentShowArea = BackgroundMenuArea11;

	ituWidgetSetY((ITUWidget*)ShowConfig.MenuMoveBackground, 40);
	//ShowConfig.MenuMoveBackground->icon.widget.rect.y = 40;
	for (int i = ShowConfig.currentFocus; i <= ShowConfig.lastMenuNum; i++){
		textOfBG2Black(NumToText(i));
	}
	textOfBG2Yellow(NumToText(ShowConfig.currentFocus));
}
void updateShowConfig(int firstMenuNum, int lastMenuNum, ITUBackground *MenuMoveBackground){
	ShowConfig.currentFocus = firstMenuNum;
	ShowConfig.firstMenuNum = firstMenuNum;
	ShowConfig.lastMenuNum = lastMenuNum;
	ShowConfig.MenuMoveBackground = MenuMoveBackground;
	//ShowConfig.MenuMoveBackground->icon.widget.rect.y = 40;//移动的距离
	ituWidgetSetY((ITUWidget*)ShowConfig.MenuMoveBackground, 40);
	for (int i = ShowConfig.firstMenuNum; i <= ShowConfig.lastMenuNum; i++){//初始化所以字体为黑字
		textOfBG2Black(NumToText(i));
	}
	textOfBG2Yellow(NumToText(ShowConfig.currentFocus));//焦点字体为黄字
	updateLogo();
}

static void initText(){
	initShowConfig(11, 15, BackgroundMenuMove11);
	theCache.temp = NONE;
	theCache.promo = -1;
	initLogo();
	bgSiblingToHide(BackgroundMenuArea11);
	ituWidgetSetVisible(BackgroundMenuArea11, true);
	//BackgroundMenuArea11->icon.widget.visible = true;
}

typedef struct MainBlink{//闪烁效果
	bool isBlink;
	int blinkLast;
	ITUText *blinkText;
	ITUText *lastBlinkText;//将最后一个保存下来，意外退出要设置可见，防止出现文字消失现象
	u8 blinkMutiFlag;
	u8 blinkMuti4Flag;//0.00闪烁的位数标记 //11111设置项，有四种状态：未选中，设置百位，设置十位，设置个位；
	u8 blinkMuti7Flag;//有6种状态 未选中 08 30 A 
	u8 blinkMuti8Flag;//zpp
}blinkForShow;
blinkForShow blinkConfig;
/*
一项设置，只有一个设置量；选中则闪烁，再次选择则取消闪烁；
只负责显示，不负责改变。
*/
static bool updateBlink(ITUText *blinkText)
{
	//util_Timer = 0;
	
	ctrl_timer = BLINKLAST;//立刻开始闪烁
	blinkConfig.isBlink = reversal(blinkConfig.isBlink);
	blinkConfig.lastBlinkText = blinkConfig.blinkText;
	blinkConfig.blinkText = blinkText;
	if (blinkConfig.lastBlinkText != NULL){
		ituWidgetSetVisible(blinkConfig.lastBlinkText, true);
	}
	if (blinkConfig.isBlink){//闪烁结束时，进行底板设置
		editMode =1;
		return false;//第一次按下，开始闪烁
	}
	else{
		editMode =0;
		return true;//第二次按下，结束闪烁
	}
}
/*
一项设置，按位进行设置，用flag区分当前状态，sum为状态总量；
只负责显示，不负责改变。
*/
static bool updateBlinkWithFlag(ITUText *blinkText, u8* flag, u8 sum)
{
	u8 i = ++(*flag);
	printf("i is %d\n",i);
	if (i >= sum){
		*flag = 0;
		blinkConfig.isBlink= reversal(blinkConfig.isBlink);
	printf("flag is %d flag\n",*flag);
	}
	if (i == 1){
		blinkConfig.isBlink = reversal(blinkConfig.isBlink);
	}
	ctrl_timer = BLINKLAST;
	blinkConfig.lastBlinkText = blinkConfig.blinkText;//BackgroundMenuMove11211
	blinkConfig.blinkText = blinkText;
	if (blinkConfig.lastBlinkText != NULL){
		ituWidgetSetVisible(blinkConfig.lastBlinkText, true);
	}
	if (blinkConfig.isBlink){//闪烁结束时，进行底板设置.
		editMode =1;
		return false;
	}
	else{
		editMode =0;
		return true;
	}
}
static void initBlink(){
	blinkConfig.isBlink = false;
	blinkConfig.blinkLast = BLINKLAST;//BLINKLAST 5 0.5s
	blinkConfig.blinkText = NULL;
	blinkConfig.lastBlinkText = NULL;
}
u16 getCurTP_PriceValue(){//不使用
	u16 price = 0;
	u8* point = (u8*)&(pp.HIGH);
	point = point + theCache.temp * sizeof(TimePriceType);
	TimePriceType* pricePoint = (TimePriceType*)point;
	price = getU16(&pricePoint->TP_Price_H);
	return price;
}
bool blinkFlag11111 = false;
bool blink(){
	if (blinkConfig.blinkLast <= ctrl_timer){
		if (blinkConfig.blinkText){
			ituWidgetSetVisible(blinkConfig.blinkText, reversal(blinkConfig.blinkText->widget.visible));
		}
		ctrl_timer = 0;
		return true;
	}
	return false;
}



static char str1134[32] = { 0 }; //设置TextMenuC1134显示文字  
void initHappyHour4(u8 i){//前七位配置一周的颜色，第八位组合1134项的显示
	if (i & 0x01){
		if (UI_hht->DC_DcDay & 0x01){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleM, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleM, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x02){
		if (UI_hht->DC_DcDay & 0x02){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleTu, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleTu, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x04){
		if (UI_hht->DC_DcDay & 0x04){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleW, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleW, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x08){
		if (UI_hht->DC_DcDay & 0x08){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleTh, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleTh, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x10){
		if (UI_hht->DC_DcDay & 0x10){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleF, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleF, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x20){
		if (UI_hht->DC_DcDay & 0x20){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleSa, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleSa, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x40){
		if (UI_hht->DC_DcDay & 0x40){
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleSu, GREEN_FOR_HAPPYDAY);
		}
		else{
			ituWidgetSetColor2((ITUWidget*)BackgroundWeekSeleSu, RED_FOR_HAPPYDAY);
		}
	}
	if (i & 0x80){
		memset(str1134, 0, 32);
		int dayLen = 0;
		int offset = 0;
		if (UI_hht->DC_DcDay & 0x01){
			char day[] = "M ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x02){
			char day[] = "Tu ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x04){
			char day[] = "W ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x08){
			char day[] = "Th ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x10){
			char day[] = "F ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x20){
			char day[] = "Sa ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}if (UI_hht->DC_DcDay & 0x40){
			char day[] = "Su ";
			dayLen = sizeof(day)-1;
			memcpy(str1134 + offset, day, dayLen);
			offset += dayLen;
		}
		ituTextSetString(TextMenuC1134, str1134);
	}
}
void initHappyHour(u8 i){
	if (i & 0x01){

	}
	if (i & 0x02){

	}
	if (i & 0x04){

	}
	if (i & 0x08){
		initHappyHour4(0xff);
	}
}

static SetCycleParamtersType spt = { 0 };
void buildg_UIeventSendandQueryCycleParamters()//14
{
	spt.number = theCache.temp;
	memcpy((void*)((u8*)&spt + 1), UI_tp, sizeof(TimePriceType));

	u8 len = sizeof(SetCycleParamtersType)+1;
	u8 data[128] = { 0 };
	data[0] = UE_SetCycleParamters;
	memcpy(&data[1], (void*)&spt, sizeof(SetCycleParamtersType));

	out_write_buffer_struct_to_board((void*)data, len);
	//outQquery(ADDR200);//紧接着一个查询
}

/*
构造设置折扣时间的消息队列
例如：1 0 9 0 10 0 9 1e 11 1e 32 7f xx xx
9			长度
0x10		UE_SetHappyHourTime
0			第0个
9 1e 11 1e	九点三十到十一点三十
32			折扣强度百分之五十
7f			按位 01111111 周一到周日全部开启
*/
void buildg_UIeventSendandQueryHHT()//16
{
	g_UIevent.UI_EventType = UE_SetHappyHourTime;
	g_UIevent.UI_EventValue = theCache.promo;
	g_UIevent.UI_datalen = 0x06;
	memcpy(g_UIevent.UI_EventData, UI_hht, g_UIevent.UI_datalen);
	out_write_buffer_to_board();//将g_UIevent发送给底板
	//outQquery(ADDR300);//紧接着一个查询
}

/*
设置RTC时间

*/
static SetRtc srtc = { 0 };
void buildg_UIeventSendandQueryRtc(){
	u8 len = sizeof(SetRtc)+1;
	u8 data[128] = { 0 };

	memcpy((void*)((u8*)&srtc), &UICtrlShowData.dataCP.RTC_Year, sizeof(SetRtc));

	data[0] = UE_SetRtc;//17
	memcpy(&data[1], (void*)&srtc, sizeof(SetRtc));
	out_write_buffer_struct_to_board((void*)data, len);
}
/*
设置机器运行参数
将UICtrlShowData.dataCP中相应数据，打包发送
更改UICtrlShowData.dataCP后，调用buildg_UIeventSendandQueryMachineRunParameters()即可
*/
static SetMachineRunParameters smrp = { 0 };
void buildg_UIeventSendandQueryMachineRunParameters(){
	u8 len = sizeof(SetMachineRunParameters)+1;
	u8 data[128] = { 0 };

	memcpy((void*)((u8*)&smrp), &UICtrlShowData.dataCP.COIN1_VALUE_H, sizeof(SetMachineRunParameters));

	data[0] = UE_SetMachineRunParameters;//18
	memcpy(&data[1], (void*)&smrp, sizeof(SetMachineRunParameters));
	out_write_buffer_struct_to_board((void*)data, len);
}
/*
*************************************************************************************1.1
*/
void setData11111(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_Price_H));
	/*if (num == 0){
	ituTextSetString(TextMenuC111111, "F");
	ituTextSetString(TextMenuC111112, "R");
	ituTextSetString(TextMenuC111113, "E");
	ituTextSetString(TextMenuC111114, "E");
	return;
	}*/
	u16 price1 = num / 100;
	u16 price2 = num % 100 / 10;
	u16 price3 = num % 10;
	textSetU16Number(TextMenuC111111, price1);
	//ituTextSetString(TextMenuC111112, ".");
	textSetU16Number(TextMenuC111113, price2);
	textSetU16Number(TextMenuC111114, price3);
}
void setData11112(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_Default_H));
	char str[32];
	sprintf(str, "%02d", num);
	ituTextSetString(TextMenuC11112, str);
}
void setData11113(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_CoinAdd_H));
	char str[32];
	sprintf(str, "%02d", num);
	ituTextSetString(TextMenuC11113, str);
}
void setData11114(){
	if (!UI_tp){
		return;
	}
	if (UI_tp->TP_Available == 0){
		ituTextSetString(TextMenuC11114, "DISABLE");
	}
	else if (UI_tp->TP_Available == 1){
		ituTextSetString(TextMenuC11114, "ENABLE");
	}
	else{
		ituTextSetString(TextMenuC11114, "ERROR");
	}
}
void setData11115(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_Free_H));
	char str[32];
	sprintf(str, "%02d", num);
	ituTextSetString(TextMenuC11115, str);
}
void setData11116(){
	if (!UI_tp){
		return;
	}

	u16 num = getU16(&(UI_tp->TP_Maximum_H));
	char str[32];
	printf("UI_tp->TP_Maximum_H:%d",UI_tp->TP_Maximum_H);
	printf("UI_tp->TP_Maximum_L:%d",UI_tp->TP_Maximum_L);
	
	sprintf(str, "%02d", num);
	ituTextSetString(TextMenuC11116, str);
}


void setData11211(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_Temperature_H));
	char str[32];
	sprintf(str, "%d", num);
	ituTextSetString(TextMenuC11211, str);
}
void setData11212(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_TempLimit_H));
	char str[32];
	sprintf(str, "%d", num);
	ituTextSetString(TextMenuC11212, str);
}
void setData11213(){
	if (!UI_tp){
		return;
	}
	u8 num = getU8(&(UI_tp->TP_CoolingTime));
	char str[32];
	sprintf(str, "%d", num);
	ituTextSetString(TextMenuC11213, str);
}
void setData11214(){
	if (!UI_tp){
		return;
	}
	u16 num = getU16(&(UI_tp->TP_CoolingTemp_H));
	char str[32];
	sprintf(str, "%d", num);
	ituTextSetString(TextMenuC11214, str);
}

static void setData111(){
	setData11111();
	setData11112();
	setData11113();
	setData11114();
	setData11115();
	setData11116();
}
static void setData112(){
	setData11211();
	setData11212();
	setData11213();
	setData11214();
}
static void setData200OnTimer(){
	setData111();
	setData112();
}
void setData1131(){
	ituWidgetSetVisible(TextMenuC11316, false);
	if (!UI_hht){
		return;
	}
	textSetIntNumber(TextMenuC11311, UI_hht->DC_StartHour / 10);
	textSetIntNumber(TextMenuC11312, UI_hht->DC_StartHour % 10);
	textSetIntNumber(TextMenuC11314, UI_hht->DC_StartMinute / 10);
	textSetIntNumber(TextMenuC11315, UI_hht->DC_StartMinute % 10);

}
void setData1132(){
	ituWidgetSetVisible(TextMenuC11326, false);
	if (!UI_hht){
		return;
	}
	textSetIntNumber(TextMenuC11321, UI_hht->DC_EndHour / 10);
	textSetIntNumber(TextMenuC11322, UI_hht->DC_EndHour % 10);
	textSetIntNumber(TextMenuC11324, UI_hht->DC_EndMinute / 10);
	textSetIntNumber(TextMenuC11325, UI_hht->DC_EndMinute % 10);
}
void setData1133(){
	if (!UI_hht){
		return;
	}
	u8 num = getU8(&(UI_hht->DC_AddTime));
	char str[32];
	sprintf(str, "%02d", num);
	ituTextSetString(TextMenuC1133, str);
}
void setData1134(){
	if (!UI_hht){
		return;
	}
	u8 num = getU8(&(UI_hht->DC_DcDay));
	initHappyHour4(0xff);
}

static void setData113(){
	setData1131();
	setData1132();
	setData1133();
	setData1134();
}

static void setData300OnTimer(){
	setData113();
}

static void setData114(){
	textSetU16WithType(TextMenuC114, UICtrlShowData.data114, '2');
}
/*
*************************************************************************************1.1end
*/
/*
*************************************************************************************1.2
*/
static void setData121(){
	u32 TOTAL_COLLECTIONS = getU32((u8*)&UICtrlShowData.dataIP.count.allCoinSum);
	u32 TRIP_COLLECTIONS = getU32((u8*)&UICtrlShowData.dataIP.count.tripCoinSum);
	char str1[32] = { 0 };
	char str2[32] = { 0 };
	sprintf(str1, "%d.%02d", TOTAL_COLLECTIONS / 100, TOTAL_COLLECTIONS % 100);
	sprintf(str2, "%d.%02d", TRIP_COLLECTIONS / 100, TRIP_COLLECTIONS % 100);

	ituTextSetString(TextMenuC1211, str1);
	ituTextSetString(TextMenuC1212, str2);
}
static void setData122(){
	u32 high = getU32((u8*)&UICtrlShowData.dataIP.count.highTime);
	u32 medium = getU32((u8*)&UICtrlShowData.dataIP.count.mediumTime);
	u32 low = getU32((u8*)&UICtrlShowData.dataIP.count.lowTime);
	u32 noheat = getU32((u8*)&UICtrlShowData.dataIP.count.noheatTime);
	u32 total = getU32((u8*)&UICtrlShowData.dataIP.count.allSumTime);
	u32 trip = getU32((u8*)&UICtrlShowData.dataIP.count.tripSumTime);
	char str1[32] = { 0 };
	char str2[32] = { 0 };
	char str3[32] = { 0 };
	char str4[32] = { 0 };
	char str5[32] = { 0 };
	char str6[32] = { 0 };
	sprintf(str1, "%d", high);
	sprintf(str2, "%d", medium);
	sprintf(str3, "%d", low);
	sprintf(str4, "%d", noheat);
	sprintf(str5, "%d", total);
	sprintf(str6, "%d", trip);

	ituTextSetString(TextMenuC1221, str1);
	ituTextSetString(TextMenuC1222, str2);
	ituTextSetString(TextMenuC1223, str3);
	ituTextSetString(TextMenuC1224, str4);
	ituTextSetString(TextMenuC1225, str5);
	ituTextSetString(TextMenuC1226, str6);
}
void setData43COnTimer(){
	setData121();
	setData122();
}

static void setData123util(ITUText* text, u8 type, u8 offset){
	char str1[64] = { 0 };
	WPEVENT_INFO_PARAMRTER_TYPE_INFO* demo = &UICtrlShowData.dataIP.info1;
	demo += ((offset - 1) + (5 * (type - 1)));
	
	if (1/*demo->isUsed == 1*/){
		if (type == 1){
			sprintf(str1, "<20%02d.%02d.%02d-%02d:%02d>", demo->year, demo->month, demo->day, demo->hour, demo->minute);
		}
		else if (type == 2){
			sprintf(str1, "<20%02d.%02d.%02d-%02d:%02d>", demo->year, demo->month, demo->day, demo->hour, demo->minute);
		}
	}
	else{
		if (type == 1){
			sprintf(str1, "<0000.00.00-00:00>");
		}
		else if (type == 2){
			sprintf(str1, "<0000.00.00-00:00>");
		}
	}
	ituTextSetString(text, str1);
}
/*
type1 info  type2 warning
*/
static void setData400OnTimer(){
	setData123util(TextMenuC1231, 1, 1);
	setData123util(TextMenuC1232, 1, 2);
	setData123util(TextMenuC1233, 1, 3);
	setData123util(TextMenuC1234, 1, 4);
	setData123util(TextMenuC1235, 1, 5);

	setData123util(TextMenuC1241, 2, 1);
	setData123util(TextMenuC1242, 2, 2);
	setData123util(TextMenuC1243, 2, 3);
	setData123util(TextMenuC1244, 2, 4);
	setData123util(TextMenuC1245, 2, 5);
}
/*
*************************************************************************************1.2end
*/
/*
*************************************************************************************1.3
*/
//static u8 setflag1331=0;
//static u8 setflag1334=0;




void setData1331(){
	printf("set data 1331\n");
	//if(setflag1331==0)
	if(editMode)
			return;
		{
			//setflag1331 =1;
			dateVlue[0] = dateVlueH1;
			dateVlue[1] = dateVlueH2;
			dateVlue[2] = UICtrlShowData.dataCP.RTC_Year/10;
			dateVlue[3] = UICtrlShowData.dataCP.RTC_Year%10;
			dateVlue[4] = UICtrlShowData.dataCP.RTC_Mouth/10;
			dateVlue[5] = UICtrlShowData.dataCP.RTC_Mouth%10;
			dateVlue[6] = UICtrlShowData.dataCP.RTC_Day/10;
			dateVlue[7] = UICtrlShowData.dataCP.RTC_Day%10;
		      setData133101();
		}

	//char string[32] = { 0 };
	//sprintf(string, "20%02d-%02d-%02d", UICtrlShowData.dataCP.RTC_Year, UICtrlShowData.dataCP.RTC_Mouth, UICtrlShowData.dataCP.RTC_Day);
	//ituTextSetString(TextMenuC1331, string);
}//zpp

void setData1332(){
	setData133401();
	/*if (UICtrlShowData.dataCP.TIME_ShowType== 0){
				ituTextSetString(TextMenuC1332, "24H");
			}
			else if (UICtrlShowData.dataCP.TIME_ShowType == 1){
				ituTextSetString(TextMenuC1332, "12H");}*/
	//TIME_ShowType1224
	if (UICtrlShowData.dataCP.TIME_ShowType== 0){
				ituTextSetString(TextMenuC1332, "24H");
			}
			else if (UICtrlShowData.dataCP.TIME_ShowType == 1){
				ituTextSetString(TextMenuC1332, "12H");}

}//zpp


//zpp
void setData1333(){

	int hour = 0;
	char str[32] = { 0 };
	
	switch (UICtrlShowData.data1333)
	{
	case -12://西部的 WESTERN
		hour = -12;
		strcpy(str, "WESTERN (UTC-12)");
		break;
	case -11://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-11)");
		break;
	case -10://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-10)");
		break;
	case -9://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-9)");
		break;
	case -8://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-8)");
		break;
	case -7://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-7)");
		break;
	case -6://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-6)");
		break;
	case -5://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-5)");
		break;
	case -4://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-4)");
		break;
	case -3://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-3)");
		break;
	case -2://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-2)");
		break;
	case -1://西部的 Western
		hour = UICtrlShowData.data1333;
		strcpy(str, "WESTERN (UTC-1)");
		break;
	case 0:
		hour = 0;
		strcpy(str,"EASTERN (UTC)");
		break;
	case 1:
		hour = 1;
		strcpy(str, "EASTERN (UTC+1)");
		break;
	case 2 :
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+2)");
		break;
	case 3:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+3)");
		break;
	case 4:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+4)");
		break;
	case 5:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+5)");
		break;
	case 6:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+6)");
		break;
	case 7:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+7)");
		break;
	case 8:
		hour = 8;
		strcpy(str, "EASTERN (UTC+8)");
		break;
	case 9:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+9)");
		break;
	case 10:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+10)");
		break;
	case 11:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+11)");
		break;
	case 12:
		hour = UICtrlShowData.data1333;
		strcpy(str, "EASTERN (UTC+12)");
		break;
	default:
		break;
	}
	theConfig.timeZone = UICtrlShowData.data1333;
	ituTextSetString(TextMenuC1333, str);
}
//zpp
void setData1334(){
	printf("set data 1334 editMode:%d\n",editMode);

	if(editMode)
			return;

	//if(setflag1334==0)
		{
			//setflag1331 =1;

			timeVlue[0] = UICtrlShowData.dataCP.RTC_Hour/10;
			timeVlue[1] = UICtrlShowData.dataCP.RTC_Hour%10;
			timeVlue[2] = UICtrlShowData.dataCP.RTC_Minute/10;
			timeVlue[3] = UICtrlShowData.dataCP.RTC_Minute%10;
			timeVlue[4] = UICtrlShowData.dataCP.RTC_Second/10;
			timeVlue[5] = UICtrlShowData.dataCP.RTC_Second%10;
			
			
		      setData133401();
			
			 
			  
		}
}

void setData13341(){
	//char string[32] = { 0 };
	//sprintf(string, "<%02d:%02d:%02d>", UICtrlShowData.dataCP.RTC_Hour, UICtrlShowData.dataCP.RTC_Minute, UICtrlShowData.dataCP.RTC_Second);
	//ituTextSetString(TextMenuC13341, string);
}

void setData1335(){
	textSetYesNo(TextMenuC1335, UICtrlShowData.dataCP.TIME_AutoDST);//zpp

}
void setData133(){
	

	setData1331();
	setData1332();
	setData1333();
	setData1334();//zpp
	setData13341();
	//setData133407();
	setData1335();
	refreshServiceLayer = true;//zpp
}
void setData500OnTimer(){
	setData133();
}

static void setData1311(int focus){
	u16 num = 0;
	ITUText* text1 = TextMenuC13111;
	//ITUText* text2 = TextMenuC11212;
	ITUText* text3 = TextMenuC13113;
	ITUText* text4 = TextMenuC13114;

	switch (focus)
	{
	case 1311:
		findWidget(TextMenuC13111);
		findWidget(TextMenuC13113);
		findWidget(TextMenuC13114);
		num = getU16(&UICtrlShowData.dataCP.COIN1_VALUE_H);
		text1 = TextMenuC13111;
		text3 = TextMenuC13113;
		text4 = TextMenuC13114;
		break;
	case 1312:
		findWidget(TextMenuC13121);
		findWidget(TextMenuC13123);
		findWidget(TextMenuC13124);
		num = getU16(&UICtrlShowData.dataCP.COIN2_VALUE_H);
		text1 = TextMenuC13121;
		text3 = TextMenuC13123;
		text4 = TextMenuC13124;
		break;
	default:
		break;
	}

	u16 price1 = num / 100;
	u16 price2 = num % 100 / 10;
	u16 price3 = num % 10;
	textSetU16Number(text1, price1);
	//ituTextSetString(TextMenuC111112, ".");
	textSetU16Number(text3, price2);
	textSetU16Number(text4, price3);
}
static void setData1313(){
	findWidget(TextMenuC1313);
	if (UICtrlShowData.dataCP.SHOW_DECIMAL){
		ituTextSetString(TextMenuC1313, "YES");
	}
	else{
		ituTextSetString(TextMenuC1313, "NO");
	}
}
static void setData1314(){
	findWidget(TextMenuC1314);
	if (UICtrlShowData.dataCP.AUTO_START_PAID){
		ituTextSetString(TextMenuC1314, "ENABLE");
	}
	else{
		ituTextSetString(TextMenuC1314, "DISABLE");
	}
}
void setData1321(){
	findWidget(TextMenuC1321);
	if (UICtrlShowData.dataCP.SERIAL_PAYMENT_DEVICE){
		ituTextSetString(TextMenuC1321, "CARD");
	}
	else{
		ituTextSetString(TextMenuC1321, "COIN");
	}
}
void setData1322(){
	findWidget(TextMenuC1322);
	textSetU16Number(TextMenuC1322, getU16(&UICtrlShowData.dataCP.MACHINE_ADDRESS_H));
}
void setData1323(){
	findWidget(TextMenuC1323);
	if (0){
		ituTextSetString(TextMenuC1323, "(CONNECTED)");
	}
	else{
		ituTextSetString(TextMenuC1323, "(DISCONNECTED)");
	}
}
void setData1324(){// <auto>(192.168.1.111)(CONNECTED)
	findWidget(TextMenuC1324);
	char textCache[64] = { 0 };
	char textCache2[64] = { 0 };
	char textCache3[64] = { 0 };
	if (1){
		sprintf(textCache, "<auto>");
	}
	if (1){
		sprintf(textCache2, "%s(192.168.1.1)", textCache);
	}
	if (1){
		sprintf(textCache3, "%s(DISCONNECTED)", textCache2);
	}
	ituTextSetString(TextMenuC1324, textCache3);
}
void setData1325(){//FF-FF-FF-FF-FF-FF
	findWidget(TextMenuC1325);
	//if (0){
	//	ituTextSetString(TextMenuC1325, "FA-FB-FC-FD-FE-FF");
	//}
	//else{
		//ituTextSetString(TextMenuC1325, "FF-FF-FF-FF-FF-FF");
		
		getMacStr();
	
		strmac[17]=0;
		ituTextSetString(TextMenuC1325,strmac) ;
		printf("1601mac:%s\n",strmac);
		printf("1602menumac:%d\n",TextMenuC1325);
	//}
}
void setData1326(){
	findWidget(TextMenuC1326);
	if (0){
		ituTextSetString(TextMenuC1326, "(CONNECTED)");
	}
	else{
		ituTextSetString(TextMenuC1326, "(DISCONNECTED)");
	}
}
static void setData131(){
	setData1311(1311);
	setData1311(1312);
	setData1313();
	setData1314();
}
static void setData132(){
	setData1321();
	setData1322();
	setData1323();
	setData1324();
	setData1325();
	setData1326();
}
void setData133101()
{
	u8 i=0;
	for(;i<8;i++)
		{
			printf("dateVlue[%d] : %d\n",i,dateVlue[i]);
			textSetU16Number(TextMenuC133101[i], dateVlue[i]);
		}
	printf("set data 133101\n");
}

void setData133401()
{
	u8 i=0;
	setData133407();
	for(;i<6;i++)
		{
			printf("dateVlue[%d] : %d\n",i,timeVlue[i]);
			textSetU16Number(TextMenuC133401[i], timeVlue[i]);
			
		}
	printf("set data 133401\n");
	
}


void setData133407(){

//TIME_ShowType1224

//UICtrlShowData.dataCP.TIME_ShowType
printf("TextMenuC1332 is %s \n",TextMenuC1332->string);

if(UICtrlShowData.dataCP.TIME_ShowType== 0)//24h
	{
	
	//TextMenuC1332->widget.name
	ituTextSetString(TextMenuC133407, " ");
	ituWidgetSetVisible(TextMenuC133407, false); 
	
	}
else if(UICtrlShowData.dataCP.TIME_ShowType== 1)//if(TextMenuC1332=="12H")
	{
	ituWidgetSetVisible(TextMenuC133407, true);
	
	if((timeVlue[0]*10+timeVlue[1])>=12)
			{
			//timeVlue[0]=timeVlue[0]-1;
			//timeVlue[1]=timeVlue[1]-2;
			timeVlue[0] = (timeVlue[0]*10+timeVlue[1] -12)/10;
			timeVlue[1] = (timeVlue[0]*10+timeVlue[1] -12)%10;
			ituTextSetString(TextMenuC133407, "PM");
			}
	else if((timeVlue[0]*10+timeVlue[1])<12)
			{
			ituTextSetString(TextMenuC133407, "AM");
			}

	
	
	if (TIME_ShowTypeampm== 0){
		
			ituTextSetString(TextMenuC133407, "AM");
		}
	else if (TIME_ShowTypeampm == 1){
			ituTextSetString(TextMenuC133407, "PM");}
		}
	}//当用1332的24h控制时



static void setData13401(){
	textSetU16WithType(TextMenuC13401, UICtrlShowData.data13401, '2');
}
static void setData13402(){
	textSetOnOff(TextMenuC13402,UICtrlShowData.dataCP.BEEPER);
}
static void setData13403(){
	textSetU16WithType(TextMenuC13403, UICtrlShowData.dataCP.BEEPER_END, '2');
}
static void setData13405(){
	textSetU16WithType(TextMenuC13405, getU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H), '2');
}
static void setData13406(){
	textSetU16WithType(TextMenuC13406,UICtrlShowData.dataCP.WRINKLE_TIME, '2');
}

static void setData13407(){
	//textSetU16WithType(TextMenuC13408, UICtrlShowData.dataCP.isShowTemp_IN, '2');
	printf("13407 ---%d\n",UICtrlShowData.dataCP.isShowTemp_OUT);
	textSetYesNo(TextMenuC13407, UICtrlShowData.dataCP.isShowTemp_OUT);
}
static void setData13408(){
	//textSetU16WithType(TextMenuC13408, UICtrlShowData.dataCP.isShowTemp_IN, '2');
      printf("13408---%d\n",UICtrlShowData.dataCP.isShowTemp_IN);

	textSetYesNo(TextMenuC13408, UICtrlShowData.dataCP.isShowTemp_IN);
}
static void setData13409(){
	//textSetU16WithType(TextMenuC13408, UICtrlShowData.dataCP.isShowTemp_IN, '2');
    //  printf("13408---%d\n",UICtrlShowData.dataCP.isShowTemp_IN);

textSetU16WithType(TextMenuC13409, getU16(&UICtrlShowData.dataCP.SCREEN_OUT_TIME_H), '2');
}
extern void setBrightness(unsigned char num);

static void setData13410(){

	textSetU16WithType(TextMenuC13410,UICtrlShowData.dataCP.SCREEN_BRIGHTNESS, '%');
	setBrightness(UICtrlShowData.dataCP.SCREEN_BRIGHTNESS);

}
static void setData13411(){
	if(UICtrlShowData.dataCP.TEMPERATURE_UNIT==0)
		{
			ituTextSetString(TextMenuC13411, "°F");
		}
	else
		{
			ituTextSetString(TextMenuC13411, "°C");
		}
    //textSetU16WithType(TextMenuC13410,UICtrlShowData.dataCP.SCREEN_BRIGHTNESS, '2');
	//setBrightness(UICtrlShowData.dataCP.SCREEN_BRIGHTNESS);

}
static void setData13412(){
	if (UICtrlShowData.dataCP.FIRST_LANG == 0){
		ituTextSetString(TextMenuC13412, "English");
	}
	else if (UICtrlShowData.dataCP.FIRST_LANG == 1){
		ituTextSetString(TextMenuC13412, "Español");
	}
	else if (UICtrlShowData.dataCP.FIRST_LANG == 2){
		ituTextSetString(TextMenuC13412, "Francais");
	}

}
static void setData13413(){
	if (UICtrlShowData.dataCP.SECOND_LANG== 0){
		ituTextSetString(TextMenuC13413, "English");
	}
	else if (UICtrlShowData.dataCP.SECOND_LANG == 1){
		ituTextSetString(TextMenuC13413, "Español");
	}
	else if (UICtrlShowData.dataCP.SECOND_LANG == 2){
		ituTextSetString(TextMenuC13413, "Francais");
	}
}
static void setData13414(){
	textSetYesNo(TextMenuC13414, UICtrlShowData.dataCP.BUTTON_PIN);
	
	/*if (UICtrlShowData.dataCP.BUTTON_PIN== 0){
			ituTextSetString(TextMenuC13414, "No");
		}
		else if (UICtrlShowData.dataCP.BUTTON_PIN == 1){
			ituTextSetString(TextMenuC13414, "Yes");
		}*/
	
}//zpp
static void setData13404(){
	char str[32] = { 0 };
	switch (UICtrlShowData.dataCP.DEFAULT_PROGRAM)
	{
	case 0:
		strcpy(str, "NONE");
		break;
	case 1:
		strcpy(str, "HIGH");
		break;
	case 2:
		strcpy(str, "MEDIUM");
		break;
	case 3:
		strcpy(str, "LOW");
		break;
	case 4:
		strcpy(str, "NO HEAT");
		break;
	default:
		strcpy(str, "NONE");
		printf("%s() %ld ERROR\r\n", __FUNCTION__, __LINE__);
		break;
	}
	ituTextSetString(TextMenuC13404, str);
}
static void setData134(){
	//if(editMode)
		//	return;

	printf("---setData134\n");
	setData13402();
	setData13403();
	setData13404();
	setData13405();
	setData13406();
	setData13407();
	setData13408();//houda add
	setData13409();
	setData13410();
	setData13411();
	setData13412();
	setData13413();
	setData13414();//zpp


}

void setData520OnTimer(){
	setData131();
	setData132();
	setData134();
	//setData114();
}




//void setData14101(){//
//	if (getbit(UICtrlShowData.dataMS.LOAD_STATUS3, 7)){
//		ituTextSetString(TextMenuC14101, "UNLOCKED");
//	}
//	else{
//		ituTextSetString(TextMenuC14101, "LOCKED");
//	}
//}
 void setData14101()//fan
{	
	bool flag;
	printf("14101 LOAD_STATUS1 %x\n",UICtrlShowData.dataMRS.LOAD_STATUS1);
	if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x04) == 0x04)
		{
			flag = true;
		}
	else
		flag = false;
	
	textSetOnOff(TextMenuC14101,flag );
}
void setData14102()
{	
	bool flag;
	printf("14102 LOAD_STATUS1 %x\n",UICtrlShowData.dataMRS.LOAD_STATUS1);
	if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x08) == 0x08)
		{
			flag = true;
		}
	else
		flag = false;
	
	textSetOnOff(TextMenuC14102,flag );
}
void setData14103()
{	
	bool flag;
	printf("14103 LOAD_STATUS1 %x\n",UICtrlShowData.dataMRS.LOAD_STATUS1);
	if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x02) == 0x02)
		{
			flag = true;
		}
	else
		flag = false;
	
	textSetOnOff(TextMenuC14103,flag );
}

void setData14201(){
	

	//setU16(&UICtrlShowData.dataMS.WP_BOOT_H, 0xffff);//test
	u16 num1 = getU16(&UICtrlShowData.dataMRS.IN_NTC1_H);
	u16 num2 = getU16(&UICtrlShowData.dataMRS.OUT_NTC1_H);

	//u16 num3 = UICtrlShowData.dataMRS.SWITCH_STATUS2;
	u16 num3 = getbit(UICtrlShowData.dataMRS.SWITCH_STATUS2, 4);
	u16 num4 = getbit(UICtrlShowData.dataMRS.SWITCH_STATUS2, 3);

	u16 num5 = getbit(UICtrlShowData.dataMRS.SWITCH_STATUS2, 1);
	u16 num6 =  getbit(UICtrlShowData.dataMRS.SWITCH_STATUS1, 4);
       u16 tmp =  0;//getbit(UICtrlShowData.dataMRS.SWITCH_STATUS2, 2);
	printf("14201 num3:%d\n",num3);

	textSetU16Number(TextMenuC14201, num1);
	textSetU16Number(TextMenuC14202, num2);
	if(num3&&num4)
		{
			ituTextSetString(TextMenuC14203, "CCW");
		}
	else if (num3||num4)
		{
			ituTextSetString(TextMenuC14203, "CC");
		}
	else
		{
			ituTextSetString(TextMenuC14203, "NO");
		}
	if(num5)
		{
			ituTextSetString(TextMenuC14204, "OPEN");
		}
	else
		{
			ituTextSetString(TextMenuC14204, "CLOSED");
		}

	
		if(num6)
		{
			ituTextSetString(TextMenuC14205, "OPEN");
		}
	else
		{
			ituTextSetString(TextMenuC14205, "CLOSED");
		}
	tmp =  getbit(UICtrlShowData.dataMRS.SWITCH_STATUS2, 2);
	if(tmp)
		{
			ituTextSetString(TextMenuC14206, "OPEN");
		}
	else
		{
			ituTextSetString(TextMenuC14206, "CLOSED");
		}
	tmp =  getbit(UICtrlShowData.dataMRS.LOAD_STATUS1, 2);
	if(tmp)
		{
			ituTextSetString(TextMenuC14207, "OPEN");
		}
	else
		{
			ituTextSetString(TextMenuC14207, "CLOSED");
		}

	tmp =  getbit(UICtrlShowData.dataMRS.LOAD_STATUS1, 3);
	if(tmp)
		{
			ituTextSetString(TextMenuC14208, "OPEN");
		}
	else
		{
			ituTextSetString(TextMenuC14208, "CLOSED");
		}
	//num = getU16(&UICtrlShowData.dataCP.COIN1_VALUE_H);
	//textSetU16Number(TextMenuC14209, getU16(&UICtrlShowData.dataCP.COIN1_VALUE_H));
	//textSetU16Number(TextMenuC14210, getU16(&UICtrlShowData.dataCP.COIN2_VALUE_H));
	
	//ituTextSetString((TextMenuC14204, "CLOSED");
	
//	textSet04HexNumber(TextMenuC14442, num5);
//	textSet04HexNumber(TextMenuC14444, num6);
}
void setData1442(){
	//setU16(&UICtrlShowData.dataMS.WP_BOOT_H, 0xffff);//test
	u16 num1 = getU16(&UICtrlShowData.dataMS.WP_BOOT_H);
	u16 num2 = getU16(&UICtrlShowData.dataMS.WP_USER_H);

	u16 num3 = getU16(&UICtrlShowData.dataMS.POWER_BOOT_H);
	u16 num4 = getU16(&UICtrlShowData.dataMS.POWER_USER_H);

	u16 num5 = getU16(&UICtrlShowData.dataMS.MOTOR_BOOT_H);
	u16 num6 = getU16(&UICtrlShowData.dataMS.MOTOR_USER_H);

	textSet04HexNumber(TextMenuC14422, num1);
	textSet04HexNumber(TextMenuC14424, num2);
	textSet04HexNumber(TextMenuC14432, num3);
	textSet04HexNumber(TextMenuC14434, num4);
	textSet04HexNumber(TextMenuC14442, num5);
	textSet04HexNumber(TextMenuC14444, num6);
}

//void setData14201(){
//	if ((UICtrlShowData.data14201 & 0x80)){//DoorSwitch
//		ituTextSetString(TextMenuC14201, "YES");
//	}
//	else{
//		ituTextSetString(TextMenuC14201, "NO");
//	}
//	if ((UICtrlShowData.data14201 & 0x40)){//DoorLockStatus
//		ituTextSetString(TextMenuC14202, "YSE");
//	}
//	else{
//		ituTextSetString(TextMenuC14202, "NO");
//	}
//}
void setData600OnTimer(){
	setData1442();//其他版本号
	//setData14203();
	//setData141();
	//setData14101();
	//setData14206();
}
void setData700OnTimer(){
	setData14201();//其他版本号
	//setData14203();
	//setData141();
	setData14101();
	setData14102();
	setData14103();

	//setData14206();
}
//add or decrease
void setting(int aord)
{
	int change = 1;//改
	int realChange = change * aord;
	int rangeMax = 99;
	int rangeMin = 0;
	int i;
	u8 j;
	//char str5[5];
	//double f;
	u8* point1111 = NULL;

	{
		ctrl_timer = 0;
	if (blinkConfig.blinkText != NULL){
		ituWidgetSetVisible(blinkConfig.blinkText, true);
	}
	}//按键后取消一次闪烁
	printf("%s----ShowConfig.currentFocus:%d\n",__FUNCTION__,ShowConfig.currentFocus);
	switch (ShowConfig.currentFocus){
	case 114://int
		rangeMin = 0;
		rangeMax = 100;
		i = UICtrlShowData.data114;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.data114 = (u8)i;
		setData114();
		break;
	case 11111://tp->TP_Price
		i = getU16(&UI_tp->TP_Price_H);
		rangeMax = 999;
		rangeMin = 0;
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 1:
			realChange *= 100;
			break;
		case 2:
			realChange *= 10;
			break;
		case 3:
			break;
		default:
			break;
		}
		//check
		i += realChange;
		if (i > rangeMax || i < rangeMin){//加减或保持数据
			i -= realChange;
		}
		//保存到UI_tp后，setData就可以刷新显示
		setU16(&(UI_tp->TP_Price_H), (u16)i);//保存数据
		setData11111();
		break;
	case 11112://tp->TP_Default
		i = getU16(&UI_tp->TP_Default_H);
		rangeMax = 90;
		rangeMin = 1;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		//保存到UI_tp后，setData就可以刷新显示
		setU16(&(UI_tp->TP_Default_H), (u16)i);//保存数据
		setData11112();
		break;
	case 11113://tp->TP_CoinAdd
		i = getU16(&(UI_tp->TP_CoinAdd_H));
		rangeMax = 90;
		rangeMin = 1;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_CoinAdd_H), (u16)i);//保存数据
		setData11113();
		break;
	case 11114:
		setU8(&(UI_tp->TP_Available), (u8)reversal(getU8(&(UI_tp->TP_Available))));
		setData11114();
		break;
	case 11115://TP_Free
		i = getU16(&(UI_tp->TP_Free_H));
		rangeMax = 90;
		rangeMin = 1;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_Free_H), (u16)i);//保存数据
		setData11115();
		break;
	case 11116://TP_Maximum
	       
		i = getU16(&(UI_tp->TP_Maximum_H));
		rangeMax = 90;
		rangeMin = 1;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_Maximum_H), (u16)i);//保存数据
		setData11116();
		break;
	case 11211://TP_Temperature
		i = getU16(&(UI_tp->TP_Temperature_H));
		j = 7;
		rangeMax = 180;
		rangeMin = 0;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_Temperature_H), (u16)i);//保存数据
		setData11211();
		break;
	case 11212://TP_TempLimit
		i = getU16(&(UI_tp->TP_TempLimit_H));
		j = 8;
		rangeMax = 375;
		rangeMin = 150;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_TempLimit_H), (u16)i);//保存数据
		setData11212();
		break;
	case 11213://TP_CoolingTime
		i = UI_tp->TP_CoolingTime;
		j = 9;
		rangeMax = 9;
		rangeMin = 1;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU8(&(UI_tp->TP_CoolingTime), (u8)i);//保存数据
		setData11213();
		break;
	case 11214://TP_CoolingTemp
		i = getU16(&(UI_tp->TP_CoolingTemp_H));
		j = 10;
		rangeMax = 125;
		rangeMin = 0;

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&(UI_tp->TP_CoolingTemp_H), (u16)i);//保存数据
		setData11214();
		break;
	case 1131:
		switch (blinkConfig.blinkMutiFlag)
		{
		case 1://小时+10
			rangeMax = 23;
			rangeMin = 0;
			realChange *= 10;
			i = getU8(&(UI_hht->DC_StartHour));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_StartHour), (u8)i);
			setData1131();
			break;
		case 2://小时+1
			rangeMax = 23;
			rangeMin = 0;
			realChange *= 1;
			i = getU8(&(UI_hht->DC_StartHour));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_StartHour), (u8)i);
			setData1131();
			break;
		case 3://分钟+10
			rangeMax = 59;
			rangeMin = 0;
			realChange *= 10;
			i = getU8(&(UI_hht->DC_StartMinute));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_StartMinute), (u8)i);
			setData1131();
			break;
		case 4://分钟+1
			rangeMax = 59;
			rangeMin = 0;
			realChange *= 1;
			i = getU8(&(UI_hht->DC_StartMinute));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_StartMinute), (u8)i);
			setData1131();
			break;
		case 5:

			break;
		default:
			break;
		}
		break;
	case 1132:
		switch (blinkConfig.blinkMutiFlag)
		{
		case 1:
			rangeMax = 23;
			rangeMin = 0;
			realChange *= 10;
			i = getU8(&(UI_hht->DC_EndHour));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_EndHour), (u8)i);
			setData1132();
			break;
		case 2:
			rangeMax = 23;
			rangeMin = 0;
			realChange *= 1;
			i = getU8(&(UI_hht->DC_EndHour));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_EndHour), (u8)i);
			setData1132();
			break;
		case 3:
			rangeMax = 59;
			rangeMin = 0;
			realChange *= 10;
			i = getU8(&(UI_hht->DC_EndMinute));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_EndMinute), (u8)i);
			setData1132();
			break;
		case 4:
			rangeMax = 59;
			rangeMin = 0;
			realChange *= 1;
			i = getU8(&(UI_hht->DC_EndMinute));
			i += realChange;
			if (i > rangeMax || i < rangeMin){
				i -= realChange;
			}
			setU8(&(UI_hht->DC_EndMinute), (u8)i);
			setData1132();
			break;
		case 5:

			break;
		default:
			break;
		}
		break;
	case 1133:
		i = UI_hht->DC_AddTime;
		j = 3;//1 2 3 4
		rangeMax = 99;
		rangeMin = 0;//close

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU8(&(UI_hht->DC_AddTime), (u8)i);
		setData1133();
		break;
	case 1134:

		break;
	case 1213:
		if (UICtrlShowData.data1213 == 0){
			UICtrlShowData.data1213 = 1;
			ituTextSetString(TextMenuC1213, "YES");
		}
		else{
			UICtrlShowData.data1213 = 0;
			ituTextSetString(TextMenuC1213, "NO");
		}
		break;
	case 1227:
		if (UICtrlShowData.data1227 == 0){
			UICtrlShowData.data1227 = 1;
			ituTextSetString(TextMenuC1227, "YES");
		}
		else{
			UICtrlShowData.data1227 = 0;
			ituTextSetString(TextMenuC1227, "NO");
		}
		break;
	case 1311://COIN VALUE 1:
		point1111 = &UICtrlShowData.dataCP.COIN1_VALUE_H;
		i = getU16(point1111);
		rangeMax = 999;
		rangeMin = 0;
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 1:
			realChange *= 100;
			break;
		case 2:
			realChange *= 10;
			break;
		case 3:
			break;
		default:
			break;
		}
		//check
		i += realChange;
		if (i > rangeMax || i < rangeMin){//加减或保持数据
			i -= realChange;
		}
		setU16(point1111, (u16)i);//保存数据
		setData1311(ShowConfig.currentFocus);
		break;
	case 1312://COIN VALUE 2
		point1111 = &UICtrlShowData.dataCP.COIN2_VALUE_H;
		i = getU16(point1111);
		rangeMax = 999;
		rangeMin = 0;
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 1:
			realChange *= 100;
			break;
		case 2:
			realChange *= 10;
			break;
		case 3:
			break;
		default:
			break;
		}
		//check
		i += realChange;
		if (i > rangeMax || i < rangeMin){//加减或保持数据
			i -= realChange;
		}
		setU16(point1111, (u16)i);//保存数据
		setData1311(ShowConfig.currentFocus);
		break;
	case 1313://SHOW DECIMAL:
		UICtrlShowData.dataCP.SHOW_DECIMAL = reversal(UICtrlShowData.dataCP.SHOW_DECIMAL);
		setData1313();
		break;
	case 1314://AUTO START PAID:
		UICtrlShowData.dataCP.AUTO_START_PAID = reversal(UICtrlShowData.dataCP.AUTO_START_PAID);
		setData1314();
		break;

	case 1321:
		UICtrlShowData.dataCP.SERIAL_PAYMENT_DEVICE = reversal(UICtrlShowData.dataCP.SERIAL_PAYMENT_DEVICE);
		setData1321();
		break;
	case 1322:
		point1111 = &UICtrlShowData.dataCP.MACHINE_ADDRESS_H;
		i = getU16(point1111);
		rangeMax = 999;
		rangeMin = 0;
		i += realChange;
		if (i > rangeMax || i < rangeMin){//加减或保持数据
			i -= realChange;
		}
		setU16(point1111, (u16)i);//保存数据
		setData1322();
		break;

	case 1331:
		//set time 	
			{
		    if(blinkConfig.blinkMuti8Flag==0)
		    	{
		    		printf("blinkConfig.blinkMuti8Flag == 0\n");
		    		blinkConfig.blinkMuti8Flag =1;
		    	}	
		     i = dateVlue[blinkConfig.blinkMuti8Flag-1];
		     switch(blinkConfig.blinkMuti8Flag-1)
		     	{
		     	case 0:
					rangeMax = 9;
					rangeMin = 2;
					break;
				case 1:
					rangeMax = 9;
					rangeMin = 0;
					break;
				case 2:
					rangeMax = 9;
					rangeMin = 0;
					break;
				case 3:
					rangeMax = 9;
					rangeMin = 0;
					break;
				case 4:
					rangeMax = 1;
					rangeMin = 0;
					break;
				case 5:
					if(dateVlue[4] ==0)
						{
							rangeMax = 9;
							rangeMin = 1;
						}
					else
						{
							rangeMax = 2;
							rangeMin = 0;	
						}
					break;
				case 6:
						rangeMax = 3;
						rangeMin = 0;	
					break;
				case 7:
					if(dateVlue[6] !=3)
						{
							rangeMax = 9;
							rangeMin = 0;
						}
					else
						{
							rangeMax = 1;
							rangeMin = 0;	
							
						}
					break;
				default:
					rangeMax = 9;
					rangeMin = 0;
					break;
					
		     	}
			 printf("rangeMax:%d  rangeMin:%d \n",rangeMax,rangeMin);

			 i += realChange;
			  printf("i: %d   rangeMax:%d  rangeMin:%d \n",i,rangeMax,rangeMin);

			if (i > rangeMax || i < rangeMin){//鍔犲噺鎴栦繚鎸佹暟鎹?
				i -= realChange;
			}

			printf("133101 realChange :%d\n i: %d\n blinkConfig.blinkMuti8Flag : %d \n",realChange,i,blinkConfig.blinkMuti8Flag);

			if((blinkConfig.blinkMuti8Flag-1)==0)
				{
					dateVlue[0]= i;
				}
			else if((blinkConfig.blinkMuti8Flag-1)==1)
				{
					dateVlue[1]= i;
				}
			else
				{
					dateVlue[blinkConfig.blinkMuti8Flag-1] =i;
				}
			if(dateVlue[6]==3)
				{
				if(dateVlue[7]>1)
					{
						dateVlue[7] =0;
					}
				}
			//textSetU16Number(TextMenuC133101[blinkConfig.blinkMuti8Flag], i);
			setData133101();
			printf("133101 dateVlue[blinkConfig.blinkMuti8Flag :%d\n",dateVlue[blinkConfig.blinkMuti8Flag-1]);
			}
		break;

	case 1332:
		//i = UICtrlShowData.dataCP.BUTTON_PIN;
		//TIME_ShowType1224
		//UICtrlShowData.dataCP.TIME_ShowType
		{
		i = UICtrlShowData.dataCP.TIME_ShowType;//zpp 

		if(i==0)
			{
				i =1;
			}
		else
			{
				i=0;
			}
		UICtrlShowData.dataCP.TIME_ShowType= i;
		
		setData1332();
		
			}
		break;//zpp
		
	case 1333://zpp
		i = UICtrlShowData.data1333;//本地暂存的数据
		rangeMax = 12;//上下限
		rangeMin = -12;//

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.data1333 = i;//将处理完的数据，保存到本地
		setData1333();//1333显示的改变
		break;//zpp
	case 1334://zpp
		{
		    if(blinkConfig.blinkMuti7Flag==0)
		    	{
		    		printf("blinkConfig.blinkMuti7Flag == 0\n");
		    		blinkConfig.blinkMuti7Flag =1;
		    	}	
		     i = timeVlue[blinkConfig.blinkMuti7Flag-1];
		     switch(blinkConfig.blinkMuti7Flag-1)
		     	{
		     	case 0:
					{
						if(UICtrlShowData.dataCP.TIME_ShowType==0)
							{
								rangeMax = 2;
								rangeMin = 0;
							}
						else
							{
								rangeMax = 1;
								rangeMin = 0;	
							}
		     			}
					break;
				case 1:
					if(UICtrlShowData.dataCP.TIME_ShowType==0)
							{
								if(timeVlue[0]==2)
									{
										rangeMax = 3;
										rangeMin = 0;
									}
								else
									{
										rangeMax = 9;
										rangeMin = 0;
									}
							}
					else
						       {
						       if(timeVlue[0]==1)
						       	{
						       		if(timeVlue[1]>2)
						       			{
						       				timeVlue[1] =0;
						       			}
							       	rangeMax = 2;
									rangeMin = 0;
						       	}
							else
								{
									rangeMax = 9;
									rangeMin = 0;
								}
							}
					break;
				case 2:
					rangeMax = 5;
					rangeMin = 0;
					break;
				case 3:
					rangeMax = 9;
					rangeMin = 0;
					break;
				case 4:
					rangeMax = 5;
					rangeMin = 0;
					break;
				case 5:
						{
							rangeMax = 9;
							rangeMin = 0;
						}
					
					break;
					
				case 6:
						{
						
						i = TIME_ShowTypeampm;//zpp 

						if(i==0)
						{
							i =1;
						}
						else
						{
							i=0;
						}
						TIME_ShowTypeampm= i;
		
						setData133407();
						}
					break;
					
				default:
				
					rangeMax = 9;
					rangeMin = 0;
					break;
				
					
		     	}
			 printf("rangeMax:%d  rangeMin:%d \n",rangeMax,rangeMin);

			if(blinkConfig.blinkMuti7Flag<7) 
			{
					i += realChange;
				  printf("i: %d   rangeMax:%d  rangeMin:%d \n",i,rangeMax,rangeMin);

				if (i > rangeMax || i < rangeMin){//鍔犲噺鎴栦繚鎸佹暟鎹?
					i -= realChange;
				}

				printf("133401 realChange :%d\n i: %d\n blinkConfig.blinkMuti7Flag : %d \n",realChange,i,blinkConfig.blinkMuti7Flag);

				
					{
						timeVlue[blinkConfig.blinkMuti7Flag-1] =i;
					}
				//textSetU16Number(TextMenuC133101[blinkConfig.blinkMuti8Flag], i);
				setData133401();
			}
			
			printf("133101 dateVlue[blinkConfig.blinkMuti8Flag :%d\n",timeVlue[blinkConfig.blinkMuti7Flag-1]);
			}
		break;//zpp
		
	case 1335:
		//i = UICtrlShowData.dataCP.BUTTON_PIN;
		{
		i = UICtrlShowData.dataCP.TIME_AutoDST;//zpp 

		if(i==0)
			{
				i =1;
			}
		else
			{
				i=0;
			}
		UICtrlShowData.dataCP.TIME_AutoDST= i;
		
		setData1335();
			}
		break;//zpp
		
	case 133407:
		//i = UICtrlShowData.dataCP.BUTTON_PIN;
		{
		i = TIME_ShowTypeampm;//zpp 

		if(i==0)
			{
				i =1;
			}
		else
			{
				i=0;
			}
		TIME_ShowTypeampm= i;
		
		setData133407();
			}
		break;//zpp

	case 13401:
		i = UICtrlShowData.data13401;
		rangeMax = 99;
		rangeMin = 1;//close

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.data13401 = i;
		setData13401();
		break;
	case 13402:
		i = UICtrlShowData.dataCP.BEEPER;
		if (i == 0){
			i = 1;
		}
		else{
			i = 0;
		}
		UICtrlShowData.dataCP.BEEPER = i;
		printf("---setData13402 line 1905\n");
		setData13402();
		break;
	case 13403:
		i = UICtrlShowData.dataCP.BEEPER_END;
		rangeMax = 30;
		rangeMin = 0;//close

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.dataCP.BEEPER_END = i;
		setData13403();
		break;
	case 13404:
		i = UICtrlShowData.dataCP.DEFAULT_PROGRAM;
		rangeMax = 4;
		rangeMin = 0;//close

		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.dataCP.DEFAULT_PROGRAM = i;
		setData13404();
		break;
	case 13405:
		//UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H;
		i = getU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H);
		printf("13405 value:%d\n",i);
		rangeMax = 30;
		rangeMin = 0;//close
		//realChange =5;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H , (u16 )i);
		setData13405();
		break;
	case 13406:
		i = UICtrlShowData.dataCP.WRINKLE_TIME;
		printf("13406 value:%d\n",i);
		rangeMax = 30;
		rangeMin = 0;//close
		//realChange =5;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		//setU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H , (u16 )i);
		UICtrlShowData.dataCP.WRINKLE_TIME = i;
		setData13406();
	  break;
	case 13407:
		if(UICtrlShowData.dataCP.isShowTemp_OUT==0)
			UICtrlShowData.dataCP.isShowTemp_OUT = 1;
		else
			UICtrlShowData.dataCP.isShowTemp_OUT =  0;
			printf("11UICtrlShowData.dataCP.out : %d\n",UICtrlShowData.dataCP.isShowTemp_OUT);

		setData13407();
		break;
	case 13408:
		i= UICtrlShowData.dataCP.isShowTemp_IN;

		if(i==0)
			{
				i =1;
			}
		else
			{
				i=0;
			}
		UICtrlShowData.dataCP.isShowTemp_IN= i;
		printf("UICtrlShowData.dataCP.isShowTemp_IN : %d\n",UICtrlShowData.dataCP.isShowTemp_IN );
		setData13408();
		break;
	case 13409:
		{
		i = getU16(&UICtrlShowData.dataCP.SCREEN_OUT_TIME_H);
		printf("13409 value:%d\n",i);
		rangeMax = 0xFFFF;
		rangeMin = 0;//close
		realChange =5;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		setU16(&UICtrlShowData.dataCP.SCREEN_OUT_TIME_H , (u16 )i);
		setData13409();
		}
	break;
	case 13410:
		i = UICtrlShowData.dataCP.SCREEN_BRIGHTNESS;
		printf("13410 value:%d\n",i);
		printf("13410 realChange:%d\n",realChange);

		rangeMax = 100;
		rangeMin = 20;//close
		realChange *= 10;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		UICtrlShowData.dataCP.SCREEN_BRIGHTNESS = i;
		setData13410();
	break;
	case 13411:
		{	
		i = UICtrlShowData.dataCP.TEMPERATURE_UNIT;
		if(i==0)
			i=1;
		else
			i=0;
		
		UICtrlShowData.dataCP.TEMPERATURE_UNIT= i;
		setData13411();
		}
		break;	
	case 13412:
		{	
		i = UICtrlShowData.dataCP.FIRST_LANG;
		printf("13412 value:%d\n",i);
		printf("13412 realChange:%d\n",realChange);

		rangeMax = 2;
		rangeMin = 0;//close
		//realChange =5;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		//setU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H , (u16 )i);
		UICtrlShowData.dataCP.FIRST_LANG= i;
		setData13412();
		}
	break;
	case 13413:
		{	
		i = UICtrlShowData.dataCP.SECOND_LANG;
		printf("13413 value:%d\n",i);
		rangeMax = 2;
		rangeMin = 0;//close
		//realChange =5;
		i += realChange;
		if (i > rangeMax || i < rangeMin){
			i -= realChange;
		}
		//setU16(&UICtrlShowData.dataCP.DEFAULT_PROGRAM_TIMEOUT_H , (u16 )i);
		UICtrlShowData.dataCP.SECOND_LANG= i;
		setData13413();
		}
	break;
	case 13414:
		//i = UICtrlShowData.dataCP.BUTTON_PIN;
		{
		i = UICtrlShowData.dataCP.BUTTON_PIN;//zpp begin

		if(i==0)
			{
				i =1;
			}
		else
			{
				i=0;
			}
		UICtrlShowData.dataCP.BUTTON_PIN= i;
		
		setData13414();
			}
		break;//end zpp 
	case 13415:
	case 13416:
		break;
	case 14101:
		{
			if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x04)==0x04)
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1&= 0xFB;
				}
			else
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1|=0x04;
				}
			setData14101();
		}
		break;
	case 14102:
		{
			if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x08)==0x08)
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1&= 0xF7;
				}
			else
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1|=0x08;
				}
			setData14102();
		}
		break;
	case 14103:
		{
			if((UICtrlShowData.dataMRS.LOAD_STATUS1&0x02)==0x02)
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1&= 0xFD;
					UICtrlShowData.dataMRS.LOAD_STATUS1|= 0x01;
				}
			else
				{
					UICtrlShowData.dataMRS.LOAD_STATUS1&= 0xFE;
					UICtrlShowData.dataMRS.LOAD_STATUS1|= 0x02;
				}
			setData14103();
		}
		break;
	default:
		break;
	}
}
void refreshBorder(){//改变黑色边框的坐标和大小，来达到选择的感觉效果。
	switch (flag1134)
	{
	case 0:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 38, 38, 44, 44);
		break;
	case 1:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 88, 38, 44, 44);
		break;
	case 2:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 138, 38, 44, 44);
		break;
	case 3:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 188, 38, 44, 44);
		break;
	case 4:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 238, 38, 44, 44);
		break;
	case 5:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 288, 38, 44, 44);
		break;
	case 6:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 338, 38, 44, 44);
		break;
	case 7:
		ituWidgetSetXYWH((ITUWidget*)BackgroundWeekChangeBorder, 168, 88, 84, 44);
		break;
	default:
		break;
	}
}
void settingHHT(int i){
	flag1134 += i;
	if (flag1134 > 7){
		flag1134 = 0;
	}
	if (flag1134 < 0){
		flag1134 = 7;
	}
	refreshBorder();
}
bool MainButtonUpOnPress(void)
{
	BUTTON_LOG"|A|"LOG_END;//up
	if (blinkConfig.isBlink == true && blinkConfig.blinkText != NULL){//设置数据
		setting(1);
		return true;
	}
	if (ituWidgetIsVisible(BackgroundWeekBorderArea)){
		settingHHT(-1);
		return true;
	}
	if (ShowConfig.MenuMoveBackground == NULL){
		return 0;
	}
	if (ShowConfig.currentFocus > ShowConfig.firstMenuNum){

		ShowConfig.MenuMoveBackground->icon.widget.rect.y += MOVETEXTSIZE;
		textOfBG2Black(NumToText(ShowConfig.currentFocus--));//
		textOfBG2Yellow(NumToText(ShowConfig.currentFocus));//改变字体颜色
		updateLogo();//更新left and right logo
	}
	else{
		return 0;
	}
	return true;
}
bool MainButtonDownOnPress(void)
{
	BUTTON_LOG"|V|"LOG_END;//down
	if (blinkConfig.isBlink == true && blinkConfig.blinkText != NULL){
		setting(-1);
		return true;
	}
	if (ituWidgetIsVisible(BackgroundWeekBorderArea)){
		settingHHT(1);
		return true;
	}
	if (ShowConfig.MenuMoveBackground == NULL){
		return 0;
	}
	if (ShowConfig.currentFocus < ShowConfig.lastMenuNum){
		ShowConfig.MenuMoveBackground->icon.widget.rect.y -= MOVETEXTSIZE;
		textOfBG2Black(NumToText(ShowConfig.currentFocus++));
		textOfBG2Yellow(NumToText(ShowConfig.currentFocus));
		updateLogo();
	}
	else{
		return 0;
	}
	return true;
}







//页面转换 & 选中
int DownLoadApp();
u8 timeTypeAP = 5;
bool MainProcessButtomDown(void)//confirm//页面转换 & 选中
{
	BUTTON_LOG"|>|"LOG_END;//select

	switch (ShowConfig.currentFocus)
	{
	case 11://Main Menu 1.1 -> PROGRAM SETTINGS 1.1.1
		ituWidgetSetVisible(BackgroundMenuArea11, false);
		ituWidgetSetVisible(BackgroundMenuArea111, true);
		updateShowConfig(111, 115, BackgroundMenuMove111);

		setData114(UICtrlShowData.data114);
		break;
	case 12://Main Menu 1.1 -> PROGRAM SETTINGS 1.2.1
		ituWidgetSetVisible(BackgroundMenuArea11, false);
		ituWidgetSetVisible(BackgroundMenuArea121, true);
		updateShowConfig(121, 125, BackgroundMenuMove121);
		break;
	case 13://Main Menu 1.1 -> PROGRAM SETTINGS 1.3.1
		ituWidgetSetVisible(BackgroundMenuArea11, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;
	case 14://Main Menu 1.1 -> PROGRAM SETTINGS 1.4.1
		ituWidgetSetVisible(BackgroundMenuArea11, false);
		ituWidgetSetVisible(BackgroundMenuArea141, true);
		updateShowConfig(141, 145, BackgroundMenuMove141);
		break;
	case 15://Main Menu 1.1 -> PROGRAM SETTINGS 1.1.1
		//ituLayerGoto(startLayer);
		outQsetTypeValue(UE_Cycle, 1);
		break;
	case 111://PROGRAM SETTINGS 1.1.1 -> TIME/PRICING1.1.1.1
		ituWidgetSetVisible(BackgroundMenuArea111, false);
		ituWidgetSetVisible(BackgroundMenuArea1111, true);
		updateShowConfig(1111, 1115, BackgroundMenuMove1111);
		break;
	case 112://PROGRAM SETTINGS 1.1.2 -> TIME/PRICING1.1.2.1
		ituWidgetSetVisible(BackgroundMenuArea111, false);
		ituWidgetSetVisible(BackgroundMenuArea1121, true);
		updateShowConfig(1121, 1125, BackgroundMenuMove1121);
		break;
	case 113://PROGRAM SETTINGS 1.1.3 -> TIME/PRICING1.1.3.1
		ituWidgetSetVisible(BackgroundMenuArea111, false);
		ituWidgetSetVisible(BackgroundMenuArea1131, true);
		theCache.promo = 0;
		UI_hht = &UICtrlShowData.dataDP.dc0;
		refreshServiceLayer = true;
		updateShowConfig(1131, 1136, BackgroundMenuMove1131);

		setData300OnTimer();
		break;
	case 114://blink&change TextMenuC114
		if (updateBlink(TextMenuC114)){
			printf("communication 114\n");
		}
		break;
	case 115://PROGRAM SETTINGS 1.1.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea111, false);
		ituWidgetSetVisible(BackgroundMenuArea11, true);
		updateShowConfig(11, 15, BackgroundMenuMove11);
		break;
	case 1111://TIME/PRICING1.1.1.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = HIGH;
		if (ShowConfig.currentFocus == 1111){ theCache.temp = HIGH; tp = &pp.HIGH; UI_tp = &UICtrlShowData.dataPP.HIGH; }
	case 1112://TIME/PRICING1.1.1.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = MIDIUM;
		if (ShowConfig.currentFocus == 1112){ theCache.temp = MIDIUM; tp = &pp.MIDIUM; UI_tp = &UICtrlShowData.dataPP.MIDIUM; }
	case 1113://TIME/PRICING1.1.1.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = LOW;
		if (ShowConfig.currentFocus == 1113){ theCache.temp = LOW; tp = &pp.LOW; UI_tp = &UICtrlShowData.dataPP.LOW; }
	case 1114://TIME/PRICING1.1.1.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = NOHEAT;
		if (ShowConfig.currentFocus == 1114){ theCache.temp = NOHEAT; tp = &pp.NOHEAT; UI_tp = &UICtrlShowData.dataPP.NOHEAT; }
		ituWidgetSetVisible(BackgroundMenuArea1111, false);
		ituWidgetSetVisible(BackgroundMenuArea11111, true);
		updateShowConfig(11111, 11117, BackgroundMenuMove11111);

		setData11111();
		setData11112();
		setData11113();
		setData11114();
		setData11115();
		setData11116();
		break;
	case 1115://TIME/PRICING1.1.1.1 -> PROGRAM SETTINGS 1.1.1
		ituWidgetSetVisible(BackgroundMenuArea1111, false);
		ituWidgetSetVisible(BackgroundMenuArea111, true);
		updateShowConfig(111, 115, BackgroundMenuMove111);
		break;
	case 11111://blink&change TextMenuC11111
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 0:
			updateBlinkWithFlag(TextMenuC111111, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 1:
			updateBlinkWithFlag(TextMenuC111113, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 2:
			updateBlinkWithFlag(TextMenuC111114, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 3:
			updateBlinkWithFlag(TextMenuC111114, &blinkConfig.blinkMuti4Flag, 4);
			break;
		default:
			break;
		}
		if (blinkConfig.blinkMuti4Flag == 0){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11111\n");
		}
		break;
	case 11112://blink&change TextMenuC11112
		if (updateBlink(TextMenuC11112)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11112\n");
		}
		break;
	case 11113://blink&change TextMenuC11113
		if (updateBlink(TextMenuC11113)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11113\n");
		}
		break;
	case 11114://blink&change TextMenuC11114
		if (updateBlink(TextMenuC11114)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11114\n");
		}
		break;
	case 11115://blink&change TextMenuC11115
		if (updateBlink(TextMenuC11115)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11115\n");
		}
		break;
	case 11116://blink&change TextMenuC11116
		if (updateBlink(TextMenuC11116)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11116\n");
		}
		break;
	case 11117://TIME/PRICING1.1.1.1.1 -> PROGRAM SETTINGS 1.1.1.1
		ituWidgetSetVisible(BackgroundMenuArea11111, false);
		ituWidgetSetVisible(BackgroundMenuArea1111, true);
		theCache.temp = NONE;
		tp = NULL;
		UI_tp = NULL;
		updateShowConfig(1111, 1115, BackgroundMenuMove1111);
		break;
	case 1121://TIME/PRICING1.1.2.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = HIGH;
		if (ShowConfig.currentFocus == 1121){ theCache.temp = HIGH; tp = &pp.HIGH; UI_tp = &UICtrlShowData.dataPP.HIGH; }
	case 1122://TIME/PRICING1.1.2.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = MIDIUM;
		if (ShowConfig.currentFocus == 1122){ theCache.temp = MIDIUM; tp = &pp.MIDIUM; UI_tp = &UICtrlShowData.dataPP.MIDIUM; }
	case 1123://TIME/PRICING1.1.2.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = LOW;
		if (ShowConfig.currentFocus == 1123){ theCache.temp = LOW; tp = &pp.LOW; UI_tp = &UICtrlShowData.dataPP.LOW; }
	case 1124://TIME/PRICING1.1.2.1 -> PROGRAM SETTINGS 1.1.1.1.1 LOGO = NOHEAT;
		if (ShowConfig.currentFocus == 1124){ theCache.temp = NOHEAT; tp = &pp.NOHEAT; UI_tp = &UICtrlShowData.dataPP.NOHEAT; }
		ituWidgetSetVisible(BackgroundMenuArea1121, false);
		ituWidgetSetVisible(BackgroundMenuArea11211, true);
		updateShowConfig(11211, 11215, BackgroundMenuMove11211);

		setData11211();
		setData11212();
		setData11213();
		setData11214();
		break;
	case 1125://TIME/PRICING1.1.2.2\1 -> PROGRAM SETTINGS 1.1.1
		ituWidgetSetVisible(BackgroundMenuArea1121, false);
		ituWidgetSetVisible(BackgroundMenuArea111, true);
		updateShowConfig(111, 115, BackgroundMenuMove111);
		break;
	case 11211://blink&change TextMenuC11211
		if (updateBlink(TextMenuC11211)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11211\n");
		}
		break;
	case 11212://blink&change TextMenuC11212
		if (updateBlink(TextMenuC11212)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11212\n");
		}
		break;
	case 11213://blink&change TextMenuC11213
		if (updateBlink(TextMenuC11213)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11213\n");
		}
		break;
	case 11214://blink&change TextMenuC11214
		if (updateBlink(TextMenuC11214)){
			buildg_UIeventSendandQueryCycleParamters();
			printf("communication 11214\n");
		}
		break;
	case 11215://TIME/PRICING1.1.2.1.1 -> PROGRAM SETTINGS 1.1.2.1
		ituWidgetSetVisible(BackgroundMenuArea11211, false);
		ituWidgetSetVisible(BackgroundMenuArea1121, true);
		theCache.temp = NONE;
		tp = NULL;
		UI_tp = NULL;
		updateShowConfig(1121, 1125, BackgroundMenuMove1121);
		break;
	case 1131:// 08 30 A

		switch (blinkConfig.blinkMutiFlag)
		{
		case 0:
			updateBlinkWithFlag(TextMenuC11311, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 1:
			updateBlinkWithFlag(TextMenuC11312, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 2:
			updateBlinkWithFlag(TextMenuC11314, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 3:
			updateBlinkWithFlag(TextMenuC11315, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 4:
			updateBlinkWithFlag(TextMenuC11315, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 5:
			updateBlinkWithFlag(TextMenuC11316, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		default:
			break;
		}
		if (blinkConfig.blinkMutiFlag == 0){
			buildg_UIeventSendandQueryHHT();
			printf("communication 1131\n");
		}
		break;
	case 1132://12 30 p
		switch (blinkConfig.blinkMutiFlag)
		{
		case 0:
			updateBlinkWithFlag(TextMenuC11321, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 1:
			updateBlinkWithFlag(TextMenuC11322, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 2:
			updateBlinkWithFlag(TextMenuC11324, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 3:
			updateBlinkWithFlag(TextMenuC11325, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 4:
			updateBlinkWithFlag(TextMenuC11325, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		case 5:
			updateBlinkWithFlag(TextMenuC11326, &blinkConfig.blinkMutiFlag, timeTypeAP);
			break;
		default:
			break;
		}
		if (blinkConfig.blinkMutiFlag == 0){
			buildg_UIeventSendandQueryHHT();
			printf("communication 1132\n");
		}
		break;
	case 1133:
		if (updateBlink(TextMenuC1133)){
			buildg_UIeventSendandQueryHHT();
			printf("communication 1133\n");
		}
		break;
	case 1134:
		if (!ituWidgetIsVisible(BackgroundWeekBorderArea)){
			refreshBorder();
			ituWidgetSetVisible(BackgroundWeekBorderArea, true);
			initHappyHour(0xff);
			break;
		}
		if (flag1134 == 7){//最终保存
			initHappyHour(0xff);
			ituWidgetSetVisible(BackgroundWeekBorderArea, false);
			flag1134 = 0;
			buildg_UIeventSendandQueryHHT();
			printf("communication 1134\n");
		}
		else{//依次设置
			if (flag1134 < 0){}
			else if (flag1134>7){}
			else{
				offset8 = 0x01 << flag1134;
				if (UI_hht->DC_DcDay & offset8){
					UI_hht->DC_DcDay -= offset8;
				}
				else{
					UI_hht->DC_DcDay += offset8;
				}
				initHappyHour4(offset8 + 0x80);
			}
		}
		break;
	case 1135://TIME/PRICING1.1.2.2\1 -> PROGRAM SETTINGS 1.1.1 exit
		ituWidgetSetVisible(BackgroundMenuArea1131, false);
		ituWidgetSetVisible(BackgroundMenuArea111, true);
		theCache.promo = NONE;
		UI_hht = NULL;
		updateShowConfig(111, 115, BackgroundMenuMove111);
		break;
	case 1136://TIME/PRICING1.1.2.2\1 -> PROGRAM SETTINGS 1.1.1 next
		if ((int8_t)theCache.promo >= 9){//exit
			ituWidgetSetVisible(BackgroundMenuArea1131, false);
			ituWidgetSetVisible(BackgroundMenuArea111, true);
			theCache.promo = NONE;
			UI_hht = NULL;
			updateShowConfig(111, 115, BackgroundMenuMove111);
		}
		else{//next
			theCache.promo++;
			UI_hht = (&UICtrlShowData.dataDP.dc0) + theCache.promo;
			setData300OnTimer();
			updateShowConfig(1131, 1136, BackgroundMenuMove1131);
		}
		break;
		/*
		********************************************1.1 end*********************************************************
		*/
	case 121:
		ituWidgetSetVisible(BackgroundMenuArea121, false);
		ituWidgetSetVisible(BackgroundMenuArea1211, true);
		setData121();
		updateShowConfig(1211, 1214, BackgroundMenuMove1211);
		break;
	case 122:
		ituWidgetSetVisible(BackgroundMenuArea121, false);
		ituWidgetSetVisible(BackgroundMenuArea1221, true);
		setData122();
		updateShowConfig(1221, 1228, BackgroundMenuMove1221);
		break;
	case 123:
		ituWidgetSetVisible(BackgroundMenuArea121, false);
		ituWidgetSetVisible(BackgroundMenuArea1231, true);
		//setData123();
		updateShowConfig(1231, 1235, BackgroundMenuMove1231);
		break;
		break;
	case 124:
		ituWidgetSetVisible(BackgroundMenuArea121, false);
		ituWidgetSetVisible(BackgroundMenuArea1241, true);
		updateShowConfig(1241, 1245, BackgroundMenuMove1241);
		break;
	case 125://PROGRAM SETTINGS 1.2.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea121, false);
		ituWidgetSetVisible(BackgroundMenuArea11, true);
		updateShowConfig(11, 15, BackgroundMenuMove11);
		break;
	case 1213:
		if (updateBlink(TextMenuC1213)){
			if (UICtrlShowData.data1213 == 1){
				UICtrlShowData.data1213 = 0;
				outQsetTypeValue(UE_ResetParamters, 0);
				ituTextSetString(TextMenuC1213, "NO");

				ituWidgetSetVisible(BackgroundMenuArea1211, false);
				ituWidgetSetVisible(BackgroundMenuArea121, true);
				updateShowConfig(121, 125, BackgroundMenuMove121);
			}
			printf("communication 1213\n");
		}
		break;
	case 1214:
		ituWidgetSetVisible(BackgroundMenuArea1211, false);
		ituWidgetSetVisible(BackgroundMenuArea121, true);
		updateShowConfig(121, 125, BackgroundMenuMove121);
		break;
	case 1227:
		if (updateBlink(TextMenuC1227)){
			if (UICtrlShowData.data1227 == 1){
				UICtrlShowData.data1227 = 0;
				outQsetTypeValue(UE_ResetParamters, 1);
				ituTextSetString(TextMenuC1227, "NO");
				ituWidgetSetVisible(BackgroundMenuArea1221, false);
				ituWidgetSetVisible(BackgroundMenuArea121, true);
				updateShowConfig(121, 125, BackgroundMenuMove121);
			}
			printf("communication 1227\n");
		}
		break;
	case 1228:
		ituWidgetSetVisible(BackgroundMenuArea1221, false);
		ituWidgetSetVisible(BackgroundMenuArea121, true);
		updateShowConfig(121, 125, BackgroundMenuMove121);
		break;
	case 1231:
	case 1232:
	case 1233:
	case 1234:
	case 1235:
		ituWidgetSetVisible(BackgroundMenuArea1231, false);
		ituWidgetSetVisible(BackgroundMenuArea121, true);
		updateShowConfig(121, 125, BackgroundMenuMove121);
	
	case 1241:
	case 1242:
	case 1243:
	case 1244:
	case 1245:
		ituWidgetSetVisible(BackgroundMenuArea1241, false);
		ituWidgetSetVisible(BackgroundMenuArea121, true);
		updateShowConfig(121, 125, BackgroundMenuMove121);
		break;
		/*
		********************************************1.2 end*********************************************************
		*/
	case 131://PROGRAM SETTINGS 1.3.1 -> PAYMENT SETTINGS 1.3.1.1
		ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea1311, true);
		updateShowConfig(1311, 1315, BackgroundMenuMove1311);
		setData131();
		break;
	case 132://PROGRAM SETTINGS 1.3.1 -> CONNECTION SETTINGS 1.3.2.1
		/*ituWidgetSetVisible(BackgroundMenuArea1321, true);
		bgSiblingToHide(BackgroundMenuArea11);*/

		/*ituWidgetSetVisible(BackgroundTextBlack1321, true);
		ituWidgetSetVisible(BackgroundMenu1321, true);
		ituWidgetSetVisible(TextMenu1327, true);
		*/
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea1321, true);
		updateShowConfig(1321, 1327, BackgroundMenuMove1321);
		break;
	case 133://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea1331, true);
		updateShowConfig(1331, 1336, BackgroundMenuMove1331);
		setData133();
		break;
	case 134://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea1341, true);
		updateShowConfig(13401, 13416, BackgroundMenuMove1341);
		break;
	case 135://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea1351, true);
		updateShowConfig(1351, 1353, BackgroundMenuMove1351);
		break;
	case 136://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea131, false);
		ituWidgetSetVisible(BackgroundMenuArea11, true);
		updateShowConfig(11, 15, BackgroundMenuMove11);
		break;
	case 1311:
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 0:
			updateBlinkWithFlag(TextMenuC13111, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 1:
			updateBlinkWithFlag(TextMenuC13113, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 2:
			updateBlinkWithFlag(TextMenuC13114, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 3:
			updateBlinkWithFlag(TextMenuC13114, &blinkConfig.blinkMuti4Flag, 4);
			break;
		default:
			break;
		}
		if (blinkConfig.blinkMuti4Flag == 0){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("communication 1311\n");
		}
		break;
	case 1312:
		switch (blinkConfig.blinkMuti4Flag)
		{
		case 0:
			updateBlinkWithFlag(TextMenuC13121, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 1:
			updateBlinkWithFlag(TextMenuC13123, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 2:
			updateBlinkWithFlag(TextMenuC13124, &blinkConfig.blinkMuti4Flag, 4);
			break;
		case 3:
			updateBlinkWithFlag(TextMenuC13124, &blinkConfig.blinkMuti4Flag, 4);
			break;
		default:
			break;
		}
		if (blinkConfig.blinkMuti4Flag == 0){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("communication 1312\n");
		}
		break;
	case 1313:
		if (updateBlink(TextMenuC1313)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("communication 1313\n");
		}
		break;
	case 1314:
		if (updateBlink(TextMenuC1314)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("communication 1314\n");
		}
		break;

	case 1315://PAYMENT SETTINGS 1.3.1.1 -> PROGRAM SETTINGS 1.3.1
		ituWidgetSetVisible(BackgroundMenuArea1311, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;
	
	case 1321:
		if (updateBlink(TextMenuC1321)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC1321\n");
		}
		break;
	case 1322:
		if (updateBlink(TextMenuC1322)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC1322\n");
		}
		break;
	
	case 1323://1323 ->13231
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuArea1321, false);
		ituWidgetSetVisible(BackgroundMenuArea13231, true);
		updateShowConfig(13231, 13234, BackgroundMenuMove13231);
		break;
	case 13231:
		break;
	case 13232:
		break;
	case 13233:
		break;
	case 13234://13231 -> 1321
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuMove13231, false);
		ituWidgetSetVisible(BackgroundMenuArea1321, true);
		updateShowConfig(1321, 1327, BackgroundMenuMove1321);
		break;
	case 1324:
		break;
	case 1325:
		break;
	case 1326://1326 ->13261
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuArea1321, false);
		ituWidgetSetVisible(BackgroundMenuArea13261, true);
		updateShowConfig(13261, 13263, BackgroundMenuMove13261);
		break;
	case 13261:
		break;
	case 13262:
		break;
	case 13263://13261 -> 1321
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuMove13261, false);
		ituWidgetSetVisible(BackgroundMenuArea1321, true);
		updateShowConfig(1321, 1327, BackgroundMenuMove1321);
		break;
	case 1327://CONNECTION SETTINGS 1.3.2.1 -> PROGRAM SETTINGS 1.3.1
		//ituWidgetSetVisible(BackgroundTextBlack1321, false);
		bgSiblingToHide(BackgroundMenuArea11);
		//ituWidgetSetVisible(BackgroundMenuMove1321, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;



	case 1331://DATE: 2020-04-22 (YYYY-MM-DD)
//		setflag1331 =1; //houda modi
		printf("blinkConfig.blinkMuti8Flag:%d\n",blinkConfig.blinkMuti8Flag);
		{
			if(blinkConfig.blinkMuti8Flag>=8)
				{
				     blinkConfig.blinkMuti8Flag = 8;
					updateBlinkWithFlag(TextMenuC133101[blinkConfig.blinkMuti8Flag-1], &blinkConfig.blinkMuti8Flag, 9);
				}
			else
				
				//updateBlinkWithFlag(TextMenuC133401[blinkConfig.blinkMuti7Flag], &blinkConfig.blinkMuti7Flag, 8);
					updateBlinkWithFlag(TextMenuC133101[blinkConfig.blinkMuti8Flag], &blinkConfig.blinkMuti8Flag, 9);
		}
		if (blinkConfig.blinkMuti8Flag == 0){
			buildg_UIeventSendandQueryRtc();
			printf("TextMenuC1331\n");
		}
		break;//zpp
	case 1332://zpp
		if (updateBlink(TextMenuC1332)){
			{
			buildg_UIeventSendandQueryRtc();
			printf("TextMenuC1332\n");
			}
		}
		break;//zpp
		
	case 1333://zpp
		if (updateBlink(TextMenuC1333)){//使TextMenuC1333开始闪烁和停止闪烁
			//取消闪烁时进入，进行底板设置，或者存储本地
			ConfigSave();
			//sntp_update();
			printf("TextMenuC1333\n");
		}
		break;//zpp
		
	case 1334://zpp
	//	setflag1334 =1;
		if(UICtrlShowData.dataCP.TIME_ShowType== 0)//24h
		
			{
				printf("blinkConfig.blinkMuti7Flag:%d\n",blinkConfig.blinkMuti7Flag);
		
				ituWidgetSetVisible(TextMenuC133407, false);
				{
				
				if(blinkConfig.blinkMuti7Flag>=6)
					{
				   	 	 blinkConfig.blinkMuti7Flag = 6;
						updateBlinkWithFlag(TextMenuC133401[blinkConfig.blinkMuti7Flag-1], &blinkConfig.blinkMuti7Flag, 7);
					}
				else
				
						updateBlinkWithFlag(TextMenuC133401[blinkConfig.blinkMuti7Flag], &blinkConfig.blinkMuti7Flag, 7);}
		
				if (blinkConfig.blinkMuti7Flag == 0){
				
				buildg_UIeventSendandQueryRtc();
				printf("TextMenuC1334\n");}
			}
		else if(UICtrlShowData.dataCP.TIME_ShowType== 1)//12h
			{
			ituWidgetSetVisible(TextMenuC133407, true);
			
			{
			if(blinkConfig.blinkMuti7Flag>=7)
				{
				     blinkConfig.blinkMuti7Flag = 7;
					updateBlinkWithFlag(TextMenuC133401[blinkConfig.blinkMuti7Flag-1], &blinkConfig.blinkMuti7Flag, 8);
				}
			else
				
				updateBlinkWithFlag(TextMenuC133401[blinkConfig.blinkMuti7Flag], &blinkConfig.blinkMuti7Flag, 8);}
		
			if (blinkConfig.blinkMuti7Flag == 0){
			

			buildg_UIeventSendandQueryRtc();
			printf("TextMenuC1334\n");}
			}
		break;	
	case 1335://zpp
		if (updateBlink(TextMenuC1335)){
			{
			buildg_UIeventSendandQueryRtc();
			printf("TextMenuC1335\n");
			}
		}
		break;//zpp
		
	case 1336://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea1331, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;
	//case 133407://zpp
	//	if (updateBlink(TextMenuC133407)){
	//		{
	//		buildg_UIeventSendandQueryRtc();
	//		printf("TextMenuC133407\n");
	//		}
	//	}
	//	break;//zpp
	case 13401:
		if (updateBlink(TextMenuC13401)){
			
			printf("TextMenuC13401\n");
		}
		break;


	case 13402:
		if (updateBlink(TextMenuC13402)){
			{
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13402\n");
			}
		}
		break;
	case 13403:
		if (updateBlink(TextMenuC13403)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13403\n");
		}
		break;
	case 13404:
		if (updateBlink(TextMenuC13404)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13404\n");
		}
		break;
	case 13405:
		if (updateBlink(TextMenuC13405)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13405\n");
		}
		break;
	case 13406:
		if (updateBlink(TextMenuC13406)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13406\n");
		}
		break;
	case 13407:
		if (updateBlink(TextMenuC13407)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13407\n");
		}
		break;
	case 13408:
		if (updateBlink(TextMenuC13408)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13408\n");
		}
		break;
	case 13409:
		if (updateBlink(TextMenuC13409)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13409\n");
		}
		break;
	case 13410:
		if (updateBlink(TextMenuC13410)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13410\n");
		}
		break;
	case 13411:
		if (updateBlink(TextMenuC13411)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13411\n");
		}
		break;
	case 13412:
		if (updateBlink(TextMenuC13412)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13412\n");
		}
		break;
	case 13413:
		if (updateBlink(TextMenuC13413)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13413\n");
		}
		break;
	case 13414:
		if (updateBlink(TextMenuC13414)){
			buildg_UIeventSendandQueryMachineRunParameters();
			printf("TextMenuC13414\n");
		}
		break;
	case 13415:
		if (updateBlink(TextMenuC13415)){
			//if (UICtrlShowData.data13414){//reset
			//	outQsetTypeValue(UE_ResetParamters, 2);
			//	UICtrlShowData.data13414 = 0;
			//	ituTextSetString(blinkConfig.blinkText, "NO");
			//}
			printf("TextMenuC13415\n");
		}
		break;
	case 13416://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea1341, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;
	case 1353://PROGRAM SETTINGS 1.3.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea1351, false);
		ituWidgetSetVisible(BackgroundMenuArea131, true);
		updateShowConfig(131, 136, BackgroundMenuMove131);
		break;
		/*
		********************************************1.3 end*********************************************************
		*/
	case 141:
		ituWidgetSetVisible(BackgroundMenuArea141, false);
		ituWidgetSetVisible(BackgroundMenuArea1411, true);
		updateShowConfig(14101, 14105, BackgroundMenuMove1411);
		break;
	case 142:
		ituWidgetSetVisible(BackgroundMenuArea141, false);
		ituWidgetSetVisible(BackgroundMenuArea1421, true);
		updateShowConfig(14201, 14210, BackgroundMenuMove1421);
		break;
	case 143:

		break;
	case 144://PROGRAM SETTINGS 1.4.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea141, false);
		ituWidgetSetVisible(BackgroundMenuArea1441, true);
		updateShowConfig(1441, 1445, BackgroundMenuMove1441);
		setData1442();
		//DownLoadApp();
		break;
	case 145://PROGRAM SETTINGS 1.4.1 -> Main Menu 1.1
		ituWidgetSetVisible(BackgroundMenuArea141, false);
		ituWidgetSetVisible(BackgroundMenuArea11, true);
		updateShowConfig(11, 15, BackgroundMenuMove11);
		break;
	case 14101:
		if (updateBlink(TextMenuC14101)){
			{
			// unsigned char tmpswitch;
			 {
			 	//tmpswitch = tmpswitch;
			 }
			outQsetTypeValue(UE_SetLoadContorlCommand, UICtrlShowData.dataMRS.LOAD_STATUS1);
			printf("TextMenuC14101\n");
			}
		}
		break;
	case 14102:
		if (updateBlink(TextMenuC14102)){
			{
			outQsetTypeValue(UE_SetLoadContorlCommand, UICtrlShowData.dataMRS.LOAD_STATUS1);
			printf("TextMenuC14102\n");
			}
		}
		break;
	case 14103:
		if (updateBlink(TextMenuC14103)){
			{
			outQsetTypeValue(UE_SetLoadContorlCommand, UICtrlShowData.dataMRS.LOAD_STATUS1);
			printf("TextMenuC14103\n");
			}
		}
		break;
	case 14104:
		break;
	case 14105:
		ituWidgetSetVisible(BackgroundMenuArea1411, false);
		ituWidgetSetVisible(BackgroundMenuArea141, true);
		updateShowConfig(141, 145, BackgroundMenuMove141);
		break;
	
	case 14201:
	case 14202:
	case 14203:
	case 14204:
	case 14205:
	case 14206:
	case 14207:
	case 14208:
	case 14209:
	case 14210:
		ituWidgetSetVisible(BackgroundMenuArea1421, false);
		ituWidgetSetVisible(BackgroundMenuArea141, true);
		updateShowConfig(141, 145, BackgroundMenuMove141);
		break;
	
	case 1441://
		theConfig.screenLog = reversal(theConfig.screenLog);
		ConfigSave();
		//upgrade
		//DownLoadApp();
		break;
	case 1445://back
		ituWidgetSetVisible(BackgroundMenuArea1441, false);
		ituWidgetSetVisible(BackgroundMenuArea141, true);
		updateShowConfig(141, 145, BackgroundMenuMove141);
		break;
	
	default:
		break;
	}
	return true;
}




static u8 changeDataTimer = 0;
bool changeTextAfter1S(){//每一秒，比较一次数据；
	changeDataTimer++;
	if (changeDataTimer < 100 || blinkConfig.isBlink || ituWidgetIsVisible(BackgroundWeekBorderArea)){
		if (blinkConfig.isBlink || ituWidgetIsVisible(BackgroundWeekBorderArea)){
			changeDataTimer = 0;//改数据时，设置保存数据后1s比较数据；
		}
		return false;
	}
	changeDataTimer = 0;
	bool result = false;
	if (strcompare((u8*)&UICtrlShowData.dataPP, (u8*)&pp, sizeof(WPEVENT_PROGRAM_PARAMRTER)) != -1){//0x200
		printf("change 200 bcz datachange\n");
		memcpy(&UICtrlShowData.dataPP, &pp, sizeof(WPEVENT_PROGRAM_PARAMRTER));//init
		setData200OnTimer();
		result |= true;
	}
	if (strcompare((u8*)&UICtrlShowData.dataDP, (u8*)&dp, sizeof(WPEVENT_DISCOUNT_PARAMRTER)) != -1){//0x300
		printf("change 300 bcz datachange\n");
		memcpy(&UICtrlShowData.dataDP, &dp, sizeof(WPEVENT_DISCOUNT_PARAMRTER));//init
		setData300OnTimer();
		result |= true;
	}
	if (strcompare((u8*)&UICtrlShowData.dataIP, (u8*)&ip, sizeof(WPEVENT_INFO_PARAMRTER)) != -1){//0x400
		printf("change 400 bcz datachange\n");
		memcpy(&UICtrlShowData.dataIP, &ip, sizeof(WPEVENT_INFO_PARAMRTER));//init
		setData400OnTimer();
		setData43COnTimer();
		result |= true;
	}
	if (strcompare((u8*)&UICtrlShowData.dataCP, (u8*)&cp, 0x15) != -1){//0x500//关于时间的
		printf("change 500 bcz datachange\n");
		memcpy(&UICtrlShowData.dataCP, &cp, 0x15);//init
		setData500OnTimer();
		result |= true;
	}
	if (strcompare(((u8*)&UICtrlShowData.dataCP + 20), ((u8*)&cp + 20), sizeof(SetMachineRunParameters)) != -1){//0x520
		printf("change 520 bcz datachange\n");
		printf(" <%d> <%d> datachange\n",cp.isShowTemp_OUT,cp.isShowTemp_IN);

		memcpy(((u8*)&UICtrlShowData.dataCP + 20), ((u8*)&cp + 20), sizeof(SetMachineRunParameters));//init
		UICtrlShowData.dataCP.isShowTemp_IN = cp.isShowTemp_IN;
		UICtrlShowData.dataCP.isShowTemp_OUT= cp.isShowTemp_OUT;
		if(UICtrlShowData.dataCP.SCREEN_BRIGHTNESS<20)
			{
						printf("change SCREEN_BRIGHTNESS bcz datachange\n");

				UICtrlShowData.dataCP.SCREEN_BRIGHTNESS = 60;
			}
		setData520OnTimer();
		result |= true;
	}
	if (strcompare((u8*)&UICtrlShowData.dataMS, (u8*)&ms, sizeof(WPEVENT_MACHINE_STATE)) != -1){//0x600
		printf("change 600 bcz datachange\n");
		memcpy(&UICtrlShowData.dataMS, &ms, sizeof(WPEVENT_MACHINE_STATE));//init
		setData600OnTimer();
		result |= true;
	}
	if (strcompare((u8*)&UICtrlShowData.dataMRS, (u8*)&mrs, sizeof(WPEVENT_MACHINE_RUN_STATE)) != -1){//0x600
		printf("change 600 bcz datachange\n");
		memcpy(&UICtrlShowData.dataMRS, &mrs, sizeof(WPEVENT_MACHINE_RUN_STATE));//init
		setData700OnTimer();
		result |= true;
	}
	if (UICtrlShowData.data14201 != ui.StateA){//门状态
		UICtrlShowData.data14201 = ui.StateA;
		//setData14201();
		//setData14101();
		result |= true;
	}
	return result;
}
//时效性数据 第一时间变化
bool changeTextMaster(){
	bool result = false;
	if (UICtrlShowData.dataCP.RTC_Second != cp.RTC_Second){
		UICtrlShowData.dataCP.RTC_Second = cp.RTC_Second;
		setData500OnTimer();
		result |= true;
	}
	return result;
}
ITUText* TextMenuC1441;
bool MainOnEnter(ITUWidget* widget, char* param)
{
#ifdef O_WIN32
	printf("\n\n\n\n\n\n");
#endif
	findMainWidget();
	initText();
	initBlink();
	editMode =0;
	ButtonUpOnPress = &MainButtonUpOnPress;//MainButtonUpOnPress
	ButtonDownOnPress = &MainButtonDownOnPress;//MainButtonDownOnPress
	ProcessButtomDown = &MainProcessButtomDown;
	//为了第一时间初始化一次
	changeDataTimer = 100;
	changeTextAfter1S();
	//初始化end
	//设置版本号
	findWidget(TextMenuC1441);
	ituTextSetString(TextMenuC1441, theConfig.sw);
	//设置版本号end

//	findWidget(BackgroundPinCode);//zpp
//	initPermission();
	UICtrlShowData.data1333 = theConfig.timeZone;//zpp
	return true;
}


bool MainOnTimer(ITUWidget* widget, char* param)
{
	bool ret = false;

	ret |= changeTextMaster();
	ret |= changeTextAfter1S();

	/*
	blinkConfig
	*/
	if (blinkConfig.isBlink){
		ret |= blink();//反转blinkConfig.blinkText->widget.visible,initBlink后,需要配置updateBlink
	}

	if (refreshLayerFlag){
		refreshLayerFlag = false;
		ret |= true;
	}
	if (refreshServiceLayer){
		ret = true;
		refreshServiceLayer = false;
	}
	if (ret){
		printf("%s() %ld retrun true\r\n", __FUNCTION__, __LINE__);
	}
	return ret;
}

bool MainOnLeave(ITUWidget* widget, char* param)
{
	memset(&blinkConfig, 0, sizeof(blinkForShow));//设置过程中退出，清除缓存
	if (blinkConfig.blinkText){
		if (!ituWidgetIsVisible(blinkConfig.blinkText)){//有可能在闪烁时退出
			ituWidgetSetVisible(blinkConfig.blinkText, true);
		}
	}
	bgSiblingToHide(BackgroundMenuArea11);

	theCache.temp = NONE;
	tp = NULL;
	UI_tp = NULL;
	return true;
}
