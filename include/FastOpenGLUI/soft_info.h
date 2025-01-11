#pragma once

/*
 * 软件信息
 */
#include "string"
const std::wstring SOFT_TYPE = L"MyInstallTesting";
#define MAIN_PROGRAM_NAME "MyApp"
#define INK_NAME _T("安装测试")
#define SOFT_CODE "PDMain"
#define SOFT_CODE_W _T("PDMain")
#define SOFT_NAME "AI打印机驱动助手"
#define SOFT_NAME_W _T("AI打印机驱动助手")
#define INSTALL_FOLDER SOFT_CODE
#define INSTALL_FOLDER_W SOFT_CODE_W
#define OPER_FOLDER SOFT_CODE
#define OPER_FOLDER_W SOFT_CODE_W
#define DOWNLOAD_FOLDER "PDMain"
#define DOWNLOAD_FOLDER_W _T("PDMain")

/*
 * 开始菜单和桌面快捷方式
 */
#define DESKTOP_LINK_FILENAME_W _T("AI打印机驱动助手.lnk")
#define STAERMENU_FOLDER_W SOFT_NAME_W
#define STAERMENU_LINK_FILENAME_W _T("启动AI打印机驱动助手.lnk")
#define STAERMENU_LINK_UNINSTNAME_W _T("卸载AI打印机驱动助手.lnk")


/*
 * 其他控制特性
 */
#define DEFAULT_FULLCHANNEL "guanwang"
#define SPACE_BYTE_SIZE 28 * 1024 * 1024
#define PASSWHITE_360 220405
