#pragma once

/***************************************************************************
 *
 * FendyC
 * 2022-02-11
 * Copyright (C) 2022
 *
 * Sign info define head
 *
 ***************************************************************************/
 
 #include "soft_info.h"
 
 /*
 * root node
 */
 #define REG_SOFTWARE_NODE                			"Software\\BKPrinter"

 /*
 * Install action
 */
#define INSTALL_HKEY_NODE							"Software\\BKPrinter\\Install"
#define INSTALL_HEKY_ATTR_QID						"QID"
#define INSTALL_HKEY_ATTR_VERSION					"Version"
#define INSTALL_HKEY_ATTR_PATH						"Path"
#define INSTALL_HKEY_ATTR_INSTALL_COUNT				"InstallCount"
#define INSTALL_HKEY_ATTR_INSTALL_DATE				"InstallDate"

 /*
 * Uninstall
 */
#define REG_UNINST_NODE                  			"Software\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\BKPrinter"
#define REG_UNINST_DISPLAYNAME           			SOFT_NAME
#define REG_UNINST_PUBLISHER             			SOFT_NAME
