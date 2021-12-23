#include "scene.h"
#include <unistd.h>//usleep


/*
#include "projutils.h"
*/
#define VERSIONNO "V1.0.0" 


#ifdef WIN32
#define O_WIN32 //理想状态下，开启后编译的版本可以脱离底板，关闭后的版本必须依赖底板。
#else
#define O_RTOS
#endif

#ifdef O_WIN32
#define O_RTOS//shandiao
#endif 


typedef unsigned char u8;
typedef unsigned short int u16;
typedef unsigned long u32;

typedef signed char int8_t;

extern bool getbit(u8 num, u8 x);
extern void revbit(u8* num, u8 x);
#define setbit(x,y) x|=(1<<y) //将X的第Y位置1
#define clrbit(x,y) x&=~(1<<y) //将X的第Y位清0

#define LSB(A) (uint8_t)(A & 0x00FF)
#define MSB(A) (uint8_t)((A & 0xFF00) >> 8)

#define findWidget(name)  do{if(!name)(name = easyFindWidget((name),(#name)));}while(0)

//typedef unsigned char uint8_t;
//typedef unsigned int uint16_t;
//typedef unsigned long uint32_t;

//typedef uint8_t  u8;
//typedef uint16_t u16;
//typedef uint32_t u32;

//typedef signed int int16_t;
//typedef signed long int32_t;
#define BUTTON_DEBUG
#ifdef BUTTON_DEBUG
#define BUTTON_LOG    printf(
#else
#define BUTTON_LOG   (void)(1 ? 0 :
#endif

#define RUISA_DEBUG
#ifdef RUISA_DEBUG
#define RUISA_LOG    printf(
#else
#define RUISA_LOG   (void)(1 ? 0 :
#endif

#define UPS_DEBUG //write to uart log
#ifdef UPS_DEBUG
#define UPS_LOG printf(
#else
#define UPS_LOG   (void)(1 ? 0 :
#endif

//#define WRITEBUS_DEBUG //write to uart log
#ifdef WRITEBUS_DEBUG
#define WRITEBUS_LOG printf(
#else
#define WRITEBUS_LOG   (void)(1 ? 0 :
#endif

#define READBUS_DEBUG //read from uart log
#ifdef READBUS_DEBUG
#define READBUS_LOG printf(
#else
#define READBUS_LOG   (void)(1 ? 0 :
#endif

#define LOG_END         );

extern u8 util_Timer;
extern int strcompare(u8 *str1, u8 *str2, u8 length);

/**
* Encapsulation ituSceneFindWidget.
*
* @param widget	The name in development time.
* @param name	The name in the itu.
*/
void* easyFindWidget(void* widget, char* name);

/**
*
*/
extern u8 isboot;

//执行前先更改theConfig.lang
//带有ConfigSave();
void ChangeLangCommand();

bool DelayToSetNextLang(ITUCommandFunc func, int delay);
bool DelayToSetConfigLang(int lang, int delay);
bool DelayToSetAllLang(int delay);
/**
*CRC
*/
#define LSB(A) (uint8_t)(A & 0x00FF)
#define MSB(A) (uint8_t)((A & 0xFF00) >> 8)
void CalcCrc16(u16 * crc, u8 data);
bool checkCRC(u8 * data, u8 len, u16 recv_crc);//houda 

/**
*
*/
void gotoMainLayer();

int reversal(int rev);


u8 getU8(u8* start);
void setU8(u8* start, u8 num);
u16 getU16(u8* start);
void setU16(u8* start, u16 num);
int16_t getInt16(u8* start);
void setInt16(u8* start, int16_t num);

u32 getU32(u8* start);
void setU32(u8* start, u32 num);

void strreversal(char *s);

void (*layerCodeProcessMouseDown)();
void  emptyFunction();

void* TimerTask(void* arg);

/*
0xffffffff白色
0xff000000黑色
0xffff0000红色
*/
ITUColor makeColor(long ARGB);
void textSetBackColor(ITUText* text, long ARGB);
void bgSiblingToHide(ITUBackground *background);
void textSetOnOff(ITUText* text, bool flag);
void textSetYesNo(ITUText* text, bool flag);
void textSetFloatNumber(ITUText* text, float num, int type);
/*
'2'两位整数不足补零
*/
void textSetU16WithType(ITUText* text, u16 num, int type);
void textSetIntNumber(ITUText* text, int num);
void textSetU16Number(ITUText* text, u16 num);
void textSetU32Number(ITUText* text, u32 num);
void textSet04HexNumber(ITUText* text, u16 num);

void ituWidgetSetRect(ITUWidget* widget, ITURectangle rect);
void ituWidgetSetXYWH(ITUWidget* widget, int x, int y, int width, int height);

void ituWidgetSetColor2(ITUWidget* widget, long ARGB);



#ifndef THREAD_HELPER_H
#define THREAD_HELPER_H

void CreateWorkerThread(void *(*start_routine)(void *), void *arg);

#endif  /* THREAD_HELPER_H */