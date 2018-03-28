
RvdExtps.dll: dlldata.obj RvdExt_p.obj RvdExt_i.obj
	link /dll /out:RvdExtps.dll /def:RvdExtps.def /entry:DllMain dlldata.obj RvdExt_p.obj RvdExt_i.obj \
		kernel32.lib rpcndr.lib rpcns4.lib rpcrt4.lib oleaut32.lib uuid.lib \

.c.obj:
	cl /c /Ox /DWIN32 /D_WIN32_WINNT=0x0400 /DREGISTER_PROXY_DLL \
		$<

clean:
	@del RvdExtps.dll
	@del RvdExtps.lib
	@del RvdExtps.exp
	@del dlldata.obj
	@del RvdExt_p.obj
	@del RvdExt_i.obj
