// FileZilla - a Windows ftp client

// Copyright (C) 2004 - Tim Kosse <tim.kosse@gmx.de>

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

// OptionsTypePage.cpp: Implementierungsdatei
//

#pragma once
#pragma message("Using include dir's ExceptionReport")
#ifndef MSExecptionTString
#include <string>
typedef std::basic_string<TCHAR> MSExecptionTString;
#endif

//ÐèÒª
class CExceptionReport
{
public:
	CExceptionReport();
	~CExceptionReport();

	static LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo);

private:

	static const MSExecptionTString FormatCurrentTimeString();

	static bool writeMiniDump(PEXCEPTION_POINTERS pExceptionInfo);

	static void SuspendThreads();


	static LPTOP_LEVEL_EXCEPTION_FILTER m_previousExceptionFilter;
	static TCHAR m_pDmpFileName[MAX_PATH];
	static HANDLE m_hDumpFile;
	
	static BOOL m_bFirstRun;
};


extern CExceptionReport ExceptionReport; //global instance of class




