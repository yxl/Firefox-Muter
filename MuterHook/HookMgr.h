#pragma once

#include <Windows.h>
#include <map>
#include <algorithm>

using namespace std;

class HookItem 
{
public:
	HMODULE hModule;
	// Import Module name
	LPSTR szImportModule;
	// Function to be hooked
	LPSTR szFunc;
	// Hook function
	PROC pHookFunc;
	// Original function
	PROC pOrigFunc;

	HookItem()
	{
		hModule = NULL;
		szImportModule = NULL;
		szFunc = NULL;
		pHookFunc = NULL;
		pOrigFunc = NULL;

	}

	HookItem(HMODULE hModule, LPCSTR szModule, LPCSTR szFunc, PROC pHookFunc, PROC pOrigFunc = NULL)
	{
		this->hModule = hModule;
		this->szImportModule = newAndCopyString(szModule);
		this->szFunc = newAndCopyString(szFunc);
		this->pHookFunc = pHookFunc;
		this->pOrigFunc = pOrigFunc;
	}

	~HookItem()
	{
		SafeDeleteString(szImportModule);
		SafeDeleteString(szFunc);
	}

private:
	static void SafeDeleteString(char* str)
	{
		if (str)
		{
			delete[] str;
		}
	}

	static char* newAndCopyString(LPCTSTR str) 
	{
		char* pResult = NULL;
		if (str != NULL)
		{
			int len = strlen(str);
			pResult = new char[len + 1];
			strcpy_s(pResult, len+1, str);
		}
		return pResult;
	}
};

typedef map<PROC, HookItem*> HookMap;
typedef map<HMODULE, HookMap*> ModuleMap;

class HookMgr
{
public:
	HookMgr(void);
	~HookMgr(void);

	void InstallHookForAllModules(LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc);

	void InstallHookForOneModule(HMODULE hModule, LPCSTR szImportModule, LPCSTR szFunc, PROC pHookFunc);

	void ClearAllHooks();

	HookItem* FindHook(HMODULE hModule, PROC pOrigFunc);
	PROC GetOriginalFunc(PROC pHook);
private:   
	void UnInstallAllHooksForOneModule(HMODULE hModule);
	ModuleMap m_modules;
	map<PROC, PROC> m_originalFunctions;

	CRITICAL_SECTION m_cs;
};
