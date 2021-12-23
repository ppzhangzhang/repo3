#include <pthread.h>
#include "ctrlboard.h"
#include "network_config.h"


int nEthEable	= 0;
int nWifiEnable = 0;
static bool ThreadWiFiisRunning = false;
static bool ThreadWiFiExit = false;
static bool ThreadEthernetisRunning = false;
static bool ThreadEthernetExit = false;
void CreateWorkerThread(void *(*start_routine)(void *), void *arg);



static void* qcloud_mqtt_Task(void* arg){
	//mqtt_main();
	return NULL;
}
void MqttInit()
{
	CreateWorkerThread(qcloud_mqtt_Task, NULL);
}


/* Eason Refined in Mar. 2020 */

static void* NetworkEthTask(void* arg)
{
	ThreadEthernetisRunning = true;
	ThreadEthernetExit = false;
#if defined(CFG_NET_ETHERNET)
	NetworkPreSetting();

	while (!NetworkIsExit() && !ThreadEthernetExit)
	{
		/* Network Functions Process*/
		NetworkEthernetProcess();

		sleep(1);
	}
#endif

	return NULL;
}

static void* NetworkWifiTask(void* arg)
{
	ThreadWiFiisRunning = true;
	ThreadWiFiExit = false;
#if defined(CFG_NET_WIFI)
	NetworkWifiPreSetting();
#ifdef CFG_NET_WIFI_SDIO_POWER_ON_OFF_USER_DEFINED
	WifiPowerOn();
#endif

	while (!ThreadWiFiExit)
	{
		NetworkWifiProcess();

		sleep(1);
	}
#endif

	return NULL;
}
#ifdef CFG_NET_WIFI_SDIO_POWER_ON_OFF_USER_DEFINED
// contral wifi power on
static int gWifiPowerOn = 0;
extern WIFI_MGR_SETTING     gWifiSetting;
void WifiFirstPowerOn()
{
	itpRegisterDevice(ITP_DEVICE_SDIO, &itpDeviceSdio);
	ioctl(ITP_DEVICE_SDIO, ITP_IOCTL_INIT, NULL);

	itpRegisterDevice(ITP_DEVICE_WIFI_NGPL, &itpDeviceWifiNgpl);
	printf("====>itpInit itpRegisterDevice(ITP_DEVICE_WIFI_NGPL)\n");
	ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_INIT, NULL);


}

void WifiPowerOn()
{
	if (gWifiPowerOn == 0){
		WifiFirstPowerOn();
		gWifiPowerOn++;
	}
	else {
		wifi_on(RTW_MODE_STA);
		wifiMgr_init(WIFIMGR_MODE_CLIENT, 0, gWifiSetting);
	}

}

void WifiPowerOff()
{
	wifiMgr_terminate();

	wifi_off();

}
#endif

extern void itpWifiLwipInit(void);
extern void itpEthernetLwipInit(void);

void NetworkInit(void)
{

	if (theConfig.prodmod == 1){//工厂测试模式下
		if (theConfig.prodtestWiFi == 0){//首先测试网口
			nEthEable = 1;
			nWifiEnable = 0;
		}
		else if (theConfig.prodtestWiFi == 1){//测试WiFi
			nEthEable = 0;
			nWifiEnable = 1;
		}
		else{
			printf("%s() %ld error\r\n", __FUNCTION__, __LINE__);
			nEthEable = 1;
			nWifiEnable = 0;
		}
	}
	else{
		nEthEable = theConfig.nEthEnable;
		nWifiEnable = theConfig.nWifiEnable;
		theConfig.prodtestWiFi = 0;

	}



	/* Create Network Thread */

	/* WIFI Thread */
	if (nWifiEnable == 1){
		if (nEthEable != 1){
			printf("%s()->NetworkInit WIFI Thread\n",__FUNCTION__);
			CreateWorkerWiFiThread();
		}
		if (!theConfig.wifi_on_off){
			theConfig.wifi_on_off = true;
			ConfigSave();
		}
	}
	else{
		if (theConfig.wifi_on_off){
			theConfig.wifi_on_off = false;
			ConfigSave();
		}
	}
	/* Ethernet Thread */
	if (nEthEable == 1) {
		//ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_DISABLE, NULL);
		//ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_WIFI_NETIF_SHUTDOWN, NULL);
		printf("%s()->NetworkInit ethernet Thread\n", __FUNCTION__);
		CreateWorkerEthernetThread();
	}

	printf("emeng=>NetworkInit nEthEable=%d nWifiEnable=%d\n", nEthEable, nWifiEnable);
	//MqttInit();
}


void ThreadWiFiFuncExit(){
	ThreadWiFiExit = true;
}
void ThreadEthernetFuncExit(){
	ThreadEthernetExit = true;
}

void CreateWorkerWiFiThread(){
	{
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~[LwipInit] wifi \n");
#ifndef WIN32
#if defined(CFG_NET_WIFI) && !defined (CFG_NET_WIFI_SDIO_NGPL)
		itpWifiLwipInit();
#endif
#endif
	}
	if (ThreadWiFiisRunning){//已经在运行则退出
		return;
	}
	CreateWorkerThread(NetworkWifiTask, NULL);
}
void CreateWorkerEthernetThread(){
	{
		printf("%s()~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~[LwipInit] ethernet \n", __FUNCTION__);
		itpEthernetLwipInit();
	}
	if (ThreadEthernetisRunning){//已经在运行则退出
		return;
	}
	CreateWorkerThread(NetworkEthTask, NULL);
}
