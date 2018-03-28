// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "DuiWindow.h"
USE_DEFAULT_DLL_MAIN;
BEGIN_CLIDMAP
	CLIDMAPENTRY_BEGIN
		CLIDMAPENTRY(CLSID_SearchUI,CDuiWindow)
	CLIDMAPENTRY_END
	END_CLIDMAP
DEFINE_ALL_EXPORTFUN