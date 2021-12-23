#include "projutils.h"
#include "ite/itp.h"


/*
#include "boardstate.h"
*/





#define WRITECMD  0X01 
#define WRITE_ADDR	0X0
#define OUT_EXTERNAL_BUFFER_SIZE  128

typedef enum
{
	QUERY_NONE = 0,//0
	QUERY_200_TO_22C_2C = 1,//1
	QUERY_300_TO_33C_3C,//2
	QUERY_400_TO_43C_3C,//3
	QUERY_43C_TO_468_2C,//4
	QUERY_500_TO_515_15,//5
	QUERY_520_TO_545_25,//6
	QUERY_600_TO_52E_2E,//7
	QUERY_160_TO_170_10,//8
	QUERY_700_TO_711_12,
       QUERY_170_TO_1FF_90,
}UI_QUERY_NAME;

typedef enum
{
	WASH_Nc = 0,					//0 空事�?
	WASH_Reset,						//1 复位
	WASH_PowerButton = 3,			//3	0=关机/1=开�?电源�?
	WASH_Cycle,						//4	程序�?程序选择
	WASH_CycleStart,				//5 程序�?程序选择+运行
	WASH_StartPause,				//6	0=暂停/1=启动 启停
	WASH_WSCycle = 7,				//7	0：取消选择 1: 选中
	WASH_Invalid = 10,				//10 无效操作
	WASH_FactoryReset,				//11 0=普通用�?1=售后 0=普通用户时,只将程序记忆恢复出厂�?1 = 售后�? 除将程序记忆恢复出厂值外, 还将清零除型号外其他记忆参数如周期数�?
	WASH_ReturnIdle,				//12 返回待机状�?
	WASH_MessageReply,				//13 提示信息序号(与WP上传序号�? 对话框键�?=�?1=确定/2=取消3=我知道了/4=不再提醒  提示信息应答 �?WP发出的提示信�?用户按触相应按键进行的确认操�?
	WASH_SetCycleParamters,			//14 0e 程序�?设置程序价格
	WASH_SetOptionPrice,			//15 
	WASH_SetHappyHourTime = 16,		//16 折扣序号
	WASH_SetRtc,					//17 设置RTC时间
	WASH_SetMachineRunParameters,	//18 设置机器运行参数
	WASH_LoadControlCommand,	//19设置机器运行参数
	WASH_writeSN,
	WASH_ResetParamters = 21,
	WASH_TestOptionsSwitch	= 201,	//201
	WASH_TstSpecialNum		= 202,	//202
	WASH_UniversalDialogBox = 203,	//0xCB 通用对话�?�?用于产线自检/手控负载 0=上一�?1=下一�?2 = 启停 / 3 = 返回
	EXTRA_WASH_QUERY,
	EXTRA_WRITE_BUFFER,
	EXTRA_WRITE_BUFFER_STRUCT,
	EXTERNAL_WARNNING,
	EXTERNAL_MACHINE_PARA,
	EXTERNAL_MACHINE_UIPARA,
	EXTERNAL_SHOW_INFO,
	EXTERNAL_UI_LOGICDATA,
	EXTERNAL_UI_GPRSInfo,
	EXTERNAL_UI_OFFLINE,
	EXTERNAL_UI_SPECIALDISPLAY,
	EXTERNAL_DRY_NULL,
	EXTERNAL_DRY_NORMAL,
	EXTERNAL_DRY_UNNORMAL,
	EXTERNAL_DRY_QUERY,//queryUILOGICStatus
	EXTERNAL_DRY_RESET,
}OutExternalEventType;
typedef enum{//下发�?UI=Myself DOWN
	UE_Nc = 0,					//0 空事�?
	UE_Reset,					//1 复位
	UE_PowerButton = 3,			//3	0=关机/1=开�?电源�?
	UE_Cycle,					//4	程序�?程序选择
	UE_CycleStart,				//5 程序�?程序选择+运行
	UE_StartPause,				//6	0=暂停/1=启动 启停
	UE_WSCycle = 7,				//7	0：取消选择 1: 选中
	UE_Invalid = 10,			//10 无效操作
	UE_FactoryReset,			//11 0=普通用�?1=售后 0=普通用户时,只将程序记忆恢复出厂�?1 = 售后�? 除将程序记忆恢复出厂值外, 还将清零除型号外其他记忆参数如周期数�?
	UE_ReturnIdle,				//12 返回待机状�?0x0c
	UE_MessageReply,			//13 提示信息序号(与WP上传序号�? 对话框键�?=�?1=确定/2=取消3=我知道了/4=不再提醒  提示信息应答 �?WP发出的提示信�?用户按触相应按键进行的确认操�?
	UE_SetCycleParamters,		//14 0e 程序�?设置程序价格
	UE_SetOptionPrice,			//15 
	UE_SetHappyHourTime = 16,	//16 折扣序号
	UE_SetRtc,					//17 设置RTC时间
	UE_SetMachineRunParameters,	//18 设置机器运行参数
	UE_SetLoadContorlCommand,	//19 
	UE_ResetParamters = 21,		//21服务模式参数复位 0-RESET TRIP COLLECTION COUNTER 1 - RESET TRIP CYCLE COUNTER 2 - RESET PARAMETERS TO FACTORY
	UE_UiTouch,					//UiTouch 筒灯 �?不断电在线升�?
	UE_ModuleSet = 200,			//200 0=进入型号设置/1=序号�?2=序号�?=保存/4=放弃
	UE_TestOptionsSwitch,		//201 0=加快十倍循环开�?=寿命测试循环开�?=washer模式循环开�?=特殊显示(B_TST)循环开�?=手控负载循环开�?=demo6=Diagnostic Swtich 测试模式开关注:用于开启和关闭一些特殊的工作模式
	UE_TstSpecialNum,			//202 1=�?/ 2=�?/ 3=�?0/ 4=�?0 特殊显示序号
	UE_UniversalDialogBox,		//203 0=上一�?1=下一�?=启停/3=返回 通用对话�?�?用于产线自检/手控负载
	UE_SetMotoPara,				//204 0=转�?=加速度2=减速度 �?双字节高位在前转�?范围0-1600rpm,加速度:范围0-1001rpm/s减速度:范围0-1001rpm/s 用于几个特殊流程
	UE_UISendData,				//205 0=UI version,高位在前 1=其他 data length data�?
	UE_END = 255,
}UI_EVENT_TYPE;
typedef struct
{
	OutExternalEventType type;
	u8 arg0;//value
	u8 arg1;//set location
	u16 arg2;//set value
	u8 arg3_buf1Size;//complex set value size
	u8 buf1[OUT_EXTERNAL_BUFFER_SIZE];//complex set value size

}OutExternalEvent;
typedef enum{
	MASTER_IDLE,//0
	MASTER_STATE_READ_RST_TIME,
	MASTER_STATE_READ_UI_LOGIC,//读ui logic 0x100
	MASTER_STATE_READ_MODE_TEST,
	MASTER_STATE_READ_MAC_STATE,//读机器运行参�?0x600
	MASTER_STATE_READ_SPECIAL_DISPLAY,
	MASTER_STATE_READ_INFO,//5
	MASTER_STATE_READ_INFO_DATA,
	MASTER_STATE_WTIRE_NULLCMD,
	MASTER_STATE_WRITE_NORMAL_CMD,
	MASTER_STATE_READ_NORMAL_CMD,
	MASTER_STATE_WRITE_CTRLFLOW_PACK,//houda add 

}MASTER_STATE_TYPE;
typedef enum{
	ADDR000 = 0,//0x000
	ADDR100,	//0x100 区域二，上传参数-Ui逻辑&&上传参数-型号设置/产测初始界面/产线自检/服务模式�?
	ADDR160,	//0x160 区域二，上传参数-特殊显示 WPW-special display
	ADDR170,	//0x170 区域二，上传信息参数(128bytes,地址0x170-0x1FF)
	ADDR200,	//0x200 区域三，上报参数-程序参数
	ADDR212,
	ADDR224,
	ADDR236,
	ADDR300,	//0x300 区域四，上报参数-折扣参数
	ADDR300A06,
	ADDR300A12,
	ADDR300A18,
	ADDR300A24,
	ADDR300A30,
	ADDR300A36,
	ADDR300A42,
	ADDR300A48,
	ADDR300A54,
	ADDR400,	//0x400 区域五，上报参数-统计参数
	ADDR43C,	//0x400 区域五，上报参数-统计参数
	ADDR500,	//0x500 区域六，上报参数-控制参数
	ADDR520,	//0x500 区域六，上报参数-控制参数
	ADDR600,	//0x600	区域七，上报参数-机器运行状�?
}QueryType;


/*
#include "dryer_board.h"
*/
/*
01 00 �?
*/
#define WRITE_CMD  0X01 
#define WRITE_ADDR	0X0

#define OVEN_MAX_LEN 128

/*
82 00 �?0x100
82 70 �?0x170
*/
typedef enum{
	addr000 = 0x0,//0x000
	addr100 = 0x100,//0x100 区域二，上传参数-Ui逻辑&&上传参数-型号设置/产测初始界面/产线自检/服务模式�?
	addr160 = 0x160,//0x160 区域二，上传参数-特殊显示 WPW-special display
	addr170 = 0x170,//0x170 区域二，上传信息参数(128bytes,地址0x170-0x1FF)
	addr200 = 0x200,//0x200 区域三，上报参数-程序参数
	addr212 = 0x212,
	addr224 = 0x224,
	addr236 = 0x236,
	addr300 = 0x300,//0x300 区域四，上报参数-折扣参数 768
	addr306 = 0x306,
	addr30C = 0x30C,
	addr312 = 0x312,
	addr318 = 0x318,
	addr31E = 0x31E,
	addr324 = 0x324,
	addr32A = 0x32A,
	addr330 = 0x330,
	addr336 = 0x336,
	addr400 = 0x400,//0x400 区域五，上报参数-统计参数
	addr406 = 0x406,
	addr40C = 0x40C,
	addr412 = 0x412,
	addr418 = 0x418,
	addr41E = 0x41E,
	addr424 = 0x424,
	addr42A = 0x42A,
	addr430 = 0x430,
	addr436 = 0x436,
	addr43C = 0x43C,//32B
	addr500 = 0x500,//0x500 区域六，上报参数-控制参数
	addr520 = 0x520,//0x500 区域六，上报参数-控制参数
	addr600 = 0x600,//0x600	区域七，上报参数-机器运行状�?
	addr700 = 0x700,
}DataOffset;

/*
******************************************************************************
*/
typedef enum{
	PAUSE = 0,
	START = 1,
}StartPauseType;
typedef enum{
	NONE = 0,
	HIGH = 1,//1
	MIDIUM,//
	LOW,//
	NOHEAT,//4
}CycleType;
typedef enum{
	TstStep_PRE = 0,//	
	TstStep_NEXT, //	
	TstStep_START, //	
	TstStep_BACK, //	
	TstStep_ADDH,
	TstStep_SAVE,
	TstStep_RETURN,

}TstStep_TYPE;
//typedef enum{//下发�?
//	Nc = 0,				//0 空事�?
//	Reset,				//1 复位
//	PowerButton = 3,	//3	0=关机/1=开�?电源�?
//	Cycle,				//4	程序�?程序选择
//	CycleStart,			//5 程序�?程序选择+运行
//	StartPause,			//6	0=暂停/1=启动 启停
//	Invalid = 10,		//10 无效操作
//	FactoryReset,		//11 0=普通用�?1=售后 0=普通用户时,只将程序记忆恢复出厂�?1 = 售后�? 除将程序记忆恢复出厂值外, 还将清零除型号外其他记忆参数如周期数�?
//	ReturnIdle,			//12 返回待机状�?
//	MessageReply,		//13 提示信息序号(与WP上传序号�? 对话框键�?=�?1=确定/2=取消3=我知道了/4=不再提醒  提示信息应答 �?WP发出的提示信�?用户按触相应按键进行的确认操�?
//	SetCycleParamters,	//14 0e 程序�?设置程序价格
//	SetHappyHourTime = 16,//16 0x10	折扣序号
//	SetRtc,				//17 0x11 设置RTC时间
//	SetMachineRunParameters,//18 0x12 设置机器运行参数
//	UiTouch,			//UiTouch 筒灯 �?不断电在线升�?
//	ModuleSet = 200,	//200 0=进入型号设置/1=序号�?2=序号�?=保存/4=放弃
//	TestOptionsSwitch,	//201 0=加快十倍循环开�?=寿命测试循环开�?=washer模式循环开�?=特殊显示(B_TST)循环开�?=手控负载循环开�?=demo6=Diagnostic Swtich 测试模式开关注:用于开启和关闭一些特殊的工作模式
//	TstSpecialNum,		//202 1=�?/ 2=�?/ 3=�?0/ 4=�?0 特殊显示序号
//	UniversalDialogBox,	//203 0=上一�?1=下一�?=启停/3=返回 通用对话�?�?用于产线自检/手控负载
//	SetMotoPara,		//204 0=转�?=加速度2=减速度 �?双字节高位在前转�?范围0-1600rpm,加速度:范围0-1001rpm/s减速度:范围0-1001rpm/s 用于几个特殊流程
//	UISendData,			//205 0=UI version,高位在前 1=其他 data length data�?
//	EXTRA_QUERY,//查询
//	EXTRA_SETBUFFER,//设置
//	UIEVENT_TstStep,//洗鞋�?testmode移植
//}UI_EVENT_TYPE;


/*
******************************************************************************
*/
typedef enum{
	CN_NONE = 0,
	CN_HIGH = 1,//1
	CN_MIDIUM,//2
	CN_LOW,//3
	CN_NOHEAT,//4
}CycleNameType;
typedef enum{
	RM_INIT = -1,
	RM_OFF = 0,
	RM_STANDBY,//1
	RM_RUNNING,//2
	RM_PAUSE,
	RM_ADD
}RunModeType;
typedef enum{
	DS_idle = 0,//0
	DS_delay,
	DS_sensing,
	DS_dry,
	DS_cooling,
	DS_wrinklefree = 5,
	DS_unlock,
	DS_ending = 7,//7
}DryingStageType;
typedef enum{// == CycleNameTYPE
	cx_tst_ui = 0xf9,
	cx_tst_setspd = 0xF0,
	cx_service_mode = 0xFA,	//1
	cx_tst_run = 0xFB,		//2试运�?
	cx_machine_test = 0xFC,	//2整机测试
	cx_pcb_test = 0xFD,		//3pcb测试
	cx_tst_initial,
	cx_modul_set,
}SPECIAL_PROCESS_TYPE;
typedef struct{//WP_EVENT_UI_LOGIC 0x100  00~3F //接收�?
	u8 UI_EventSynCounter;	//0 UI事件同步�?0~255 保持和UI事件计数器UI_EventCounter同步,用于OU板确认WP接收到相应事�?
	u8 CycleName;			//1 程序�?0~53
	u8 RunMode;				//2 WP运行模式 0~4 20=关机 1=待机 2=运行 3=暂停 4 =中途添�?其他=非法
	u8 DryingStage;			//3 烘干阶段 用于Ui显示洗涤阶段,如称重中,洗涤中……见<洗涤阶段序列�?
	u8 KeyValue1;			//4 按位表示：Bit0：上�?Bit1：下�?Bit2: 启动�?&0x01 &0x02 &0x04
	u8 KeyValue2;			//5
	u8 StateA;				//6 &0x80  0：门开�?1：门关闭
	u8 StateB;				//7
	u8 FaultCode;			//8 故障代码 0=�?101~200=故障代码,详见章节<提示信息及报警代码表> 0 = 关闭所有提示信息及报警信息对话�?
	u8 RmainTimeH;			//9 
	u8 RmainTimeL;			//A 基本只用L
	u8 MoneyCostH;			//B
	u8 MoneyCostL;			//C 基本只用L
	u8 NULL0D;
	u8 NULL0E;
	u8 NULL0F;
	u8 NULL10; u8 NULL11; u8 NULL12; u8 NULL13; u8 NULL14; u8 NULL15; u8 NULL16; u8 NULL17; u8 NULL18; u8 NULL19; u8 NULL1A; u8 NULL1B; u8 NULL1C; u8 NULL1D; u8 NULL1E; u8 NULL1F;
	u8 NULL20; u8 NULL21; u8 NULL22; u8 NULL23; u8 NULL24; u8 NULL25; u8 NULL26; u8 NULL27; u8 NULL28; u8 NULL29; u8 NULL2A; u8 NULL2B; u8 NULL2C; u8 NULL2D; u8 NULL2E; u8 NULL2F;
	u8 NULL30; // &0x01 启停 0=允许/1=禁�?
	u8 NULL31; u8 Choice2; u8 NULL33; u8 NULL34; u8 NULL35; u8 NULL36; u8 NULL37; u8 NULL38; u8 NULL39; u8 NULL3A; u8 NULL3B; u8 NULL3C; u8 NULL3D; u8 NULL3E; u8 NULL3F;
	u8 NULL40; u8 NULL41; u8 NULL42; u8 NULL43; u8 NULL44; u8 NULL45; u8 NULL46; u8 NULL47; u8 NULL48; u8 NULL49; u8 NULL4A; u8 NULL4B; u8 NULL4C; u8 NULL4D; u8 NULL4E; u8 NULL4F;
	u8 NULL50; u8 NULL51; u8 NULL52; u8 NULL53; u8 NULL54; u8 NULL55; u8 NULL56; u8 NULL57; u8 NULL58; u8 NULL59; u8 NULL5A; u8 NULL5B; u8 NULL5C; u8 NULL5D; u8 NULL5E; u8 NULL5F;
}WP_EVENT_UI_LOGIC;
typedef struct{//WPEVENT_UI_LOGIC 0x100  00~3F //接收�?
	u8 data160;
	u8 data161;
	u8 data162;
	u8 data163;
	u8 data164;
	u8 data165;
	u8 data166;
	u8 data167;
	u8 data168;
	u8 data169;
	u8 data16A;
	u8 data16B;
	u8 data16C;
	u8 data16D;
	u8 data16E;
	u8 data16F;
}WPEVENT_UI_DATA160;


typedef struct{
	u8 TP_Price_H;		//0~0xffff 美分		价格
	u8 TP_Price_L;		//0~0xffff 美分		价格
	u8 TP_Default_H;		//0~0xffff	分钟	程序默认时间
	u8 TP_Default_L;		//0~0xffff	分钟	程序默认时间
	u8 TP_CoinAdd_H;		//0~0xffff	分钟	投币增加时间
	u8 TP_CoinAdd_L;		//0~0xffff	分钟	投币增加时间
	u8 TP_Available;	//0~1				程序是否可用
	u8 TP_Free_H;		//0~0xffff	分钟	最长免费烘干时�?
	u8 TP_Free_L;		//0~0xffff	分钟	最长免费烘干时�?
	u8 TP_Maximum_H;		//0~0xffff	分钟	最长烘干时�?
	u8 TP_Maximum_L;		//0~0xffff	分钟	最长烘干时�?
	u8 TP_Temperature_H;	//0~0xffff	分钟	程序温度
	u8 TP_Temperature_L;	//0~0xffff	分钟	程序温度
	u8 TP_TempLimit_H;	//0~0xffff	摄氏�?进风口温度限�?
	u8 TP_TempLimit_L;	//0~0xffff	摄氏�?进风口温度限�?
	u8 TP_CoolingTime;	//0~0xff	分钟	冷却时间
	u8 TP_CoolingTemp_H;	//0~0xffff	摄氏�?冷却温度
	u8 TP_CoolingTemp_L;	//0~0xffff	摄氏�?冷却温度
}TimePriceType;
typedef struct{
	TimePriceType HIGH;		//高温
	TimePriceType MIDIUM;	//中温
	TimePriceType LOW;		//低温
	TimePriceType NOHEAT;	//冷风
}WPEVENT_PROGRAM_PARAMRTER;


typedef struct{// 0x300  00~22 //接收�?
	u8 DC_StartHour;
	u8 DC_StartMinute;
	u8 DC_EndHour;
	u8 DC_EndMinute;
	u8 DC_AddTime;
	u8 DC_DcDay;
}WPEVENT_DISCOUNT_PARAMRTER_TYPE;
typedef struct{// 0x400  00~22 //接收�?
	u8 isUsed;
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
}WPEVENT_INFO_PARAMRTER_TYPE_INFO;
typedef struct{// 0x400  00~22 //接收�?
	u8 isUsed;
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
}WPEVENT_INFO_PARAMRTER_TYPE_WARN;
typedef struct{// 0x400  00~22 //接收�?
	u8 HIGH;
	u8 THIRD;
	u8 SECOND;
	u8 FIRST;
}BIGCOUNT;
typedef struct{// 0x400  00~22 //接收�?
	BIGCOUNT allCoinSum;
	BIGCOUNT tripCoinSum;
	BIGCOUNT highTime;
	BIGCOUNT mediumTime;
	BIGCOUNT lowTime;
	BIGCOUNT noheatTime;
	BIGCOUNT allSumTime;
	BIGCOUNT tripSumTime;
	BIGCOUNT hotProgCycle;
	BIGCOUNT warmProgCycle;
	BIGCOUNT lowProgCycle;
	BIGCOUNT coldProgCycle;
	BIGCOUNT totalCycles;
	BIGCOUNT tripCycles;
	BIGCOUNT coinTotalSum;
	BIGCOUNT cardTotalSum;
	BIGCOUNT networkTotalSum;
	BIGCOUNT coin1TotalSum;
	BIGCOUNT coin2TotalSum;
	BIGCOUNT tripcoin1TotalSum;
	BIGCOUNT tripcoin2TotalSum;
}WPEVENT_INFO_PARAMRTER_TYPE_COUNT;
typedef struct{// 0x300  00~22 //接收�?
	WPEVENT_INFO_PARAMRTER_TYPE_INFO info1;
	WPEVENT_INFO_PARAMRTER_TYPE_INFO info2;
	WPEVENT_INFO_PARAMRTER_TYPE_INFO info3;
	WPEVENT_INFO_PARAMRTER_TYPE_INFO info4;
	WPEVENT_INFO_PARAMRTER_TYPE_INFO info5;
	WPEVENT_INFO_PARAMRTER_TYPE_WARN warn1;
	WPEVENT_INFO_PARAMRTER_TYPE_WARN warn2;
	WPEVENT_INFO_PARAMRTER_TYPE_WARN warn3;
	WPEVENT_INFO_PARAMRTER_TYPE_WARN warn4;
	WPEVENT_INFO_PARAMRTER_TYPE_WARN warn5;
	WPEVENT_INFO_PARAMRTER_TYPE_COUNT count;
}WPEVENT_INFO_PARAMRTER;
typedef struct _w_d_p_event{// 0x300  00~22 //接收�?
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc0;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc1;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc2;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc3;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc4;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc5;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc6;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc7;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc8;
	WPEVENT_DISCOUNT_PARAMRTER_TYPE dc9;
}WPEVENT_DISCOUNT_PARAMRTER;




typedef struct{// 0x400  00~22 //接收�?
	u8 isUsed;
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
}WPEVENT_INFO_PARAMRTER_TYPE_OPEN;//钱盒打开
typedef struct{// 0x400  00~22 //接收�?
	u8 isUsed;
	u8 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
}WPEVENT_INFO_PARAMRTER_TYPE_ERRO;//错误
typedef struct{// 0x300  00~22 //接收�?
	WPEVENT_INFO_PARAMRTER_TYPE_OPEN open1;
	WPEVENT_INFO_PARAMRTER_TYPE_OPEN open2;
	WPEVENT_INFO_PARAMRTER_TYPE_OPEN open3;
	WPEVENT_INFO_PARAMRTER_TYPE_OPEN open4;
	WPEVENT_INFO_PARAMRTER_TYPE_OPEN open5;
	WPEVENT_INFO_PARAMRTER_TYPE_ERRO erro1;
	WPEVENT_INFO_PARAMRTER_TYPE_ERRO erro2;
	WPEVENT_INFO_PARAMRTER_TYPE_ERRO erro3;
	WPEVENT_INFO_PARAMRTER_TYPE_ERRO erro4;
	WPEVENT_INFO_PARAMRTER_TYPE_ERRO erro5;
	WPEVENT_INFO_PARAMRTER_TYPE_COUNT count;
}WPEVENT_INFO_PARAMRTER2;

typedef enum{
	dia_noborder = 0,		//0=无框/1=确定/2=我知道了+不再提醒/3=确定+取消
	dia_confirm,			//1=确定
	dia_confwithNoremind,	//2=我知道了+不再提醒
	dia_confwithCancel,		//3=确定+取消
}DialogTypeType;
typedef enum{
	bz_turnoff = 0,		//关闭当前蜂鸣�?
	bz_set_cycle,		//程序选择
	bz_poweron,			//开�?
	bz_poweroff,		//关机
	bz_pause,			//暂停
	bz_start = 5,		//启动
	bz_end,				//结束
	bz_warning,			//报警
	bz_functions = 8,	//功能热键
	bz_invalid,			//无效�?
	bz_pset = 10,		//设置参数
	bz_notification,	//提示�?
	bz_mute,			//静音
	bz_end_eu = 13,		//暂停欧洲型号结束
}MessageBuzzerType;
typedef struct{
	u8 MessageCounter;
	u8 MessageType;
	u8 MessageBuzzer;
	u8 DialogType;
	u8 MessageLength;
}WPW_INFO;
extern WPW_INFO wpwi;

typedef struct{// 0x500  00~22 //接收�?
	u8 RTC_Year;			//0~99
	u8 RTC_Mouth;			//1~12
	u8 RTC_Day;				//1~31
	u8 RTC_Hour;			//0~23
	u8 RTC_Minute;			//0~59
	u8 RTC_Second;			//0~59
	u8 RTC_Week;			//1~7 0不开�?1开�?�?0周一 6周日
	u8 UTC_Hour;			//-12~+12
	u8 UTC_Minute;			//0~59
	u8 TIME_ShowType;		//0~1
	u8 TIME_AutoDST;		//0~1
	u8 TIME_useDST;			//0~1
	u8 DST_StartMouth;		//1~12
	u8 DST_StartWeek;		//1~5
	u8 DST_StartWeekDay;	//1~7
	u8 DST_EndMouth;		//1~12
	u8 DST_EndWeek;			//1~5
	u8 DST_EndWeekDay;		//1~7 (1 = Monday, 2= Tuesday…�?.7= Sunday)
	u8 DST_AdjustHour;		//0~23
	u8 DST_AdjustMinute;	//0~59	
	u8 DST_AdjustTime;		//0~120 分钟
	u8 NULL15; u8 NULL16; u8 NULL17; u8 NULL18; u8 NULL19; u8 NULL1A; u8 NULL1B; u8 NULL1C; u8 NULL1D; u8 NULL1E; u8 NULL1F;
	u8 COIN1_VALUE_H;//20
	u8 COIN1_VALUE_L;
	u8 COIN2_VALUE_H;
	u8 COIN2_VALUE_L;
	u8 SHOW_DECIMAL;			//0~1
	u8 AUTO_START_PAID;			//0~1
	u8 SERIAL_PAYMENT_DEVICE;	//0~1 0:投币机型 1：读卡机�?
	u8 MACHINE_ADDRESS_H;
	u8 MACHINE_ADDRESS_L;
	u8 BEEPER;					//0~1 0 : BEPPER OFF 1:   BEPPER ON
	u8 BEEPER_END;				//0~30	�?
	u8 DEFAULT_PROGRAM;			//
	u8 DEFAULT_PROGRAM_TIMEOUT_H;
	u8 DEFAULT_PROGRAM_TIMEOUT_L;
	u8 WRINKLE_TIME;			//0~99	分钟
	u8 SCREEN_OUT_TIME_H;		//16
	u8 SCREEN_OUT_TIME_L;
	u8 SCREEN_BRIGHTNESS;		//0~100
	u8 TEMPERATURE_UNIT;		//0~1 0:华氏�?1:摄氏�?
	u8 FIRST_LANG;				//0~1 0:英语 1：西班牙�?
	u8 SECOND_LANG;				//0~1 
	u8 BUTTON_PIN;				//0~1
	u8 LIMIT_PIN_H;
	u8 LIMIT_PIN_L;
	u8 ALL_PIN_H;
	u8 ALL_PIN_L;				//0:不显�?1:显示
	u8 isShowTemp_OUT;	
	u8 isShowTemp_IN;//0:不显�?1:显示
	u8 external_signal_discount;// SIGNAL DISCOUNT
}WPEVENT_CTRL_PARAMRTER;
   

/*
size 0x2e
*/
typedef struct{// 0x600   //接收�?0x600-0x0x2D 46
	u8 WP_BOOT_H; //0 0x00//wp版本�?
	u8 WP_BOOT_L;
	u8 WP_USER_H;
	u8 WP_USER_L;
	u8 POWER_BOOT_H;//4//电源板版本号
	u8 POWER_BOOT_L;
	u8 POWER_USER_H;
	u8 POWER_USER_L;
	u8 MOTOR_BOOT_H;//8//电机驱动版本�?
	u8 MOTOR_BOOT_L;
	u8 MOTOR_USER_H;
	u8 MOTOR_USER_L;//11 0x0B
	u8 MAC_CODE1;	u8 MAC_CODE2;	u8 MAC_CODE3;	u8 MAC_CODE4;	u8 MAC_CODE5;//整机编码-20BYTES
	u8 MAC_CODE6;	u8 MAC_CODE7;	u8 MAC_CODE8;	u8 MAC_CODE9;	u8 MAC_CODE10;
	u8 MAC_CODE11;	u8 MAC_CODE12;	u8 MAC_CODE13;	u8 MAC_CODE14;	u8 MAC_CODE15;
	u8 MAC_CODE16;	u8 MAC_CODE17;	u8 MAC_CODE18;	u8 MAC_CODE19;	u8 MAC_CODE20;//31 0x1f
	u8 SN_CODE1;	u8 SN_CODE2;	u8 SN_CODE3;	u8 SN_CODE4;	u8 SN_CODE5;//整机编码-20BYTES
	u8 SN_CODE6;	u8 SN_CODE7;	u8 SN_CODE8;	u8 SN_CODE9;	u8 SN_CODE10;
	u8 SN_CODE11;	u8 SN_CODE12;	u8 SN_CODE13;	u8 SN_CODE14;	u8 SN_CODE15;
	u8 SN_CODE16;	u8 SN_CODE17;	u8 SN_CODE18;	u8 SN_CODE19;	u8 SN_CODE20;//31 0x1f
}WPEVENT_MACHINE_STATE;

typedef struct{// 0x700   //接收�?0x700-0x711 18
	u8 LOAD_STATUS1;//32 0x20 负载状�?
	u8 LOAD_STATUS2;
	u8 SWITCH_STATUS1;//
	u8 SWITCH_STATUS2;
	u8 OUT_NTC1_H;//624
	u8 OUT_NTC1_L; //625
	u8 IN_NTC1_H;//626
	u8 IN_NTC1_L; //627
}WPEVENT_MACHINE_RUN_STATE;

extern WP_EVENT_UI_LOGIC ui;
extern WP_EVENT_UI_LOGIC ui_tm;
extern WPEVENT_UI_DATA160 ui_data160;	//上传 DATA 0x160~16F
extern u8 dryerKeyValue1;
extern u8 dryerKeyValue2;
extern u8 keyLongPress;
u8 pressLeftMidFlag;

extern WPEVENT_PROGRAM_PARAMRTER pp;	//上传 程序参数 0x200
extern WPEVENT_DISCOUNT_PARAMRTER dp;	//上传 折扣参数 0x300
extern WPEVENT_INFO_PARAMRTER ip;		//上传 统计参数 0x400
extern WPEVENT_CTRL_PARAMRTER cp;		//上传 控制 0x500
extern WPEVENT_MACHINE_STATE ms;		//上传 机器运行 0x600
extern WPEVENT_MACHINE_RUN_STATE mrs ;
/*
******************************************************************************
*/

typedef struct{
	CycleType temp;//0,1,2,3四种温度
	u8 promo;//0~9 10种促销
}cache;
extern cache theCache;
/*
******************************************************************************
*/

typedef struct{//
	u8 number;
	u8 Price_H;		//0~0xffff 美分		价格
	u8 Price_L;		//0~0xffff 美分		价格
	u8 Default_H;		//0~0xffff	分钟	程序默认时间
	u8 Default_L;		//0~0xffff	分钟	程序默认时间
	u8 CoinAdd_H;		//0~0xffff	分钟	投币增加时间
	u8 CoinAdd_L;		//0~0xffff	分钟	投币增加时间
	u8 Available;	//0~1				程序是否可用
	u8 Free_H;		//0~0xffff	分钟	最长免费烘干时�?
	u8 Free_L;		//0~0xffff	分钟	最长免费烘干时�?
	u8 Maximum_H;		//0~0xffff	分钟	最长烘干时�?
	u8 Maximum_L;		//0~0xffff	分钟	最长烘干时�?
	u8 Temperature_H;	//0~0xffff	分钟	程序温度
	u8 Temperature_L;	//0~0xffff	分钟	程序温度
	u8 TempLimit_H;	//0~0xffff	摄氏�?进风口温度限�?
	u8 TempLimit_L;	//0~0xffff	摄氏�?进风口温度限�?
	u8 CoolingTime;	//0~0xff	分钟	冷却时间
	u8 CoolingTemp_H;	//0~0xffff	摄氏�?冷却温度
	u8 CoolingTemp_L;	//0~0xffff	摄氏�?冷却温度
}SetCycleParamtersType;
typedef struct
{
	u8 RTC_Year;			//0~99
	u8 RTC_Mouth;			//1~12
	u8 RTC_Day;				//1~31
	u8 RTC_Hour;			//0~23
	u8 RTC_Minute;			//0~59
	u8 RTC_Second;			//0~59
	u8 RTC_Week;			//1~7 0不开�?1开�?�?0周一 6周日
	u8 UTC_Hour;			//-12~+12
	u8 UTC_Minute;			//0~59
	u8 TIME_ShowType;		//0~1
	u8 TIME_ShowTypeampm;
	u8 TIME_AutoDST;		//0~1
	u8 TIME_useDST;			//0~1
	u8 DST_StartMouth;		//1~12
	u8 DST_StartWeek;		//1~5
	u8 DST_StartWeekDay;	//1~7
	u8 DST_EndMouth;		//1~12
	u8 DST_EndWeek;			//1~5
	u8 DST_EndWeekDay;		//1~7 (1 = Monday, 2= Tuesday…�?.7= Sunday)
	u8 DST_AdjustHour;		//0~23
	u8 DST_AdjustMinute;	//0~59	
	u8 DST_AdjustTime;		//0~120 分钟
}SetRtc;

typedef struct
{
	u8 COIN1_VALUE_H;
	u8 COIN1_VALUE_L;
	u8 COIN2_VALUE_H;
	u8 COIN2_VALUE_L;
	u8 SHOW_DECIMAL;			//0~1
	u8 AUTO_START_PAID;			//0~1
	u8 SERIAL_PAYMENT_DEVICE;	//0~1 0:投币机型 1：读卡机�?
	u8 MACHINE_ADDRESS_H;
	u8 MACHINE_ADDRESS_L;
	u8 BEEPER;					//0~1 0 : BEPPER OFF 1:   BEPPER ON
	u8 BEEPER_END;				//0~30	�?
	u8 DEFAULT_PROGRAM;			//
	u8 DEFAULT_PROGRAM_TIMEOUT_H;
	u8 DEFAULT_PROGRAM_TIMEOUT_L;
	u8 WRINKLE_TIME;			//0~99	分钟
	u8 SCREEN_OUT_TIME_H;		//16
	u8 SCREEN_OUT_TIME_L;
	u8 SCREEN_BRIGHTNESS;		//0~100
	u8 TEMPERATURE_UNIT;		//0~1 0:华氏�?1:摄氏�?
	u8 FIRST_LANG;				//0~1 0:英语 1：西班牙�?
	u8 SECOND_LANG;				//0~1 
	u8 BUTTON_PIN;				//0~1
	u8 LIMIT_PIN_H;
	u8 LIMIT_PIN_L;
	u8 ALL_PIN_H;
	u8 ALL_PIN_L;
	u8 isShowTemp_IN;				//0:不显�?1:显示
	u8 isShowTemp_OUT;				//0:不显�?1:显示
}SetMachineRunParameters;

/*
***********************************************************************************
*/
int writeDatatoUart(u8* data, u8 len);
int writeDatatoUartWithNoLog(u8* data, u8 len);
void writeSingleCmdToBoard(UI_EVENT_TYPE cmdType);
void writeDoubleCmdToBoard(UI_EVENT_TYPE type, u8 value);
void write_buffer_to_board();
void writeQueryCmdToBoard(DataOffset offset, u8 size);
/*
保存的一帧数�?
resendbuf	收到的完整数据，包括自己发送的+底板回复�?
sendDataLen	自己发送的 数据长度
receDataLen	底板回复�?数据长度
*/
typedef struct dealDataStruct
{
	u8 write_buf[OVEN_MAX_LEN];
	u8 resendbuf[OVEN_MAX_LEN];
	u8 sendDataLen;
	u8 receDataLen;
	u8 totalDataLen;//send + receive
	u8 resendtimes;
}dealDataType;

/*
正在接收的一帧数�?
*/
typedef struct
{
	u8 readSleepTime;//ms 38400 1ms 通信大概 38400*8/10/1000 = 31字节
	u8 readbuf[OVEN_MAX_LEN];
	int readbufLen;
	u8 readNullTimes;
	bool readBusFree;
	bool busTasking;
	bool busHaveTask;
}readStructType;
//readStructType readStruct = { READSLEEPTIME, { 0 }, 0, 0, true, false, false };

typedef struct _stevent{
	u8 UI_EventCounter;
	u8 UI_EventType;
	u8 UI_EventValue;
	u8 UI_EventData[124];
	u8 UI_datalen;//houda add 0705
}UIEVENT;
extern dealDataType dealData;
extern UIEVENT g_UIevent;
/*
******************************************************************************
*/
bool is_cx_service_mode();//服务模式
bool is_cx_normal_mode();//



void OutExternalInit(void);

extern void refreshStartLayer();
extern void refreshMainLayer();

extern bool UIDataChange;

int ExternalOutSend(OutExternalEvent* ev);
