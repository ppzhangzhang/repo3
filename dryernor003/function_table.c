#include "ite/itu.h"

extern bool LogoOnEnter(ITUWidget* widget, char* param);
extern bool LogoOnTimer(ITUWidget* widget, char* param);
extern bool LogoOnLeave(ITUWidget* widget, char* param);
extern bool CtrlOnEnter(ITUWidget* widget, char* param);
extern bool CtrlOnTimer(ITUWidget* widget, char* param);
extern bool CtrlOnLeave(ITUWidget* widget, char* param);
extern bool StartOnEnter(ITUWidget* widget, char* param);
extern bool StartOnTimer(ITUWidget* widget, char* param);
extern bool StartOnLeave(ITUWidget* widget, char* param);
extern bool MainOnEnter(ITUWidget* widget, char* param);
extern bool MainOnTimer(ITUWidget* widget, char* param);
extern bool MainOnLeave(ITUWidget* widget, char* param);
extern bool UpgradeOnEnter(ITUWidget* widget, char* param);
extern bool UpgradeOnTimer(ITUWidget* widget, char* param);
extern bool ProdmodOnEnter(ITUWidget* widget, char* param);
extern bool ProdmodOnTimer(ITUWidget* widget, char* param);
extern bool ProdmodOnLeave(ITUWidget* widget, char* param);
extern bool testModeLayerOnEnter(ITUWidget* widget, char* param);
extern bool testItemNext(ITUWidget* widget, char* param);
extern bool startTestItem(ITUWidget* widget, char* param);
extern bool testModeLayerOnTimer(ITUWidget* widget, char* param);
extern bool specialModeLayerOnEnter(ITUWidget* widget, char* param);
extern bool specialModeLayerOnTimer(ITUWidget* widget, char* param);
extern bool specialModeLayerOnLeave(ITUWidget* widget, char* param);
extern bool GototestOnEnter(ITUWidget* widget, char* param);
extern bool GototestOnTimer(ITUWidget* widget, char* param);
extern bool GototestOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOnEnter(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOnLeave(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidOnTimer(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidSignalScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidScrollListBoxOnSelect(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidNameScrollListBoxOnLoad(ITUWidget* widget, char* param);
extern bool SettingWiFiSsidStatusScrollListBoxOnLoad(ITUWidget* widget, char* param);

ITUActionFunction actionFunctions[] =
{
    "LogoOnEnter", LogoOnEnter,
    "LogoOnTimer", LogoOnTimer,
    "LogoOnLeave", LogoOnLeave,
    "CtrlOnEnter", CtrlOnEnter,
    "CtrlOnTimer", CtrlOnTimer,
    "CtrlOnLeave", CtrlOnLeave,
    "StartOnEnter", StartOnEnter,
    "StartOnTimer", StartOnTimer,
    "StartOnLeave", StartOnLeave,
    "MainOnEnter", MainOnEnter,
    "MainOnTimer", MainOnTimer,
    "MainOnLeave", MainOnLeave,
    "UpgradeOnEnter", UpgradeOnEnter,
    "UpgradeOnTimer", UpgradeOnTimer,
    "ProdmodOnEnter", ProdmodOnEnter,
    "ProdmodOnTimer", ProdmodOnTimer,
    "ProdmodOnLeave", ProdmodOnLeave,
    "testModeLayerOnEnter", testModeLayerOnEnter,
    "testItemNext", testItemNext,
    "startTestItem", startTestItem,
    "testModeLayerOnTimer", testModeLayerOnTimer,
    "specialModeLayerOnEnter", specialModeLayerOnEnter,
    "specialModeLayerOnTimer", specialModeLayerOnTimer,
    "specialModeLayerOnLeave", specialModeLayerOnLeave,
    "GototestOnEnter", GototestOnEnter,
    "GototestOnTimer", GototestOnTimer,
    "GototestOnLeave", GototestOnLeave,
    "SettingWiFiSsidOnEnter", SettingWiFiSsidOnEnter,
    "SettingWiFiSsidOnLeave", SettingWiFiSsidOnLeave,
    "SettingWiFiSsidOnTimer", SettingWiFiSsidOnTimer,
    "SettingWiFiSsidSignalScrollListBoxOnLoad", SettingWiFiSsidSignalScrollListBoxOnLoad,
    "SettingWiFiSsidScrollListBoxOnSelect", SettingWiFiSsidScrollListBoxOnSelect,
    "SettingWiFiSsidNameScrollListBoxOnLoad", SettingWiFiSsidNameScrollListBoxOnLoad,
    "SettingWiFiSsidStatusScrollListBoxOnLoad", SettingWiFiSsidStatusScrollListBoxOnLoad,
    NULL, NULL
};
