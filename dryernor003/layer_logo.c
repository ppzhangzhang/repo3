#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "boardstate.h"

#include "projutils.h"

#include "layer_ctrl.h"

/*
左下 中上 右确认
ctrlLayer 属性always visible,所以他的ontimer一直再跑
*/

bool LogoOnEnter(ITUWidget* widget, char* param)
{
#ifdef O_WIN32
	ui.MoneyCostL = (u8)0x00;
	ui.MoneyCostH = (u8)0x00;
	ui.RmainTimeL = 0x01;
	ui.CycleName = 0x01;
	ui.RunMode = RM_STANDBY;
	char data[18] = { 0, 0x19, 0, 8, 0, 8, 1, 0, 0x1e, 0, 0x5a, 0, 0x96, 1, 0x5e, 1, 0, 0x78 };
	memcpy(&(pp.HIGH), data, 18);
	memcpy(&(pp.MIDIUM), data, 18);
	memcpy(&(pp.LOW), data, 18);
	memcpy(&(pp.NOHEAT), data, 18);
	cp.SHOW_DECIMAL = 1;
	//ui.CycleName = 0xfa;
	//setU16(&tAndp.HIGH,300);
#endif

#ifdef WIN32
	pp.HIGH.TP_Available = 1;
	pp.HIGH.TP_CoinAdd_L = 1;
	pp.HIGH.TP_CoolingTemp_L = 1;
	pp.HIGH.TP_CoolingTime = 1;
	pp.HIGH.TP_Default_L = 1;
	pp.HIGH.TP_Free_L = 1;
	pp.HIGH.TP_Maximum_L = 1;
	pp.HIGH.TP_Price_L = 1;
	pp.HIGH.TP_Temperature_L = 1;
	pp.HIGH.TP_TempLimit_L = 1;

	dp.dc0.DC_StartHour = 9;
	dp.dc0.DC_StartMinute = 30;
	dp.dc0.DC_EndHour = 17;
	dp.dc0.DC_EndMinute = 30;
	dp.dc0.DC_AddTime = 50;
	dp.dc0.DC_DcDay = 0x7F;

	dp.dc2.DC_StartHour = 9;
	dp.dc2.DC_StartMinute = 30;
	dp.dc2.DC_EndHour = 17;
	dp.dc2.DC_EndMinute = 30;
	dp.dc2.DC_AddTime = 50;
	dp.dc2.DC_DcDay = 0x7F;

	cp.DEFAULT_PROGRAM = 2;

	

	cp.COIN1_VALUE_L = 25;
	cp.COIN2_VALUE_L = 100;

	cp.SHOW_DECIMAL = 1;
	cp.AUTO_START_PAID = 1;

	ui.StateA = 0x00;
	ms.LOAD_STATUS2 = 0xff;

	cp.SCREEN_BRIGHTNESS = 0x32;

	setU32(&ip.count.allCoinSum, 100);


	cp.DEFAULT_PROGRAM = 0;

	cp.COIN1_VALUE_L = 25;
	cp.COIN2_VALUE_L = 100;

	cp.SHOW_DECIMAL = 1;
	cp.AUTO_START_PAID = 1;


	ui.StateA = 0x00;
	ms.LOAD_STATUS2 = 0xff;

	cp.SCREEN_BRIGHTNESS = 0x32;

	setU32(&ip.count.allCoinSum, 10086);
	setU32(&ip.count.tripCoinSum, 10086);
	setU32(&ip.count.highTime, 10086);
	setU32(&ip.count.mediumTime, 10086);
	setU32(&ip.count.lowTime, 10086);
	setU32(&ip.count.noheatTime, 10086);
	setU32(&ip.count.allSumTime, 10086);
	setU32(&ip.count.tripSumTime, 10086);

	ms.WP_USER_H = 17;
	ms.WP_USER_L = 17;

#endif


	cp.SHOW_DECIMAL = 1;
	findWidget(ctrlLayer);
	ituLayerGoto(ctrlLayer);
    return true;
}
bool LogoOnTimer(ITUWidget* widget, char* param)
{

	return true;
}
bool LogoOnLeave(ITUWidget* widget, char* param)
{

	return true;
}
