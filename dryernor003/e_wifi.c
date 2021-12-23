#include <sys/ioctl.h>
#include <pthread.h>
#include "network_config.h"

#include "ctrlboard.h"
#include "SDL/SDL.h"

extern int getWifiListInfo();
static bool downloadAppFlag = false;

static bool TestFuncWIFIExit = false;
static bool TestFuncWIFIisRun = false;


void* TestFuncWIFI(void* arg)
{
	uint32_t tick;
	TestFuncWIFIisRun = true;
	TestFuncWIFIExit = false;
	while (!TestFuncWIFIExit){
		if (1){//theConfig.wifi_on_off
			tick = SDL_GetTicks();
#ifdef CFG_NET_WIFI
			getWifiListInfo();
			printf("getWifiListInfo use %d ms\n", (SDL_GetTicks() - tick));
#endif
#ifdef WIN32
			printf("getWifiListInfo use %d ms\n", (SDL_GetTicks() - tick));
#endif
		}
		//#ifdef CFG_NET_WIFI
		//		if (NetworkWifiIsReady()){
		//			if (1){
		//				downloadAppFlag = true;
		//				DownLoadApp();
		//			}
		//		}else{
		//			printf("-------NetworkWifiIsNotReady---------\n");
		//		}
		//#else
		//#ifdef CFG_NET_ETHERNET
		//		if (NetworkIsReady()){
		//			if (1){
		//				downloadAppFlag = true;
		//				DownLoadApp();
		//			}
		//		}
		//#endif
		//#endif
		usleep(10000000);
	}
	TestFuncWIFIisRun = false;
}

void TestFuncWIFIFuncExit(){
	TestFuncWIFIExit = true;
}

bool TestFuncWIFIFuncisRun(){
	return TestFuncWIFIisRun;
}
void CreateWorkerThread(void *(*start_routine)(void *), void *arg);
void CreateWorkerWIFIThread(){
	if (TestFuncWIFIisRun){//已经在运行则退出
		return;
	}
	CreateWorkerThread(TestFuncWIFI, NULL);
}



/////////////////////////////////////////////////
#if 1

void upgradeWifi(){
	UpgradeSetUrl("http://192.168.170.59/WASHERPKG.PKG");
	SceneQuit(QUIT_UPGRADE_FIRMWARE);
}

static void wifiConnTask(void* arg)
{
	//upgradeWifi();
}
int createWifiConnTask(void)
{
	pthread_t       task;
	pthread_attr_t  attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_attr_setstacksize(&attr, (255 * 1024));
	pthread_create(&task, &attr, wifiConnTask, NULL);
	return 0;
}
#endif
/////////////////////////////////////////////////