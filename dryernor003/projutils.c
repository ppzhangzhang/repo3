#include <assert.h>
#include "projutils.h"




static ITULayer* mainLayer;
u8 util_Timer = 0;
u8 isboot = 0;

bool getbit(u8 num, u8 x)
{
	return (num&(1 << x)) == 0 ? false : true;
}
void revbit(u8* num, u8 x){
	if (getbit(*num, x)){
		clrbit(*num, x);
	}
	else{
		setbit(*num, x);
	}
}
u8 getU8(u8* start)
{
	return (*start);
}
void setU8(u8* start, u8 num)
{
	*(start) = (u8)num;
}
u16 getU16(u8* start)
{
	return (*start) * 0x100 + (*(start + 1));
}
void setU16(u8* start, u16 num)
{
	*start = (u8)(num >> 8);
	*(start + 1) = (u8)num;
}
int16_t getInt16(u8* start)
{
	int16_t cache = 0;
	u8* ptr = (u8*)&cache;
	*(ptr + 1) = *start;
	*ptr = *(start + 1);
	return cache;
}
void setInt16(u8* start, int16_t num)
{
	*start = (u8)(num >> 8);
	*(start + 1) = (u8)num;
}

u32 getU32(u8* start)
{
	return (*start) * 0x1000000 + (*(start + 1)) * 0x10000 + (*(start + 2)) * 0x100 + (*(start + 3));
}
void setU32(u8* start, u32 num)
{
	*start = (u8)(num >> 24);
	*(start + 1) = (u8)(num >> 16);
	*(start + 2) = (u8)(num >> 8);
	*(start + 3) = (u8)num;
}

void strreversal(char *s)
{
	char *p = s, *q = &s[strlen(s) - 1];
	char tmp;

	while (p<q)
	{
		tmp = *p;
		*p = *q;
		*q = tmp;
		p++, q--;
	}
}

void* easyFindWidget(void* widget, char* name){
	if (!widget){
		widget = ituSceneFindWidget(&theScene, name);
		assert(widget);
	}
	return widget;
}

void emptyFunction(){

}




void ChangeLangCommand()
{
	if (theConfig.lang >= 0 && theConfig.lang <= 2){//LANGLAST
		ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, theConfig.lang, 0, 0);
		ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);
		ConfigSave();
	}
	else{
		//no such lang
		ituSceneUpdate(&theScene, ITU_EVENT_LANGUAGE, 0, 0, 0);
		ituSceneUpdate(&theScene, ITU_EVENT_LAYOUT, 0, 0, 0);
		ConfigSave();
	}
}

bool DelayToSetNextLang(ITUCommandFunc func, int delay)
{
	ituSceneExecuteCommand(&theScene, delay, func, 0);//0.02s x delay
	return true;
}
bool DelayToSetConfigLang(int lang, int delay)
{
	theConfig.lang = lang;
	ituSceneExecuteCommand(&theScene, delay, (ITUCommandFunc)ChangeLangCommand, 0);//0.02s x delay
	return true;
}
bool DelayToSetAllLang(int delay)
{
	ituSceneExecuteCommand(&theScene, delay, (ITUCommandFunc)ChangeLangCommand, 0);//0.02s x delay
	return true;
}

void gotoMainLayer(){
	findWidget(mainLayer);
	ituLayerGoto(mainLayer);
}
int reversal(int rev){
	if ((bool)rev == true){
		return false;
	}
	else if ((bool)rev == false){
		return true;
	}
	else{
		return true;
	}
}
//如果字符串相等返回-1，不相等则返回不相等的char偏移位置
int strcompare(u8 *str1, u8 *str2, u8 length){
	for (u8 i = 0; i < length; i++)
	{
		if (*(str1 + i) != *(str2 + i)){
			return i;
		}
	}
	return -1;
}

/*
按位设置价格
*/
void ituTextSetU16toString(ITUText* text,u16 num)
{
	char str3[3] = { 0 };
	if (num >= 10){
		return;
	}
	sprintf(str3, "%d", num);
	ituTextSetString(text, str3);
}
void ituTextSetPrice134(ITUText* text1, ITUText* text3, ITUText* text4, u16 i)
{
	if (i >= 1000){
		return;
	}
	u16 price1 = i / 100;
	u16 price2 = i % 100 / 10;
	u16 price3 = i % 10;
	ituTextSetU16toString(text1, price1);
	ituTextSetU16toString(text3, price2);
	ituTextSetU16toString(text4, price3);
}
void* TimerTask(void* arg){
	while (true){
		usleep(100000);//us 1s = 1000000
		util_Timer++;
		util_Timer %= 10;
		//printf("timer = %d\n", util_Timer);
	}
}

ITUColor makeColor(long ARGB){//GBRA
	char *p = (char *)&ARGB;
	ITUColor aColor;
	aColor.blue = *p++;
	aColor.green = *p++;
	aColor.red = *p++;
	aColor.alpha = 0xff;//*p
	return aColor;
}
void textSetBackColor(ITUText* text,long ARGB)
{
	char *p = (char *)&ARGB;
	ituTextSetBackColor(text, (uint8_t)*(p+3), (uint8_t)*(p+2), (uint8_t)*(p+1), (uint8_t)*p);
}

void bgSiblingToHide(ITUBackground *background){
	for (ITUBackground* i = background; i != NULL; i = (ITUBackground *)i->icon.widget.tree.sibling){
		if (i->icon.widget.type == ITU_BACKGROUND){
			if (ituWidgetIsVisible((ITUWidget*)i)){
				ituWidgetSetVisible((ITUWidget*)i, false);
			}
		}
	}
}

void textSetOnOff(ITUText* text, bool flag){
	ituTextSetString(text, flag ? "ON" : "OFF");
}
void textSetYesNo(ITUText* text, bool flag){
	ituTextSetString(text, flag ? "YES" : "NO");
}

void textSetU16WithType(ITUText* text, u16 num, int type){
	char str[32] = { 0 };
	switch (type)
	{
	case '%':
		sprintf(str, "%d%s", num, "%");
		break;
	case 'F':
		sprintf(str, "%d%s", num, "°F");
		break;
	case 'C':
		sprintf(str, "%d%s", num, "°C");
		break;
	case '2':
		sprintf(str, "%02d", num);
		break;
	default:
		textSetU16Number(text, num);
		return;
		break;
	}
	ituTextSetString(text, str);
}
void textSetFloatNumber(ITUText* text, float num, int type){
	char str[32] = { 0 };
	int temp = 0;
	switch (type)
	{
	case 0:

		break;
	case 1:// 5.0 to show
		temp = (int)(num * 10);
		if (temp >= 0){
			sprintf(str, "%d%s%d", temp / 10, ".", temp % 10);
		}
		else{
			temp *= -1;
			sprintf(str, "-%d%s%d", temp / 10, ".", temp % 10);
		}
		break;
	case 'C':
		temp = (int)(num);
		sprintf(str, "%d%s%d% s", temp / 10, ".", temp % 10, "C");
		break;
	case 'F':
		break;
	case '%':
		temp = (int)(num);
		sprintf(str, "%d%s%d% s", temp / 10, ".", temp % 10, "%");
		break;
	default:
		break;
	}
	ituTextSetString(text, str);
}
void textSetIntNumber(ITUText* text, int num){
	char str[32] = { 0 };
	sprintf(str,"%d",num);
	ituTextSetString(text, str);
}
void textSetU16Number(ITUText* text, u16 num)
{
	char str[32] = { 0 };
	sprintf(str, "%d", num);
	ituTextSetString(text, str);
}
void textSet04HexNumber(ITUText* text, u16 num)
{
	char str[32] = { 0 };
	sprintf(str, "%04x", num);
	ituTextSetString(text, str);
}

void ituWidgetSetRect(ITUWidget* widget, ITURectangle rect){
	ituWidgetSetPosition(widget, rect.x, rect.y);
	ituWidgetSetWidth(widget, rect.width);
	ituWidgetSetHeight(widget, rect.height);
}
void ituWidgetSetXYWH(ITUWidget* widget, int x, int y, int width, int height)
{
	ituWidgetSetPosition(widget, x, y);
	ituWidgetSetWidth(widget, width);
	ituWidgetSetHeight(widget, height);
}
void ituWidgetSetColor2(ITUWidget* widget, long ARGB)
{
	char *p = (char *)&ARGB;
	ITUColor aColor;
	aColor.blue = *p++;
	aColor.green = *p++;
	aColor.red = *p++;
	aColor.alpha = 0xff;//*p
	ituWidgetSetColor(widget, aColor.alpha, aColor.red, aColor.green, aColor.blue);
}

/*static*/ const unsigned short crc16_lut0[16] = {
	0x0000U, 0xC0C1U, 0xC181U, 0x0140U, 0xC301U, 0x03C0U, 0x0280U, 0xC241U,
	0xC601U, 0x06C0U, 0x0780U, 0xC741U, 0x0500U, 0xC5C1U, 0xC481U, 0x0440U };

/*! Second table. */
/*static*/ const unsigned short crc16_lut1[16] = {
	0x0000U, 0xCC01U, 0xD801U, 0x1400U, 0xF001U, 0x3C00U, 0x2800U, 0xE401U,
	0xA001U, 0x6C00U, 0x7800U, 0xB401U, 0x5000U, 0x9C01U, 0x8801U, 0x4400U };
void CalcCrc16(u16 * crc, u8 data)
{
	u8 temp = 0;
	temp = LSB(*crc) ^ data;
	*crc = (crc16_lut0[temp & 0xf] ^ crc16_lut1[(temp >> 4)] ^ (MSB(*crc)));
}/* end of CalcCrc */
bool checkCRC(u8 * data, u8 len, u16 recv_crc)//houda 
{
	u8 i;
	u16 crc = 0;
	//RUISA_LOG "checkCRC dataLen is %d recv crc :%x \n", len, recv_crc LOG_END
	for (i = 0; i < len; i++)
	{
		//	RUISA_LOG "checkCRC data[%d]  :%x \n",i,data[i] LOG_END
		CalcCrc16(&crc, data[i]);
	}
	//RUISA_LOG "checkCRC crc  :%x  recv crc :%x \n", crc, recv_crc LOG_END;
	if (recv_crc == crc)
	{
		return true;
	}
	RUISA_LOG "checkCRC crc  error  \n"LOG_END;
	return false;
}


//THREAD_HELPER_H
#include <pthread.h>

void CreateWorkerThread(void *(*start_routine)(void *), void *arg)
{
	pthread_t      task;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_create(&task, &attr, start_routine, arg);
}
