#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scene.h"
#include "ctrlboard.h"
#include "boardstate.h"
#include "layer_ctrl.h"

//ITUText* warnText;
ITUTextBox* warnTextBox;

static int warnFlag = -1;

static char* errStr[] =
{
	"ERROR0",//0  100
	"ERROR1",
	"Er00:\nMachine Out of Operation",//2  102
	"Er01:\nInlet air temperature too high",
	"Er02:\nOutlet air temperature too high",
	"Er03:\nInlet thermistor NTC2 \nShort Circuit",
	"Er04:\nOutlet thermistor NTC1 \nShort Circuit in Standby Mode",
	"Er05:\nCommunication disconnect \nwith PU",
	"Er06:\nCommunication disconnect \nwith DU",
	"Er14:\nHeating is not working",
	"Er17:\nInlet Thermister NTC2 \nOpen Circuit",//10
	"Er07:\nOutlet Thermister NTC1 \nOpen Circuit",
	"Er21:\nWIFI Module Disconnected/Lost",
	"Er28:\nInlet air temperature exceeded \nRegister 202 in previous cycle",
	"Er29:\nDoor open alarm",
	"Er30:\nFilter switch open",
	"Er31:\nMotor thermostat open",
	"Er32:\nInlet manual thermostat open",
	"Er33:\nInlet auto thermostat open",
	"Er34:\nAir flow switch can't connect",
	"Er35:\nOutlet thermostat open",//20
	"Er36:\nFan thermostat open",//21
	"Er37:\nCommunication error between \ndisplay and MC",//22
	"ERROR20", //
	"ERROR21",
	"ERROR22", "ERROR23", "ERROR24", "ERROR25", "ERROR26", "ERROR27", "ERROR28", "ERROR29", "ERROR30",//21~30

};
char warn[32] = "error";
void initWarnLayer(){
	//findWidget(warnText);
	findWidget(warnTextBox);
	ituTextSetFontWidth(warnTextBox, 40);
	ituTextSetFontHeight(warnTextBox, 40);
	ituSetColor(&(warnTextBox->fgColor), 0xff, 0xff, 0xff, 0x00);
	//warnText->stringSet->strings[0] = warn;
}
void warncpy(int i){
	if (!warnTextBox){
		initWarnLayer();
	}
	if (i<0 || i>30){
		textSet04HexNumber(warnTextBox, i + 100);
		//ituTextSetString(warnTextBox, "NO SUCH ERROR\0");
	}
	else{
		ituTextBoxSetString(warnTextBox, errStr[i]);
	}
}
void updateWarnStr(u8 code){

	warncpy(code - 100);
	return;
	/*switch (code)
	{
	case 102:
	warncpy(1);
	break;
	default:
	warncpy(0);
	break;
	}*/
}
void processWarn()
{
	if (warnFlag != ui.FaultCode){	//为了一次改变只执行一次
		warnFlag = ui.FaultCode;	//

		updateWarnStr(ui.FaultCode);

		/*if (ui.FaultCode > 99 && ui.FaultCode <= 140){
			updateWarnStr(ui.FaultCode);
		}*/
	}
}