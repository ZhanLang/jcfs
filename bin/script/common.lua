------------------------------------------------------------------
-- common.lua
-- Author     : 
-- Version    : 1.0.0.0
-- Date       : 2015-04-17
-- Description: 
------------------------------------------------------------------

------------------------------------------------------------------
_g_LuaVM=nil; 		--全局CLuaVM对象
------------------------------------------------------------------


-----------------------------初始化LuaVM--------------------------
function InitLuaVM(_luaVm)
	_g_LuaVM = LuaVM.ConvertToLuaVM(_luaVm);
	if(_g_LuaVM) then 
		return true;
	else 
		return false; 
	end
end
------------------------------------------------------------------


function booltoint(_boolen)
	if _boolen == true then return 1; end
	
	return 0;
end

function inttobool(_int)
	if( _int >= 0 )	then return true; end
	return false;
end

--获取最后错误值
function GetLastErrorText()
	return winapi.GetLastErrorText().c_str();
end

--获取当前路径
function GetCurrentPath()
	return winapi.GetCurrentPath().c_str();
end



--获取临时目录
function GetTempPath()
	return windir.PathProviderWin(windir.DIR_TEMP).c_str();
end




--实现HTTP请求
--host ： 请求的域名
--path ： 请求的路径
--param ： 参数 table 类型
function HttpGet(host, port, path, param)
	
	local strParam = "";
	if param and table.getn(param) then
		local first_param = true;
		strParam = strParam .. "?";
		for key, value in pairs(param) do
			if first_param then
				strParam = strParam .. key .. "=" .. http.encode_url(tostring(value));
				first_param = false;
			else
				strParam = strParam .. "&" .. key .. "=" .. http.encode_url(tostring(value));
			end
		end  
	end
	
	local httpImp = http.CHttpImpl();
	print(path.. tostring(strParam));
	httpImp.GetRequest(host, port , path.. tostring(strParam), tostring(strParam));
	local retCode = httpImp.GetRequestCode();
	if retCode == 0 then
		return retCode, httpImp.GetRequestData();
	end
	return retCode, nil;
end


function HttpPost(host, path, param)
	local url = "";
	if param and table.getn(param) then
		local first_param = true;
		for key, value in pairs(param) do
			if first_param then
				url = url .. key .. "=" .. http.encode_url(value);
				first_param = false;
				first_param = false;
			else
				url = url .. "&" .. key .. "=" .. http.encode_url(value);
			end
		end  
	end
	return http.http_post(tostring( host ), tostring( path ), tostring( url ));
end


--请求JSON, 返回JSON
function JsonRequest(host, port, rul, param)
	local httpImp = http.CHttpImpl();
	httpImp.JSonRequest(host, port, rul, param);
	local retCode = httpImp.GetRequestCode();
	if retCode == 0 then
		return retCode, httpImp.GetRequestData();
	end
	return retCode, nil;
end

--请求JSON, 返回JSON
function TextRequest(host, port, rul, param)
	local httpImp = http.CHttpImpl();
	httpImp.TextRequest(host, port, rul, param);
	local retCode = httpImp.GetRequestCode();
	if retCode == 0 then
		return retCode, httpImp.GetRequestData();
	end
	return retCode, nil;
end

--强制删除文件
function DeleteFile(_desc, _force)
	if(file.PathFileExists(_des) == true) then
		local ret = file.DeletePath(_des, true, false, true);
		if((ret == false) and (_force == true)) then
			math.randomseed(os.time());
			local moveFile = _des .. tostring(math.random());
			file.MoveFile(_des, moveFile);
			return file.DeletePathEx(moveFile,true,false,true);
		end
		
		return ret;
	end
	
	
	return false;
end


--复制文件
function CopyFile(_src, _des)
	--尝试删除文件
	winapi.DeleteFileForce(_des);
	return winapi.CopyFile(_src, _des, false, true, 0);
end


--移除指定的字符
function LuaReomveChar(str,remove)  
    local lcSubStrTab = {}  
    while true do  
        local lcPos = string.find(str,remove)  
        if not lcPos then  
            lcSubStrTab[#lcSubStrTab+1] =  str      
            break  
        end  
        local lcSubStr  = string.sub(str,1,lcPos-1)  
        lcSubStrTab[#lcSubStrTab+1] = lcSubStr  
        str = string.sub(str,lcPos+1,#str)  
    end  
    local lcMergeStr =""  
    local lci = 1  
    while true do  
        if lcSubStrTab[lci] then  
            lcMergeStr = lcMergeStr .. lcSubStrTab[lci]   
            lci = lci + 1  
        else   
            break  
        end  
    end  
    return lcMergeStr  
end  


-------------------------------------------------------
-- 参数:待分割的字符串,分割字符
-- 返回:子串表.(含有空串)
function LuaSplit(str,split)
	local lcSubStrTab = {}

   
    while true do  
        local lcPos = string.find(str,split)  
        if not lcPos then  
            lcSubStrTab[#lcSubStrTab+1] =  str      
            break  
        end  
        local lcSubStr  = string.sub(str,1,lcPos-1)  
        lcSubStrTab[#lcSubStrTab+1] = lcSubStr  
        str = string.sub(str,lcPos+1,#str)  
    end  

	return lcSubStrTab 
end 


--比较版本号
function CompareVersion(ver_1, ver_2)
	return VerCmp.CompareStr(ver_1, ver_2);
end

--获取命令行
function GetCommandLine()
	return tostring(winapi.GetCommandLine());
end

----------------------------------------------------------------
--获取产品信息
function GetProdInfo(strProd, strKey)
	return tostring(ProdInfo.GetProdInfo(tostring(strProd), tostring(strKey)));
end

--设置产品信息
function SetProdInfo(strProd, strKey, strValue)
	return ProdInfo.SetProdInfo(tostring(strProd), tostring(strKey), tostring(strValue));
end


--获取子产品信息
function GetSubProdInfo(strProd, strSub, strKey)
	return tostring(ProdInfo.GetSubProdInfo(tostring(strProd), tostring(strSub),  tostring(strKey)));
end

--设置子产品信息
function SetSubProdInfo(strProd, strSub, strKey, strValue)
	return ProdInfo.SetProdInfo(tostring(strProd), tostring(strSub) ,tostring(strKey), tostring(strValue));
end


-----------------------------------------获取XML环境变量-----------------------------------------

function GetEnvParamString( strKey)
	return tostring(_g_LuaVM.GetEnvParamString(tostring(strKey)));
end

---获取当前产品ID
function GetCurrentProdID()
	return tostring(GetEnvParamString("prouctid"));
end

function GetCurrentProdName()
	return tostring(GetEnvParamString("productname"));
end

-----------------------------------------算法-----------------------------------------

--base64
function Base64Encode(strText)
	return tostring(base64.Encode(tostring(strText)));
end

function Base64Decode(strText)
	return tostring(base64.Decode(tostring(strText)));
end

--des

--print(DesEncrypt("admin", "b2c3d4a1"));
--print(DesDecrypt("YWRtaW4A", "b2c3d4a1"));

function DesEncrypt(strText, strKey)
	return tostring(Des.Encrypt(tostring(strText), tostring(strKey)));
end


function DesDecrypt(strText, strKey)
	return tostring(Des.Decrypt(tostring(strText), tostring(strKey)));
end


function RC4_Decrypt(strText, strKey)
	local winCrypt =  winfunc.WinCrypt();
	winCrypt.RC4_Init(strKey, string.len(strKey));
	return winCrypt.RC4_Decrypt(strText, string.len(strText));
end


function RC4_Encrypt(strText, strKey)
	local winCrypt =  winfunc.WinCrypt();
	winCrypt.RC4_Init(strKey, string.len(strKey));
	return winCrypt.RC4_Encrypt(strText, string.len(strText));
end

function RC4_Encrypt_Base64(strText, strKey)
	local winCrypt =  winfunc.WinCrypt();
	winCrypt.RC4_Init(strKey, string.len(strKey));
	return Base64Encode(RC4_Encrypt(strText, strKey))
end

function RC4_Decrypt_Base64(strText, strKey)
	local winCrypt =  winfunc.WinCrypt();
	winCrypt.RC4_Init(strKey, string.len(strKey));
	return Base64Decode(RC4_Decrypt(strText, strKey))
end

function RC4_Encrypt_UTF8_Base64_Code(strText, strKey)
	local winCrypt =  winfunc.WinCrypt();
	winCrypt.RC4_Init(strKey, string.len(strKey));
	return winCrypt.RC4_Encrypt_UTF8_Base64_Code(strText, string.len(strText));
end

----
function UTF8ToWString(strText)
	return convert.utf8_string_to_wstring(tostring(strText));
end

function WStringToUTF8(strText)
	return convert.wstring_to_string_utf8(tostring(strText));
end