#pragma  once

void InstallMuterHooks();
void UnInstallMuterHooks();
void InjectAllProcess();

class HookMgr;
extern HookMgr g_hookMgr;

/**
* For Defining __asm Blocks as C Macros, 
* refers to http://msdn.microsoft.com/en-us/library/aa293825%28v=vs.60%29.aspx
*/
#define GET_CALLER(dwCaller)			\
{										\
	__asm push dword ptr [ebp+4]			\
	__asm pop  dword ptr [(dwCaller)]		\
}

HMODULE WINAPI ModuleFromAddress(PVOID pv);

HMODULE GetThisModule();

void InstallHooksForNewModule(HMODULE hModule);

BOOL WINAPI InjectIntoProcess(HANDLE hprocess);

