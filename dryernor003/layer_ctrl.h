#include "projutils.h"

/*
#include "layer_ctrl.h"
*/




ITULayer* ctrlLayer;
ITULayer* logoLayer;

ITULayer* mainLayer;
ITULayer* startLayer;
ITULayer* prodmodLayer;
ITULayer* settingWiFiSsidLayer;

extern bool busComOK;
extern bool bluetoothOK;
extern u16 ctrl_timer;
//

bool(*ButtonUpOnPress)();
bool(*ButtonDownOnPress)();
bool(*ProcessButtomDown)();

///*
//Nc ReturnIdle
//*/
extern void outQsetType(u8 type);
///*
//Cycle StartPause
//*/
extern void outQsetTypeValue(u8 type, u8 value);
extern void boardsetCycleCMD(u8 ct);
//extern void outQsetOther(u8 type, u8 location);
extern void outQquery(u16 qt);
extern void outQsetBuffer(u16 address, u8 datalen, u8* data);
//
extern void out_write_buffer_struct_to_board(void*data, u8 len);

typedef struct{
	u8 CycleName;
	u8 RunMode;
	u8 DryingStage;
	u16 RmainTime;
	u16 MoneyCost;
	bool PayShow;
}StartLayerType;

typedef struct{
	u8 CycleName;
}CtrlLayerType;

typedef enum{//下发的
	DRY_Nc = 0,				//0 空事件
	DRY_Reset,				//1 复位
	DRY_PowerButton = 3,	//3	0=关机/1=开机 电源键
	DRY_Cycle,				//4	程序名 程序选择
	DRY_CycleStart,			//5 程序名 程序选择+运行
	DRY_StartPause,			//6	0=暂停/1=启动 启停
	DRY_Invalid = 10,		//10 无效操作
	DRY_FactoryReset,		//11 0=普通用户/1=售后 0=普通用户时,只将程序记忆恢复出厂值.1 = 售后时, 除将程序记忆恢复出厂值外, 还将清零除型号外其他记忆参数如周期数等.
	DRY_ReturnIdle,			//12 返回待机状态
	DRY_MessageReply,		//13 提示信息序号(与WP上传序号同) 对话框键值0=空/1=确定/2=取消3=我知道了/4=不再提醒  提示信息应答 注:WP发出的提示信息,用户按触相应按键进行的确认操作.
	DRY_SetCycleParamters,	//14 0e 程序名 设置程序价格
	DRY_SetHappyHourTime = 16,//16 0x10	折扣序号
	DRY_SetRtc,				//17 0x11 设置RTC时间
	DRY_SetMachineRunParameters,//18 0x12 设置机器运行参数
	DRY_UiTouch,			//UiTouch 筒灯 和 不断电在线升级
	DRY_ModuleSet = 200,	//200 0=进入型号设置/1=序号加/2=序号减3=保存/4=放弃
	DRY_TestOptionsSwitch,	//201 0=加快十倍循环开关1=寿命测试循环开关2=washer模式循环开关3=特殊显示(B_TST)循环开关4=手控负载循环开关5=demo6=Diagnostic Swtich 测试模式开关注:用于开启和关闭一些特殊的工作模式
	DRY_TstSpecialNum,		//202 1=加1/ 2=减1/ 3=加10/ 4=减10 特殊显示序号
	DRY_UniversalDialogBox,	//203 0=上一步/1=下一步2=启停/3=返回 通用对话框 注:用于产线自检/手控负载
	DRY_SetMotoPara,		//204 0=转速1=加速度2=减速度 值,双字节高位在前转速:范围0-1600rpm,加速度:范围0-1001rpm/s减速度:范围0-1001rpm/s 用于几个特殊流程
	DRY_UISendData,			//205 0=UI version,高位在前 1=其他 data length data…
	DRY_EXTRA_QUERY,//查询
	DRY_EXTRA_SETBUFFER,//设置
	DRY_UIEVENT_TstStep,//洗鞋机 testmode移植
}DRY_UI_EVENT_TYPE;