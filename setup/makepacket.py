
import os
import shutil
import fnmatch
import subprocess
from xml.etree.ElementTree import ElementTree,Element
	
#当前包的版本号
version = "1.0.0.4"

#当前包的推广ID
oper = "0"
packetid = "3"
prouctid = "4"


#保存了当前路径
current_path = os.path.normcase(os.getcwd()) + '/'

#缓存路径
cache_path = current_path + "cache/" + version + '/'

#程序所在的路径
bin_path = current_path + "../bin/"

#包中的文件
need_copy_files =[ 
	#common file 
	"common file/mscom/comebase.dll",
	"common file/mscom/envmgr.dll",
	"common file/mscom/filemonitor.dll",
	"common file/mscom/msgbus.dll",
	"common file/mscom/msxml3w.dll",
	"common file/mscom/npc32.dll",
	"common file/mscom/npc64.dll",
	"common file/mscom/traywnd.dll",
	"common file/mscom/luahost.dll",
	"common file/mscom/managedb.dll",
	"common file/mscom/sqlitedb.dll",
	"common file/mscom/sqlitesdb.dll",
	"common file/mscom/downloadmgr.dll",
	"common file/mscom/DeviceControl.dll",
	"common file/mscom/msgcrt.dll",
	"common file/mscom/signalapp.dll",
	"common file/mscom/softeng.dll",
	
	
	#copy soui
	"imgdecoder-gdip.dll",
	"imgdecoder-png.dll",
	"imgdecoder-stb.dll",
	"imgdecoder-wic.dll",
	"render-gdi.dll",
	#"render-skia.dll",
	"resprovider-zip.dll",
	#"scriptmodule-lua.dll",
	"soui.dll",
	#"soui-sys-resource.dll",
	"translator.dll",
	"utilities.dll",
	"translator.dll",
	
	#copy duilib
	#"DuiLib.dll",
	
	"FreeImage.dll",
	"SciLexer.dll",
	"soft.db",
	
	#copy common
	"syslay.dll",
	"stdlog.dll",
	"msvcp100.dll",
	"msvcr100.dll",
	"lua-5.1.4.dll",
	"mscomcfg.xml",
	
	#copy setup
	"uninstall.exe",
	"uninstall.xml",
	"update.exe",
	"update.mscom",
	"update.xml",
	"setup.xml",
	
	
	#copy script
	"script/uninst.lua",
	"script/common.lua",
	"script/JCDesk.lua",
	"script/setup.lua",
	"script/JCSrv.lua",
	
	#############################################################
	#jcfs
	"common file/jcfs/fSearch.dll",
	"common file/jcfs/JCfsClient.dll",
	"common file/jcfs/JCfsUI.dll",
	
	#tray
	"common file/tray/DeskTray.dll",
	
	#update
	"common file/update/UpdateUI.dll",
	
	"x86/ShellExt32.dll",
	"x64/ShellExt64.dll",
	
	#jcsrv
	"JCSrv.exe",
	"JCSrv.mscom",
	"JCSrv.xml",
	
	#jcfs
	"JCfs.exe",
	"JCfs.mscom",
	"JCfs.xml",

	
	#JCTray
	"JCTray.exe",
	"JCTray.mscom",
	"JCTray.xml",
	
	"JCmd.exe",
	
	"directui license.txt",
	"duilib license.txt",
	"soui license.txt",
]

def enum_files(root, patterns = '*', single_level = False, yield_folders=False):
    patterns = patterns.split(';')
    for path, subdirs, files in os.walk(root):
        if yield_folders:
            files.extend(subdirs)
        files.sort()
        for name in files:
            for pattern in patterns:
                if fnmatch.fnmatch(name, pattern):
                    yield os.path.join(path,name)
                    break
        if single_level:
            break

def delete_file( des ):
	if os.path.exists(des):
		os.remove(des)
		
		
def copy_file(src, des):
	delete_file(des)
	make_dir = os.path.dirname(des) + "/"
	if not os.path.exists(make_dir):
		os.makedirs(make_dir)
	shutil.copyfile(src,des)

def set_xml_config_version( xml ):
	tree = ElementTree()
	tree.parse(xml)
	nodelist = tree.findall("envinfo/version")
	
	for node in nodelist:  
		node.text = version
		
	nodelist = tree.findall("envinfo/oper")
	for node in nodelist:  
		node.text = oper
		
	nodelist = tree.findall("envinfo/packetid")
	for node in nodelist:  
		node.text = packetid
		
	nodelist = tree.findall("envinfo/prouctid")
	for node in nodelist:  
		node.text = prouctid
		
	tree.write(xml, encoding="utf-8",xml_declaration=True)

#将文件拷贝到缓存目录中	
for file in need_copy_files:
	des = cache_path + file
	src = bin_path + file
	print(src)
	copy_file(src, des)

	
#修改xml配置文件中的信息
for file in enum_files( cache_path, '*.xml;'):
	set_xml_config_version( file )

#处理LUA脚本
for file in enum_files( cache_path, '*.lua;'):
	enc_lua = file + ".lbk"
	copy_file( file, enc_lua)
	delete_file(file)
	os.system("luac.exe -o  \"%s\" \"%s\"" % (os.path.normcase(file), os.path.normcase(enc_lua)))
	delete_file(enc_lua)

#for file in enum_files( cache_path, '*.exe;*.dll;'):
#	os.system("upx.exe \"%s\"" % os.path.normcase(file))
	

#将安文件添加到7z压缩包中
delete_file(os.path.normcase(os.getcwd() + "\\packet.7z"))
os.system("7-Zip\\7z.exe a -t7z \"%s\" \"%s\"" % (os.path.normcase(os.getcwd() + "\\packet.7z"), os.path.normcase(cache_path + "\\*")))

#执行编译程序
subprocess.call("C:\\Program Files (x86)\\Microsoft Visual Studio 10.0\\Common7\\IDE\\devenv.exe" + " ..\\code\\setup\\setup.vcxproj" + " /ReBuild Release")

#os.system("upx.exe \"%s\"" % "setup.exe")
#拷贝到发版目录中
copy_file("setup.exe", "release\\JCDeskSetup_" + version + ".exe")

name = input("Please input your name:\n")