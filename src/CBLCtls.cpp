// CBLCtls.cpp: Implementation of DLL exports.

#include "stdafx.h"
#include "res/resource.h"
#ifdef UNICODE
	#include "CBLCtlsU.h"
#else
	#include "CBLCtlsA.h"
#endif


class CBLCtlsModule :
    public CAtlDllModuleT<CBLCtlsModule>
{

public:
  #ifdef _UNICODE
		DECLARE_LIBID(LIBID_CBLCtlsLibU)
		DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CBLCTLS, "{9C943241-8C25-4b24-A0BE-61AECF9739AC}")
	#else
		DECLARE_LIBID(LIBID_CBLCtlsLibA)
		DECLARE_REGISTRY_APPID_RESOURCEID(IDR_CBLCTLS, "{4838DC24-BB1A-471f-96C9-DBD9107C4EDD}")
	#endif
};

CBLCtlsModule _AtlModule;


#ifdef _MANAGED
	#pragma managed(push, off)
#endif

// DLL entry point
extern "C" BOOL WINAPI DllMain(HINSTANCE /*hInstance*/, DWORD dwReason, LPVOID lpReserved)
{
	#ifdef _DEBUG
		// enable CRT memory leak detection & report
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF     // enable debug heap allocs & block type IDs (ie _CLIENT_BLOCK)
		    //| _CRTDBG_CHECK_CRT_DF              // check CRT allocations too
		    //| _CRTDBG_DELAY_FREE_MEM_DF       // keep freed blocks in list as _FREE_BLOCK type
		    | _CRTDBG_LEAK_CHECK_DF             // do leak report at exit (_CrtDumpMemoryLeaks)

		    // pick only one of these heap check frequencies
		    //| _CRTDBG_CHECK_ALWAYS_DF         // check heap on every alloc/free
		    //| _CRTDBG_CHECK_EVERY_16_DF
		    //| _CRTDBG_CHECK_EVERY_128_DF
		    //| _CRTDBG_CHECK_EVERY_1024_DF
		    | _CRTDBG_CHECK_DEFAULT_DF          // by default, no heap checks
		    );
		//_CrtSetBreakAlloc(209);               // break debugger on numbered allocation
		// get ID number from leak detector report of previous run
	#endif

	return _AtlModule.DllMain(dwReason, lpReserved); 
}

#ifdef _MANAGED
	#pragma managed(pop)
#endif

STDAPI DllCanUnloadNow(void)
{
	return _AtlModule.DllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	/* In early 2013, a Windows Update broke data-binding. This problem can be solved by calling
	 * regtlib.exe msdatsrc.tlb */
	TCHAR pRegtlib[512];
	ZeroMemory(pRegtlib, 512 * sizeof(TCHAR));
	TCHAR pSysDir[512];
	ZeroMemory(pSysDir, 512 * sizeof(TCHAR));
	if(GetSystemWow64Directory(pSysDir, 512) == 0 && GetLastError() == ERROR_CALL_NOT_IMPLEMENTED) {
		GetSystemDirectory(pSysDir, 512);
	}
	if(lstrlen(pSysDir) > 0) {
		PathAddBackslash(pSysDir);
	}
	if(GetWindowsDirectory(pRegtlib, 1024) > 0) {
		if(PathAppend(pRegtlib, TEXT("regtlib.exe")) && PathFileExists(pRegtlib) && PathFileExists(pSysDir)) {
			SHELLEXECUTEINFO info = {0};
			info.cbSize = sizeof(info);
			info.fMask = SEE_MASK_NOASYNC;
			info.lpVerb = TEXT("open");
			info.lpFile = pRegtlib;
			info.lpDirectory = pSysDir;
			info.lpParameters = TEXT("msdatsrc.tlb");
			info.nShow = SW_HIDE;
			ShellExecuteEx(&info);
		}
	}
	return _AtlModule.DllRegisterServer();
}

STDAPI DllUnregisterServer(void)
{
	return _AtlModule.DllUnregisterServer();
}