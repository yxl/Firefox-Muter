#include <windows.h>
#include <tlhelp32.h>

#include "ApiHooks.h"
#include "DllEntry.h"
#include "HookMgr.h"
#include "BasicHooks.h"
#include "SoundHooks.h"
#include "SDKTrace.h"

HookMgr g_hookMgr = HookMgr();

HMODULE WINAPI ModuleFromAddress(PVOID pv)
{
	MEMORY_BASIC_INFORMATION mbi;
	if(::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0)
	{
		return (HMODULE)mbi.AllocationBase;
	}
	else
	{
		return NULL;
	}
}

HMODULE GetThisModule() 
{
	return ModuleFromAddress(GetThisModule);
}

BOOL ShouldHookModule( LPCSTR szModuleName)
{
	// The words in the list should be capitilized!
	static LPCSTR aIgnoreList[] = 
	{
		DLL_NAME,   // Our own module
		"NTDLL", 
		"GDI32", // GDI Client
		"GDIPLUS", // GDI+
		"MSIMG32", // GDIEXT Client
		"WSOCK32", // Windows Socket
		"WS2_32", // Windows Socket
		"MSWSOCK", // Windows Socket
		"WSHTCPIP", // Winsock2 Helper
		"WININET", // Win32 Internet Lib
		"SETUPAPI", // Window Installation API
		"IPHLPAPI", // IP Helper API
		"DNSAPI", // DNS Client API
		"RASADHLP", // Remote Access AutoDial Helper
		"UXTHEME", // Microsoft UxTheme Lib
		"WINNSI", // Network Store Information RPC interface
		"FECLIENT", // Windows NT File Encryption Client Interface
		"APPHELP",  // Application Compatibility Helper
		"SQLITE", // SQLite Database Lib
		"KSFMON", // KSafe Monitor
		"KWSUI", // Kingsoft Webshield Module
		"KDUMP", // Kingsoft Antivirus Dump Collect Lib
		"TORTOISE", // Tortoise Veriosn Control Client
	};

	// Converts to upper case string
	char szUpperCaseName[MAX_PATH];
	strcpy_s(szUpperCaseName, szModuleName);
	_strupr_s(szUpperCaseName);

	// Ignore list length
	int n = sizeof(aIgnoreList) / sizeof(LPCSTR);

	for (int i=0; i<n; i++)
	{
		if (strstr(szUpperCaseName, aIgnoreList[i]) != NULL)
			return FALSE;
	}

	return TRUE;
}

BOOL WINAPI InjectIntoProcess(HANDLE hprocess) 
{
	TCHAR dllpath[MAX_PATH];
	LPVOID memory_pointer = VirtualAllocEx(hprocess, NULL, 
		sizeof(dllpath), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

	if (!memory_pointer) 
	{
		return FALSE;
	}

	if (!g_hInstance)
	{
		g_hInstance = GetThisModule();
	}
	::GetModuleFileName(g_hInstance, dllpath, MAX_PATH);
	if (!WriteProcessMemory(hprocess, memory_pointer, dllpath, 
		sizeof(dllpath), NULL))
	{
		return FALSE;
	}

	HANDLE remote_thread = CreateRemoteThread(hprocess, NULL, 0,
		(LPTHREAD_START_ROUTINE)LoadLibraryA, memory_pointer, 0, NULL);
	if (!remote_thread) 
	{
		return FALSE;
	}

	CloseHandle(remote_thread);

	return TRUE;
}

void InjectAllProcess() 
{
	DWORD parent_pid = 0;
	BOOL find_apihook_flag = FALSE;

	/*
	// Get parent process id
	HANDLE hprocess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process = { sizeof(PROCESSENTRY32) };
	BOOL ret = Process32First(hprocess, &process);
	while (ret) {
	if (process.th32ProcessID == GetCurrentProcessId()) {
	parent_pid = process.th32ParentProcessID;
	HANDLE hmodule = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, parent_pid);
	MODULEENTRY32 mod = { sizeof(MODULEENTRY32) };
	if (Module32First(hmodule, &mod)) {
	while(Module32Next(hmodule, &mod)) {
	if (_tcsicmp(mod.szModule, _T("mutechrome.dll")) == 0 ||
	_tcsicmp(mod.szModule, _T("apihook.dll")) == 0) {
	find_apihook_flag = TRUE;
	break;
	}
	}
	}
	if (hmodule != INVALID_HANDLE_VALUE)
	CloseHandle(hmodule);
	break;
	}
	ret = Process32Next(hprocess, &process);
	}
	if (hprocess != INVALID_HANDLE_VALUE)
	CloseHandle(hprocess);


	// If the main chrome process has been injected then return directly.
	if (find_apihook_flag)
	return;
	*/

	HANDLE hprocess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 process = { sizeof(PROCESSENTRY32) };
	BOOL ret = Process32First(hprocess, &process);
	while (ret) {

		HANDLE process_handle = OpenProcess(
			PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | 
			PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, 
			FALSE, process.th32ProcessID);
		if (!process_handle) {
			TRACE("[MuterHook] Error OpenProcess GetLastError=%ld\n", GetLastError());
		} else {
			TRACE("[MuterHook] InjectIntoProcess, ProcessId=%ld\n", 
				process.th32ProcessID);
		}
		InjectIntoProcess(process_handle);
		if (process_handle)
			CloseHandle(process_handle);
		ret = Process32Next(hprocess, &process);
	}
	if (hprocess != INVALID_HANDLE_VALUE)
		CloseHandle(hprocess);
}

void InstallMuterHooks()
{
	::LoadLibraryA("dsound.dll");
	HANDLE hSnapshot;
	MODULEENTRY32 me = {sizeof(MODULEENTRY32)};

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,0);

	BOOL bOk = Module32First(hSnapshot,&me);
	while (bOk) 
	{
		if (ShouldHookModule(me.szModule))
		{
			if (!g_hookMgr.IsModuleHooked(me.hModule))
			{
				TRACE("[MuterHook] New module is hooked! %s\n", me.szModule);
			}
			if (strstr(me.szModule, "CCTV"))
			{
				TRACE("[MuterHook] New module is hooked! %s\n", me.szModule);
			}
			InstallHooksForNewModule(me.hModule);
		}
		bOk = Module32Next(hSnapshot,&me);
	}
}

void UnInstallMuterHooks()
{
	g_hookMgr.ClearAllHooks();
}

void InstallHooksForNewModule(HMODULE hModule)
{
	if (g_hookMgr.IsModuleHooked(hModule))
	{
		return;
	}

	// Basic function hooks
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "GetProcAddress", (PROC)GetProcAddress_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryA", (PROC)LoadLibraryA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryW", (PROC)LoadLibraryW_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryExA", (PROC)LoadLibraryExA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "LoadLibraryExW", (PROC)LoadLibraryExW_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "CreateProcessA", (PROC)CreateProcessA_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "Kernel32.dll", "CreateProcessW", (PROC)CreateProcessW_hook);

	// Sound function hooks
	g_hookMgr.InstallHookForOneModule(hModule, "winmm.dll", "waveOutWrite", (PROC)waveOutWrite_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "winmm.dll", "midiStreamOut", (PROC)midiStreamOut_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "dsound.dll", "DirectSoundCreate", (PROC)DirectSoundCreate_hook);
	g_hookMgr.InstallHookForOneModule(hModule, "dsound.dll", "DirectSoundCreate8", (PROC)DirectSoundCreate8_hook);
}