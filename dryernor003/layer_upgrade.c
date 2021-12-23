#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scene.h"
#include "ctrlboard.h"

static ITULayer* mainMenuLayer;

//void NetworkSntpUpdate(void){
//
//}
#ifdef WIN32
void NetworkSntpUpdate(void){

}
#endif

bool UpgradeOnTimer(ITUWidget* widget, char* param)
{
    if (UpgradeIsFinished())
    {
        SceneQuit(QUIT_NONE);

		//NetworkSntpUpdate();

        ituLayerGoto(mainMenuLayer);
    }
	return true;
}

bool UpgradeOnEnter(ITUWidget* widget, char* param)
{
    if (!mainMenuLayer)
    {
        mainMenuLayer = ituSceneFindWidget(&theScene, "mainMenuLayer");
        assert(mainMenuLayer);
    }    
    UpgradeProcess(SceneGetQuitValue());
	return true;
}

void UpgradeReset(void)
{
    mainMenuLayer = NULL;
}
