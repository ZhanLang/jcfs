
local searchPath = tostring(winapi.GetModuleFilePath(winapi.GetModuleHandle("lua-5.1.4.dll")));

package.path = package.path .. ";" .. searchPath .. "\\script\\?.lua"
package.cpath = package.cpath .. ";" .. searchPath .. "\\script\\?.dll"

require "common"

local function GetStrDate( _strTime)
	local ta  = LuaSplit(_strTime,' ');
	local size = table.getn(ta);
	if( size == 0 ) then
		return "";
	end
	
	
	local t = LuaReomveChar(ta[1], '-');
	print(t);
	return t;
end

local function GetStrTime( _strTime )
	local ta  = LuaSplit(_strTime,' ');
	local size = table.getn(ta);
	local t = LuaReomveChar(ta[2], ':');
	return t;
end

local function booltoint(_boolen)
	if _boolen == true then
		return 1;
	else
		return 0;
	end
end



_g_operid = 0;
_g_prodname="";
_g_idata = "";	--最初的安装日期
_g_itime = "";	--最初的安装时间


--lua 公开C++接口
_g_LuaVM=nil; 		--全局CLuaVM对象
function main(_luaVm) --在脚本初始化的时候调用
	_g_LuaVM = LuaVM.ConvertToLuaVM(_luaVm);
	
	_g_prodname = tostring(_g_LuaVM.GetEnvParamString("productname"));	--产品英文名称
	_g_operid = msapp.CApp(_g_prodname).GetSetting("oper", 0);
	
	_g_idata = msapp.CApp(_g_prodname).GetSetting("idata", tostring(mstime.GetCurrentDateStr(false)));
	_g_itime = msapp.CApp(_g_prodname).GetSetting("idata", tostring(mstime.GetCurrentTimeStr(false)));
	
end

--获取安装路径
local function GetProgramPosition()
	local path = _g_LuaVM.LuaCallFunction("Setup", "GetProgramPosition", "");
	local json = cjson.decode(tostring(path));
	return tostring(json["path"]);
end

function LuaGetSilent()
	local path = _g_LuaVM.LuaCallFunction("Setup", "GetSilent", "");
	local json = cjson.decode(tostring(path));
	return json["silent"];
end

local function DeleteProgramsShrotcut()
	local prodName = tostring(_g_LuaVM.GetEnvParamString("friendprodname"));
	local DeskTopPos = string.format("%s\\%s\\", tostring(quicklink.GetStartMenuPos()), prodName);
	winapi.DeletePath(DeskTopPos,true,false,true);
	return true;
end

local function DeleteDisktopShrotcut()
	local prodName = tostring(_g_LuaVM.GetEnvParamString("friendprodname"));
	local DeskTopPos = string.format("%s\\%s.lnk", tostring(quicklink.GetDesktopPos()), prodName);
	print(DeskTopPos);
	winapi.DeletePath(DeskTopPos,true,false,true);
end

local function DeleteTaskbarShrotcut()
	local prodName = tostring(_g_LuaVM.GetEnvParamString("friendprodname"));
	local DeskTopPos = string.format("\\%s.lnk", prodName);
	quicklink.DeleteTaskBar(DeskTopPos);
	return true;
end

local function DeleteUninstallReg()
	local regPath = nil
	if(winapi.IsWindowsX64()) then
		regPath =  "SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	else
		regPath =  "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\";
	end
	
	local regObj = winfunc.WinRegKey();
	
	local bRet = regObj.Open("HKEY_LOCAL_MACHINE", regPath , "KEY_READ_WRITE");
	if not bRet then
		return false;
	end
	
	regObj.DeleteSubKey(tostring(_g_LuaVM.GetEnvParamString("productname")));
end

local function TerminateProductProcess()
	local procName = string.format("%s.exe", tostring(_g_LuaVM.GetEnvParamString("productname")));
	winapi.TerminateProcessAsName(procName);
	return true;
end

local function Report()
  local _host = "update.kyan.com.cn";
  local _path = "uninstall";
  local _param = 
    {
      ["apiver"]      = 1;--接口版本号
      ["uguid"]       = winapi.GetUserGuid(),
	  ["mainbord"]	  = winapi.GetMainBoardSerialNumber(),
	  ["hdd"]	  	  = winapi.GetDiskSerialNumber(),
	  ["mac"]		  = winapi.GetNetAdapter(),
      ["prod"]        = _g_LuaVM.GetEnvParamString("prouctid"),
	  ["packet"]      = _g_LuaVM.GetEnvParamString("packetid"),
      ["prodver"]     = _g_LuaVM.GetEnvParamString("version"),
	  ["idate"]		  = _g_idata,
      ["itime"]		  = _g_itime,
      ["oper"]        = _g_operid,
      ["wow64"]       = winapi.IsWindowsX64(),
      ["osver"]          = winapi.GetMicroVersion(),
	  ["vm"]          = booltoint(osinfo.IsVM()),
	  ["type"]        = booltoint(LuaGetSilent()),
    };
   
  local _retcode , _context  = HttpGet(_host, 80, _path, _param);
  return _retcode;
end



function Do( param )

	log(tostring(GetProgramPosition()));
	local result = {};
	result["result"] = 0;
	
	
	if WinService.IsExistedService("JCSrv") then
		local JCsrvCmd = string.format("\"%s\" /UnregServer", GetProgramPosition().."\\JCSrv.exe");
		winapi.Execute(GetProgramPosition().."\\JCSrv.exe", JCsrvCmd, true, false, 900000);
		
		--如果服务已存在，直接删除该服务
		--WinService.StopService("JCSrv",10000, false);
		--WinService.UninstallService("JCSrv");
	end
	
	--结束进程
	winapi.TerminateProcessAsName("jcfs.exe");
	winapi.TerminateProcessAsName("jcsrv.exe");
	winapi.TerminateProcessAsName("JCTray.exe");
	winapi.TerminateProcessAsName("jcimhelp.exe");
	
	--删除文件搜索缓存目录
	local AppPath = msapp.CApp(_g_prodname).GetDataPath();
	winapi.DeletePathEx(tostring(AppPath) .. "fscache\\" , true, false, true);
	
	DeleteProgramsShrotcut();
	DeleteDisktopShrotcut();
	DeleteTaskbarShrotcut();
	DeleteUninstallReg();
	Report();
	
	--删除配置文件
	local settingFile = tostring(msapp.CApp((_g_prodname)).GetSettingFile());
	winapi.DeleteFileForce(settingFile);
	
	result["result"] = 1; --安装成功
	return cjson.encode(result);
end





